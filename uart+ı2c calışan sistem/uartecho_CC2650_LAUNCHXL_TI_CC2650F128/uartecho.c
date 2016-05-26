  /*
   * Copyright (c) 2016, Texas Instruments Incorporated
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





  //
   //  ======== uartecho.c ========
    //

  // XDCtools Header files //
  #include <xdc/std.h>
  #include <xdc/runtime/System.h>
  #include <xdc/cfg/global.h>

  // BIOS Header files //
  #include <ti/sysbios/BIOS.h>
  #include <ti/sysbios/knl/Task.h>
  #include <ti/sysbios/knl/Clock.h>


  // TI-RTOS Header files //
  #include <ti/drivers/PIN.h>
  #include <ti/drivers/UART.h>
  #include <ti/drivers/I2C.h>
  #include <ti/drivers/SPI.h>
  //#include <TMP112.h>

  #include <hdc1000.h>

  // Example/Board Header files
  #include "Board.h"

  #include <stdint.h>

  #define TASKSTACKSIZE     768

  Task_Struct task0Struct;
  Char task0Stack[TASKSTACKSIZE];





  // Global memory storage for a PIN_Config table
  static PIN_State ledPinState;

  //
  // Application LED pin configuration table:
  //   - All LEDs board LEDs are off.
   //
  PIN_Config ledPinTable[] = {
      Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
      Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
      Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_GENERALPURPOSE1   | PIN_GPIO_OUTPUT_EN 	| PIN_GPIO_HIGH	 | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_CP2103UARTRESET   | PIN_GPIO_OUTPUT_EN	| PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
      PIN_TERMINATE
  };


   //  ======== echoFxn ========
   //  Task for this function is created statically. See the project's .cfg file.


  Void echoFxn(UArg arg0, UArg arg1)
  {
      char input;
      UART_Handle uart;
      UART_Params uartParams;
      const char echoPrompt[] = "\fEchoing characters:\r\n";

      // Create a UART with data processing off.
      UART_Params_init(&uartParams);
      uartParams.writeDataMode = UART_DATA_BINARY;
      uartParams.readDataMode = UART_DATA_BINARY;
      uartParams.readReturnMode = UART_RETURN_FULL;
      uartParams.readEcho = UART_ECHO_OFF;
      uartParams.baudRate = 9600;
      uart = UART_open(Board_UART0, &uartParams);

      if (uart == NULL) {
          System_abort("Error opening the UART");
      }else{
          System_printf("hello uart");
      }
      System_flush();


      UART_write(uart, echoPrompt, sizeof(echoPrompt));

      // Loop forever echoing
      while (1) {
          UART_read(uart, &input, 1);
          UART_write(uart, &input, 1);
      }
  }







   //  ======== echoFxn ========
   //  Task for this function is created statically. See the project's .cfg file.
  //
  Void taskFxn(UArg arg0, UArg arg1)
  {
    	PIN_Handle ledPinHandle;

    	ledPinHandle = PIN_open(&ledPinState, ledPinTable);

    	PIN_setOutputValue(ledPinHandle, Board_LED1, 1);
        unsigned int    i;
        uint16_t        temperature;
        uint8_t         txBuffer[2];
        uint8_t         rxBuffer[2];
        I2C_Handle      i2c;
        I2C_Params      i2cParams;
        I2C_Transaction i2cTransaction;

        uint8_t         txbuf_threshold[3];

        /* Create I2C for usage */
        I2C_Params_init(&i2cParams);
        i2cParams.bitRate = I2C_100kHz;
        i2c = I2C_open(Board_I2C_TMP, &i2cParams);
        if (i2c == NULL) {
            System_abort("Error Initializing I2C\n");
        }
        else {
            System_printf("I2C Initialized!\n");
        }

        /* Point to the T ambient register and read its 2 bytes */
     /*
        txBuffer[0] = 0x02;

        txbuf_threshold[0] = 16;
        txbuf_threshold[1] = 48;
        i2cTransaction.slaveAddress = Board_TMP112_ADDR;
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readBuf = rxBuffer;
        i2cTransaction.readCount = 2;



            if (I2C_transfer(i2c, &i2cTransaction)) {



                System_printf("Sample %d (C)\n", rxBuffer[0]);
                System_printf("Sample %d (C)\n", rxBuffer[1]);


            }
            else {
                System_printf("I2C Bus fault\n");
            }

            txbuf_threshold[0] = 0x02;
            txbuf_threshold[1] = 32;
            txbuf_threshold[2] = 48;
            i2cTransaction.slaveAddress = Board_TMP112_ADDR;
            i2cTransaction.writeBuf = txbuf_threshold;
            i2cTransaction.writeCount = 3;
            i2cTransaction.readBuf = rxBuffer;
            i2cTransaction.readCount = 2;
            if (I2C_transfer(i2c, &i2cTransaction)) {


                System_printf("Sample %d (C)\n", rxBuffer[0]);
                System_printf("Sample %d (C)\n", rxBuffer[1]);





            }
            else {
                System_printf("I2C Bus fault\n");
            }
*/



            txbuf_threshold[0] = 0x00;
             i2cTransaction.slaveAddress = Board_TMP112_ADDR;
             i2cTransaction.writeBuf = txbuf_threshold;
             i2cTransaction.writeCount = 1;
             i2cTransaction.readBuf = rxBuffer;
             i2cTransaction.readCount = 2;
             for (i = 0; i < 40; i++) {
             if (I2C_transfer(i2c, &i2cTransaction)) {


                // System_printf("Sample %d (C)\n", rxBuffer[0]);
                 System_flush();

                 //System_printf("Sample %d (C)\n", rxBuffer[1]);
                 System_flush();

                 temperature = (rxBuffer[0] << 4) | (rxBuffer[1] >> 4);
                 temperature /= 16;
                 System_printf("Sample %u: %d (C)\n", i, temperature);
                 System_flush();



             }
             else {
                 System_printf("I2C Bus fault\n");
                 System_flush();

             }




            System_flush();
            Task_sleep(100000);
             }
        /* Deinitialized I2C */
        I2C_close(i2c);
        System_printf("I2C closed!\n");

        System_flush();

    }







   //  ======== main ========
   //
  int main(void)
  {
      PIN_Handle ledPinHandle;
      Task_Params taskParams;

      // Call board init functions
      Board_initGeneral();
      Board_initUART();
      Board_initI2C();


      // Construct BIOS objects
      Task_Params_init(&taskParams);
      taskParams.stackSize = TASKSTACKSIZE;
      taskParams.stack = &task0Stack;
      // SADECE echoFxn taskini atamışsınız, şimdilik bunun yerine I2C taskini atayın
      //Task_construct(&task0Struct, (Task_FuncPtr)echoFxn, &taskParams, NULL);
      Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);


      // Open LED pins
      ledPinHandle = PIN_open(&ledPinState, ledPinTable);

      if(!ledPinHandle) {
          System_abort("Error initializing board LED pins\n");
      }

      PIN_setOutputValue(ledPinHandle, Board_LED0, 1);
     // Task_sleep(1000);
     // PIN_setOutputValue(ledPinHandle, Board_LED1, 1);
      PIN_setOutputValue(ledPinHandle, Board_LED2, 1);
      PIN_setOutputValue(ledPinHandle, Board_CP2103UARTRESET, 1);
      PIN_setOutputValue(ledPinHandle, Board_GENERALPURPOSE1, 1);

      // This example has logging and many other debug capabilities enabled
      System_printf("This example does not attempt to minimize code or data "
                    "footprint\n");
      System_flush();

      System_printf("Starting the UART Echo example\nSystem provider is set to "
                    "SysMin. Halt the target to view any SysMin contents in "
                    "ROV.\n");
      // SysMin will only print to the console when you call flush or exit
      System_flush();

      // Start BIOS
      BIOS_start();

      return (0);
  }
