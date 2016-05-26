/*
 * Copyright (c) 2014-2015, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
* INCLUDES
*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/drivers/I2C.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <CC1310_LAUNCHXL.h>

#include "hdc1000.h"

#include "string.h"

/*****************************************************************************
* GLOBAL VARIABLES
*/
extern PIN_Handle       DONE_handle;

/*****************************************************************************
* LOCAL FUNCTION PROTOTYPES
*/
void nDRDY_HwiFxn(PIN_Handle hPin, PIN_Id pinId);
static void hdcSyncClockCallback(UArg a0);

/*****************************************************************************
* LOCAL VARIABLES
*/

//I2C variables
extern I2C_Handle       i2c;
extern I2C_Params       i2cParams;
extern I2C_Transaction  i2cTransaction;
uint8_t                 txBuf[3] = {0};
bool                    status = false;

//GPIO variables
PIN_Config              nDRDYPinTable[] = {
	Board_HDC_nDRDY | PIN_INPUT_EN | PIN_NOPULL | PIN_HYSTERESIS,
    PIN_TERMINATE                               
};

#if HDC_MEAS_METHOD==HDC_MEAS_INTERRUPT
	static PIN_State        nDRDY_state;
	static PIN_Handle       nDRDY_handle;
#endif

//Clock management
static Semaphore_Struct hdcSyncSem;
static Semaphore_Handle hdcSyncSemHandle;
static Clock_Struct hdcSyncClock;
static Clock_Handle hdcSyncClockHandle;



/*****************************************************************************
* FUNCTIONS
*/
/*****************************************************************************
* @brief    HWI interrupt function to trigger on nDRDY pin
*
* @return   None
******************************************************************************/
void nDRDY_HwiFxn(PIN_Handle hPin, PIN_Id pinId)
{
	if(pinId == Board_HDC_nDRDY){
		Semaphore_post(hdcSyncSemHandle);
	}
}

/*****************************************************************************
* @brief    Clock callback function
*
* @return   None
******************************************************************************/
static void hdcSyncClockCallback(UArg a0)
{
	Semaphore_post(hdcSyncSemHandle);
}

/*****************************************************************************
* @brief    Initialize HDC GPIO and clock driver
*
* @return   None
******************************************************************************/
void HDC1000_peripheral_init(void)
{
	/* Setup timer used for HDC delays */
	Clock_Params hdcSyncClockParams;
	Clock_Params_init(&hdcSyncClockParams);
	hdcSyncClockParams.startFlag = 0;
	hdcSyncClockParams.period = 0; /* One-off clock */
	Clock_construct(&hdcSyncClock, hdcSyncClockCallback, 0, &hdcSyncClockParams);
	hdcSyncClockHandle = Clock_handle(&hdcSyncClock);

	/* Create semaphore to sync HDC delays */
	Semaphore_Params hdcSyncParams;
	Semaphore_Params_init(&hdcSyncParams);
	Semaphore_construct(&hdcSyncSem, 0, &hdcSyncParams);
	hdcSyncSemHandle = Semaphore_handle(&hdcSyncSem);

	/* Delay by HDC_START_TIME to meet HDC start up time requirement */
	Clock_setTimeout(hdcSyncClockHandle, HDC_START_TIME / Clock_tickPeriod);
	Clock_start(hdcSyncClockHandle);

	/* Wait for hdc clock wakeup */
	Semaphore_pend(hdcSyncSemHandle, BIOS_WAIT_FOREVER);

	#if HDC_MEAS_METHOD==HDC_MEAS_INTERRUPT
		/* Configure GPIO interrupt */
		nDRDY_handle = PIN_open(&nDRDY_state, nDRDYPinTable);
		PIN_registerIntCb(nDRDY_handle, &nDRDY_HwiFxn);
		PIN_setConfig(nDRDY_handle, PIN_BM_IRQ, Board_HDC_nDRDY | PIN_IRQ_NEGEDGE);
	#endif
}

/*****************************************************************************
* @brief    Initialize HDC1000 and configure the temp/humidity resolution
*
* @return   None
******************************************************************************/
void HDC1000_config_init(void)
{
	uint8_t tries = 0;
	status = false;

	while(status == false)
	{
		//Write to configuration register
		txBuf[0] = HDC1000_CONFIG_ADDR;
		txBuf[1] = HDC1000_TEMP_RH_11BIT_MSB;
		txBuf[2] = HDC1000_TEMP_RF_11BIT_LSB;

		i2cTransaction.writeBuf = txBuf;
		i2cTransaction.writeCount = 3;
		i2cTransaction.readBuf = 0;
		i2cTransaction.readCount = 0;
		i2cTransaction.slaveAddress = HDC1000_ADDRESS;
		status = I2C_transfer(i2c, &i2cTransaction);

		tries++;

		if(status == false)
		{
			//ERROR: HDC is not responding

			//If tried 5 times, power off the uC
			if(tries == 5){
				PIN_setOutputValue(DONE_handle, Board_SYS_DONE, 1);
			}

			//Wait 1ms and try I2C access again

			Clock_setTimeout(hdcSyncClockHandle, HDC_CONFIG_TIME / Clock_tickPeriod);
			Clock_start(hdcSyncClockHandle);
			Semaphore_pend(hdcSyncSemHandle, BIOS_WAIT_FOREVER);
		}
	}
}

/*****************************************************************************
* @brief    Return the HDC1000 manufacturer ID
*
* @return   None
* @param    buffer will hold manufacturer ID
******************************************************************************/
void HDC1000_revision(uint8_t* buffer)
{
	status = false;
	txBuf[0] = HDC1000_MAN_ID_ADDR;
  
	i2cTransaction.writeBuf = txBuf;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = buffer;
	i2cTransaction.readCount = 2;
	i2cTransaction.slaveAddress = HDC1000_ADDRESS;
	status = I2C_transfer(i2c, &i2cTransaction);
  
	if(status == false){
		/* Error */
	}
}

/*****************************************************************************
* @brief    Return the configuration register content
*
* @return   None
* @param    buffer will contain configuration register content
******************************************************************************/
void HDC1000_configuration_read(uint8_t* buffer)
{
	status = false;
	txBuf[0] = HDC1000_CONFIG_ADDR;
  
	i2cTransaction.writeBuf = txBuf;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = buffer;
	i2cTransaction.readCount = 2;
	i2cTransaction.slaveAddress = HDC1000_ADDRESS;
	status = I2C_transfer(i2c, &i2cTransaction);
  
	if(status == false){
		/* Error */
	}
}

/*****************************************************************************
* @brief    Start temperature and humidity measurement
*
* @return   None
******************************************************************************/
void HDC1000_start_measurement()
{
	status = false;
	txBuf[0] = HDC1000_TEMP_ADDR;
  
	//Write data to TEMP register to trigger a measurement
	i2cTransaction.writeBuf = txBuf;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = 0;
	i2cTransaction.readCount = 0;
	i2cTransaction.slaveAddress = HDC1000_ADDRESS;
	status = I2C_transfer(i2c, &i2cTransaction);
	if(status == false){
		/* Error */
	}
}

/*****************************************************************************
* @brief    Wait for the nDRDY_HDC pin to go low indicating a measurement
* 			is ready for reading, read temperature and humidity registers
*
* @return   None
* @param    buffer will contain both temperature and humidity measurement
*           [0] Temperature MSB
*           [1] Temperature LSB
*           [2] Relative humidity MSB
*           [3] Relative humidity LSB
******************************************************************************/
void HDC1000_read_measurement(uint8_t* buffer)
{
	status = false;

	#if HDC_MEAS_METHOD==HDC_MEAS_INTERRUPT
		//Wait for nDRDY_RDY pin to signal measurement is ready to read
		Semaphore_pend(hdcSyncSemHandle, BIOS_WAIT_FOREVER);
	#endif

	#if HDC_MEAS_METHOD==HDC_MEAS_DELAY
		/* Wait for HDC to have data */
		Clock_setTimeout(hdcSyncClockHandle, HDC_MEAS_TIME / Clock_tickPeriod);
		Clock_start(hdcSyncClockHandle);

		/* Wait for hdc signal */
		Semaphore_pend(hdcSyncSemHandle, BIOS_WAIT_FOREVER);
	#endif

	//Read data
	status = false;
	i2cTransaction.writeBuf = txBuf;
	i2cTransaction.writeCount = 0;
	i2cTransaction.readBuf = buffer;
	i2cTransaction.readCount = 4;
	i2cTransaction.slaveAddress = HDC1000_ADDRESS;
	status = I2C_transfer(i2c, &i2cTransaction);
	if(status == false){
		/* Error */
	}
}


