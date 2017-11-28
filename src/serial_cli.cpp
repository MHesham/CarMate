/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#include <Arduino_FreeRTOS.h>
#include <FreeRTOS_CLI.h>
#include <lock.h>
#include "carmate.h"
#include "tasks_config.h"
#include "utils.h"

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE 50

/* Dimentions a buffer to be used by the UART driver, if the UART driver uses a
buffer at all. */
#define cmdQUEUE_LENGTH 16

/* DEL acts as a backspace. */
#define cmdASCII_DEL (0x7F)

/* The maximum time to wait for the mutex that guards the UART to become
available. */
#define cmdMAX_MUTEX_WAIT_MS 300
#define cmdMAX_RX_WAIT_MS 300

#ifndef configCLI_BAUD_RATE
#define configCLI_BAUD_RATE 9600
#endif

/*-----------------------------------------------------------*/

/*
 * The task that implements the command console processing.
 */
static void prvSerialConsoleTask(void *pvParameters);
void vSerialConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority);

/*-----------------------------------------------------------*/

/* Const messages output by the command console. */
static const char pcWelcomeMessage[] PROGMEM =
    "FreeRTOS command server.\r\nType "
    "Help to view a list of registered "
    "commands.\r\n\r\n>";
static const char pcEndOfOutputMessage[] PROGMEM =
    "\r\n[Press ENTER to execute the previous command again]\r\n>";
static const char pcNewLine[] PROGMEM = "\r\n";

/* Used to guard access to the UART in case messages are sent to the UART from
more than one task. */
static SemaphoreHandle_t xTxMutex = NULL;
static QueueHandle_t xRxQueue = NULL;

/*-----------------------------------------------------------*/

void vSerialConsoleInit() {

  Serial1.begin(configCLI_BAUD_RATE);
	/* Initialise the UART. */
	while (!Serial1)
  {

  }

  /* Create the semaphore used to access the UART Tx. */
  xTxMutex = xSemaphoreCreateMutex();
  configASSERT(xTxMutex);

  xRxQueue = xQueueCreate(cmdQUEUE_LENGTH, sizeof(char));
  configASSERT(xRxQueue);

  /* Create that task that handles the console itself. */
  xTaskCreate(
      prvSerialConsoleTask, /* The task that implements the command
                                    console. */
      tskcfgCONSOLE_TASK_NAME, /* Text name assigned to the task.  This is just to assist
                debugging.  The kernel does not use this name itself. */
      tskcfgCONSOLE_TASK_STACK_SIZE, /* The size of the stack allocated to the task.
                                  */
      NULL,                  /* The parameter is not used, so NULL is passed. */
      tskcfgCONSOLE_TASK_PRIORITY, /* The priority allocated to the task. */
      NULL);                 /* A handle is not required, so just pass NULL. */

  vPrintf_P(PSTR("Serial console started\n"));
}
/*-----------------------------------------------------------*/

void serialEvent1() {
  while (Serial1.available()) {
    char cRxedChar = Serial1.read();
    xQueueSend(xRxQueue, &cRxedChar, cmdMAX_RX_WAIT_MS);
  }
}

char prvSerialReadChar(void) {
  char cRxedChar;
  xQueueReceive(xRxQueue, &cRxedChar, portMAX_DELAY);
  return cRxedChar;
}

void prvPrintfNoLock_P(PGM_P format, ...) {
  va_list arg;
  char *pcOutputString = FreeRTOS_CLIGetOutputBuffer();

  va_start(arg, format);
  vsnprintf_P(pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE,
              (const char *)format, arg);
  Serial1.print(pcOutputString);
  va_end(arg);
}

void prvSerialWriteChar(char c) {
  LockGuard lk(xTxMutex);
  Serial1.print(c);
}

static void prvSerialConsoleTask(void *pvParameters) {
  char cRxedChar;
  char *pcOutputString;
  UniqueLock txLock(xTxMutex, DeferLock);
  uint8_t ucInputIndex = 0;
  static char cInputString[cmdMAX_INPUT_SIZE];
  static char cLastInputString[cmdMAX_INPUT_SIZE];
  BaseType_t xReturned;

  (void)pvParameters;

  xEventGroupWaitBits(xSystemEvents, carmateSYSTEM_UP_EVT_BIT, pdFALSE, pdTRUE,
                      portMAX_DELAY);

  /* Obtain the address of the output buffer.  Note there is no mutual
  exclusion on this buffer as it is assumed only one command console interface
  will be used at any one time. */
  pcOutputString = FreeRTOS_CLIGetOutputBuffer();

  vPrintf_P(pcWelcomeMessage);

  for (;;) {
    cRxedChar = prvSerialReadChar();

    /* Ensure exclusive access to the UART Tx. */
    if (txLock.TryLockFor(cmdMAX_MUTEX_WAIT_MS)) {
      Serial1.print(cRxedChar);

      /* Was it the end of the line? */
      if (cRxedChar == '\n' || cRxedChar == '\r') {
        /* Just to space the output from the input. */
        prvPrintfNoLock_P(pcNewLine);

        /* See if the command is empty, indicating that the last command
        is to be executed again. */
        if (ucInputIndex == 0) {
          /* Copy the last command back into the input string. */
          strcpy(cInputString, cLastInputString);
        }

        /* Pass the received command to the command interpreter.  The
        command interpreter is called repeatedly until it returns
        pdFALSE  (indicating there is no more output) as it might
        generate more than one string. */
        do {
          /* Get the next output string from the command interpreter. */
          xReturned = FreeRTOS_CLIProcessCommand(
              cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE);

          /* Write the generated string to the UART. */
          Serial1.print(pcOutputString);
        } while (xReturned != pdFALSE);

        /* All the strings generated by the input command have been
        sent.  Clear the input string ready to receive the next command.
        Remember the command that was just processed first in case it is
        to be processed again. */
        strcpy(cLastInputString, cInputString);
        ucInputIndex = 0;
        memset(cInputString, 0x00, cmdMAX_INPUT_SIZE);

        prvPrintfNoLock_P(pcEndOfOutputMessage);
      } else {
        if (cRxedChar == '\r') {
          /* Ignore the character. */
        } else if ((cRxedChar == '\b') || (cRxedChar == cmdASCII_DEL)) {
          /* Backspace was pressed.  Erase the last character in the
          string - if any. */
          if (ucInputIndex > 0) {
            ucInputIndex--;
            cInputString[ucInputIndex] = '\0';
          }
        } else {
          /* A character was entered.  Add it to the string entered so
          far.  When a \n is entered the complete  string will be
          passed to the command interpreter. */
          if ((cRxedChar >= ' ') && (cRxedChar <= '~')) {
            if (ucInputIndex < cmdMAX_INPUT_SIZE) {
              cInputString[ucInputIndex] = cRxedChar;
              ucInputIndex++;
            }
          }
        }
      }

      /* Must ensure to give the mutex back. */
      txLock.Unlock();
    }
  }
}

void vPrintf_P(PGM_P format, ...) {
  LockGuard lk(xTxMutex);
  va_list arg;
  char *pcOutputString = FreeRTOS_CLIGetOutputBuffer();

  va_start(arg, format);
  vsnprintf_P(pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE,
              (const char *)format, arg);
  Serial1.print(pcOutputString);
  va_end(arg);
}

/*-----------------------------------------------------------*/
