//*****************************************************************************
//
//! @file freertos_fit.h
//!
//! @brief Global includes for the freertos_fit app.
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

#ifndef FREERTOS_LOWPOWER_H
#define FREERTOS_LOWPOWER_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

// weiting ADD
#include <time.h>


//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"


//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "led_task.h"

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void disable_print_interface(void);

//WEITING ADD
extern TaskHandle_t xTaskHandle_question;
extern TaskHandle_t xTaskHandle_LedTask;

extern bool restart;
extern int numbe_of_the_question;
extern char *text_go;
/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
extern QueueHandle_t xQueue;
typedef enum
{
	User1 = 1,
	User2,
	settlement,
	timer_strat
} sender_t;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
	sender_t pcTextForUser;
	long pushTime;
} Data_t;



#endif // FREERTOS_LOWPOWER_H
