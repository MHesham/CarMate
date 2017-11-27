/*
    FreeRTOS V8.2.0rc1 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?".  Have you defined configASSERT()?  *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

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

    ***************************************************************************
     *                                                                       *
     *   Investing in training allows your team to be as productive as       *
     *   possible as early as possible, lowering your overall development    *
     *   cost, and enabling you to bring a more robust product to market     *
     *   earlier than would otherwise be possible.  Richard Barry is both    *
     *   the architect and key author of FreeRTOS, and so also the world's   *
     *   leading authority on what is the world's most popular real time     *
     *   kernel for deeply embedded MCU designs.  Obtaining your training    *
     *   from Richard ensures your team will gain directly from his in-depth *
     *   product knowledge and years of usage experience.  Contact Real Time *
     *   Engineers Ltd to enquire about the FreeRTOS Masterclass, presented  *
     *   by Richard Barry:  http://www.FreeRTOS.org/contact
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *    You are receiving this top quality software for free.  Please play *
     *    fair and reciprocate by reporting any suspected issues and         *
     *    participating in the community forum:                              *
     *    http://www.FreeRTOS.org/support                                    *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/******************************************************************************
*
* See the following URL for information on the commands defined in this file:
* http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_UDP/Embedded_Ethernet_Examples/Ethernet_Related_CLI_Commands.shtml
*
******************************************************************************/

/* FreeRTOS includes. */
#include "Arduino_FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include <FreeRTOS_CLI.h>

/*
 * Implements the run-time-stats command.
 */
static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer,
                                      size_t xWriteBufferLen,
                                      const char *pcCommandString);

/*
 * Implements the task-stats command.
 */
static BaseType_t prvRunTimeStatsCommand(char *pcWriteBuffer,
                                         size_t xWriteBufferLen,
                                         const char *pcCommandString);

/*
 * Implements that mem-stats command.
 */
static BaseType_t prvMemStatsCommand(char *pcWriteBuffer,
                                     size_t xWriteBufferLen,
                                     const char *pcCommandString);

/*
 * Implements the "trace start" and "trace stop" commands;
 */
#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1
static BaseType_t prvStartStopTraceCommand(char *pcWriteBuffer,
                                           size_t xWriteBufferLen,
                                           const char *pcCommandString);
#endif

/* Structure that defines the "run-time-stats" command line command.   This
generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t xRunTimeStats = {
    "run-time-stats", /* The command string to type. */
    "\r\nrun-time-stats:\r\n Displays a table showing how much processing time "
    "each FreeRTOS task has used\r\n",
    prvRunTimeStatsCommand, /* The function to run. */
    0                       /* No parameters are expected. */
};

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xTaskStats = {
    "task-stats", /* The command string to type. */
    "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS "
    "task\r\n",
    prvTaskStatsCommand, /* The function to run. */
    0                    /* No parameters are expected. */
};

/* Structure that defines the "mem-stats" command line command.  This generates
a table that gives information on the heap free space and usage. */
static const CLI_Command_Definition_t xMemStats = {
    "mem-stats", /* The command string to type. */
    "\r\nmem-stats:\r\n Displays a table showing the state FreeRTOS heap\r\n",
    prvMemStatsCommand, /* The function to run. */
    0                   /* No parameters are expected. */
};

void vRegisterCLICommands(void) {
  /* Register all the command line commands defined immediately above. */
  FreeRTOS_CLIRegisterCommand(&xTaskStats);
  FreeRTOS_CLIRegisterCommand(&xRunTimeStats);
  FreeRTOS_CLIRegisterCommand(&xMemStats);
}
/*-----------------------------------------------------------*/

static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer,
                                      size_t xWriteBufferLen,
                                      const char *pcCommandString) {
  static const char cHeader[] PROGMEM =
      "Task          State  Priority  Stack	"
      "#\r\n************************************************\r\n";

  /* Remove compile time warnings about unused parameters, and check the
  write buffer is not NULL.  NOTE - for simplicity, this example assumes the
  write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  /* Generate a table of task stats. */
  strcpy_P(pcWriteBuffer, cHeader);
  vTaskList(pcWriteBuffer + strlen_P(cHeader));

  /* There is no more data to return after this single string, so return
  pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvRunTimeStatsCommand(char *pcWriteBuffer,
                                         size_t xWriteBufferLen,
                                         const char *pcCommandString) {
  static const char cHeader[] PROGMEM =
      "Task            Abs Time      % "
      "Time\r\n****************************************\r\n";

  /* Remove compile time warnings about unused parameters, and check the
  write buffer is not NULL.  NOTE - for simplicity, this example assumes the
  write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  /* Generate a table of task stats. */
  strcpy_P(pcWriteBuffer, cHeader);
  vTaskGetRunTimeStats(pcWriteBuffer + strlen_P(cHeader));

  /* There is no more data to return after this single string, so return
  pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvMemStatsCommand(char *pcWriteBuffer,
                                     size_t xWriteBufferLen,
                                     const char *pcCommandString) {
  /* Remove compile time warnings about unused parameters, and check the
  write buffer is not NULL.  NOTE - for simplicity, this example assumes the
  write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  size_t heapFreeSpace = xPortGetFreeHeapSize();
  size_t heapMinimumSpace = xPortGetMinimumEverFreeHeapSize();
  snprintf_P(pcWriteBuffer, xWriteBufferLen,
             PSTR("Total Space:%u\r\nFree Space:%u\r\nMin Space:%u\r\n"),
             (uint32_t)configTOTAL_HEAP_SIZE, (uint32_t)heapFreeSpace,
             (uint32_t)heapMinimumSpace);

  /* There is no more data to return after this single string, so return
  pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/