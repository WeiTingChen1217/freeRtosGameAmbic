//*****************************************************************************
//
//! @file rtos.c
//!
//! @brief Essential functions to make the RTOS run correctly.
//!
//! These functions are required by the RTOS for ticking, sleeping, and basic
//! error checking.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2020, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 2.5.1 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>


#include "am_mcu_apollo.h"
#include "am_bsp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "freertos_lowpower.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;
TaskHandle_t xTaskHandle_LedTask;
TaskHandle_t xTaskHandle_start;
TaskHandle_t xTaskHandle_question;



//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    //
    // Check the timer interrupt status.
    //
    ui32Status = am_hal_ctimer_int_status_get(false);
    am_hal_ctimer_int_clear(ui32Status);

    //
    // Run handlers for the various possible timer events.
    //
    am_hal_ctimer_int_service(ui32Status);
}

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
}


//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    while (1);
}

void
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
QueueHandle_t xQueue;
static void vReceiverTask( void *pvParameters )
{
	char *pcTaskName;
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = ( char * ) pvParameters;
	/* Print out the name of this task. */
	am_util_debug_printf( pcTaskName );
	
	/* Declare the variable that will hold the values received from the queue. */
	sender_t lReceivedValue;
	BaseType_t xStatus;
	const TickType_t xTicksToWait = pdMS_TO_TICKS( 500UL );
	
	long time_start;
	long time_stop;
	
	Data_t Scoreboard[10];
	for(int i = 0; i < sizeof(Scoreboard)/sizeof(Data_t); i++){
		Scoreboard[i].pcTextForUser = 0;
		Scoreboard[i].pushTime = 0.0;
	}

	/* This task is also defined within an infinite loop. */
	for( ;; )
	{
		xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

		if( xStatus == pdPASS )
		{
			if(lReceivedValue == timer_strat){
				time_start = xTaskGetTickCount();
				am_util_debug_printf("Go!! \r\n");

			}
			else if(lReceivedValue == settlement){
				am_util_debug_printf("Settlement \r\n");
				vTaskSuspend(xTaskHandle_question);
				
				// Scoreboard
				// User1 Score
				int User1_number = 0;
				int User2_number = 0;
				long User1_score = 0.0;
				long User2_score = 0.0;

				for(int i = 0; i < sizeof(Scoreboard); i++){
					if(Scoreboard[i].pcTextForUser == User1){
						User1_number++;
						User1_score += Scoreboard[i].pushTime;
					}else if(Scoreboard[i].pcTextForUser == User2){
						User2_number++;
						User2_score += Scoreboard[i].pushTime;
					}
				}
				am_util_debug_printf("User1 Score is %ld ms \r\n", User1_score/User1_number);
				am_util_debug_printf("User2 Score is %ld ms \r\n", User2_score/User2_number);
				if(User1_score < User2_score){
					am_util_debug_printf("The Winner is User1 \r\n");
				}else if(User1_score > User2_score){
					am_util_debug_printf("The Winner is User2 \r\n");
				}else{
					am_util_debug_printf("No one win this GAME \r\n");
				}
					
				
				// Reset the peramiters
				restart = 0;
				numbe_of_the_question = 0;
				// Scoreboard
				for(int i = 0; i < sizeof(Scoreboard)/sizeof(Data_t); i++){
					Scoreboard[i].pcTextForUser = 0;
					Scoreboard[i].pushTime = 0.0;
				}
				am_devices_led_off(am_bsp_psLEDs, 0);
				am_devices_led_off(am_bsp_psLEDs, 1);
				am_devices_led_off(am_bsp_psLEDs, 2);
				am_devices_led_off(am_bsp_psLEDs, 3);
				am_devices_led_off(am_bsp_psLEDs, 4);
			}
			else{
				time_stop = xTaskGetTickCount() - time_start;
				am_util_debug_printf("TIME SPEND:%ld \r\n", time_stop);
				
				Scoreboard[numbe_of_the_question].pcTextForUser = lReceivedValue;
				Scoreboard[numbe_of_the_question].pushTime 		= time_stop;
			}
		}
		else
		{
			/* We did not receive anything from the queue even after waiting for 100ms.
			This must be an error as the sending tasks are free running and will be
			continuously writing to the queue. */
//			am_util_debug_printf("-\r\n");
		}
	}
}


int numbe_of_the_question = 0;
void
task_question(void *pvParameters){	
	char *pcTaskName;
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = ( char * ) pvParameters;
	/* Print out the name of this task. */
	am_util_debug_printf( pcTaskName );
	
	sender_t sendMessage;
	
	BaseType_t xStatus;
	
	srand(xTaskGetTickCount());
	int min = 1;
	int max = 5;
	TickType_t x_rand = 1000 / portTICK_PERIOD_MS;

	vTaskSuspend(xTaskHandle_question);
	

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		if(numbe_of_the_question > 10){
			am_util_debug_printf("**** THE GAME IS OVER **** \r\n");
			
			sendMessage = settlement;
			xStatus = xQueueSendToBack( xQueue, &sendMessage, 0 );
			if( xStatus != pdPASS )
			{
				am_util_debug_printf( "Could not send to the queue.\r\n" );
			}
			vTaskSuspend(xTaskHandle_question);
		}else{
			am_util_debug_printf("xTaskGetTickCount: %d\t%d \r\n", xTaskGetTickCount(), xTaskGetTickCount()%5);
			x_rand = (rand() % (max - min + 1) + min) * 1000 / portTICK_PERIOD_MS;
			am_util_debug_printf("%d*** Rand %d mini second", numbe_of_the_question++, x_rand);
			am_util_debug_printf("Ready.....");
			
			// Susspend it self wait the next round
			vTaskSuspend(xTaskHandle_LedTask);
			vTaskDelay(x_rand);
			
			sendMessage = timer_strat;
			xStatus = xQueueSendToBack( xQueue, &sendMessage, 0 );
			if( xStatus != pdPASS )
			{
				/* We could not write to the queue because it was full ?this must
				be an error as the queue should never contain more than one item! */
				am_util_debug_printf( "Could not send to the queue.\r\n" );
			}


			vTaskResume(xTaskHandle_LedTask);
			vTaskDelay(1000);	// Response Time
		}
	}
}

bool restart = 0;
void
task_main(void *pvParameters)
{	
	uint32_t bitSet;

	while (1)
	{
		//
		// Wait for an event to be posted to the LED Event Handle.
		//
		bitSet = xEventGroupWaitBits(xLedEventHandle, 0x7, pdTRUE,
							pdFALSE, portMAX_DELAY);
		if (bitSet != 0)
		{
			if (restart ==0 && bitSet & (1 << 0))
			{
				am_devices_led_toggle(am_bsp_psLEDs, 4);
				vTaskResume(xTaskHandle_question);
				restart = 1;
			}
			
		}
	}
}

//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
const char *pcTextForTask_question = "!! Create a Task: task_question\r\n";
void
run_tasks(void)
{
	LedTaskSetup();

	/* The queue is created to hold a maximum of 5 long values. */
    xQueue = xQueueCreate( 1, sizeof( long ) );

	if( xQueue != NULL ){
		//
		// Create the functional tasks
		//		
		xTaskCreate(task_main, "task_main", 512, NULL, 0, &xTaskHandle_start);
		xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 0, NULL);
		xTaskCreate(task_question, "task_question", 512, (void*)pcTextForTask_question, 1, &xTaskHandle_question);
		xTaskCreate(LedTask, "LedTask", 1000, NULL, 1, &xTaskHandle_LedTask);
	}else{
		/* The queue could not be created. */
		am_util_debug_printf( "queue create ERROR!!\r\n" );
	}
   

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();
}


