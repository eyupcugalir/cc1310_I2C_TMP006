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
/******************************************************************************/

#ifndef TIDA_00488_FIRMWARE_HDC1000_H_
#define TIDA_00488_FIRMWARE_HDC1000_H_

/*****************************************************************************
* DEFINES
*/

#define HDC1000_ADDRESS			0x40	//I2C address for HDC1000

#define HDC1000_TEMP_ADDR		0x00	//Temperature measurement output register
#define	HDC1000_HUMIDITY_ADDR	0x01	//Relative humidity output register
#define HDC1000_CONFIG_ADDR		0x02	//HDC1000 configuration and status

#define HDC1000_MAN_ID_ADDR		0xFE	//Manufacturer ID register (default: 'TI')

/*
	CONFIGURATION REGISTER DESCRIPTION
	============================================
	Name	Registers	Description			Configuration
	RST		[15]		SW reset			0:  Normal operation
	Rsvd	[14]		Reserved			N/A
	HEAT	[13]		Heater				0:  Heater disabled
	MODE	[12]		Mode of acquisition	1:  Temp&Humidity in sequence
	BTST	[11]		Battery status		N/A
	TRES	[10]		Temp resolution		1:  11 bit
	HRES	[9:8]		Humidity resolution	01: 11 bit
	Rsvd	[7:0]		Reserved			N/A
*/
#define HDC1000_TEMP_RH_11BIT_MSB		0x15	//MSB of configuration
#define HDC1000_TEMP_RF_11BIT_LSB		0x00	//LSB of configuration

/* HDC1000 timing */
#define HDC_START_TIME	8000	/* Delay before first HDC I2C access */
#define HDC_MEAS_TIME	8000	/* Delay between start of conversion to measurement read */
#define HDC_CONFIG_TIME	1000	/* Amount of time to wait until I2C retry after failed attempt */

/* HDC1000 data ready trigger */
#define HDC_MEAS_INTERRUPT	0
#define HDC_MEAS_DELAY		1
#define HDC_MEAS_METHOD		HDC_MEAS_DELAY

/*****************************************************************************
* GLOBAL FUNCTIONS
*/
void HDC1000_peripheral_init(void);
void HDC1000_config_init(void);
void HDC1000_revision(uint8_t*);
void HDC1000_configuration_read(uint8_t*);
void HDC1000_start_measurement(void);
void HDC1000_read_measurement(uint8_t*);


#endif /*TIDA_00488_FIRMWARE_HDC1000_H_ */
