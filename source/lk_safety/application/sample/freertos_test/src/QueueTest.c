

/*! ****************************************************************************
* .INCLUDES
********************************************************************************/
#include <debug.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define STATIC_TEST

/*! ****************************************************************************
* .DEFINES[local]
********************************************************************************/
#define configQSendTaskStackSize	256	//Task stack size of words
#define configQRecvTaskStackSize	256	//Task stack size of words

/*! ****************************************************************************
* .DATA_TYPES[local]
********************************************************************************/

/*! ****************************************************************************
* .DATA_DEFINITIONS[all]
********************************************************************************/
static StackType_t xQSendTaskStack[configQSendTaskStackSize];	//Task stack
static StackType_t xQRecvTaskStack[configQRecvTaskStackSize];	//Task stack

static PRIVILEGED_DATA StaticTask_t xQSendTaskBuffer;	//Task control block
static PRIVILEGED_DATA StaticTask_t xQRecvTaskBuffer;	//Task control block

static TaskHandle_t xQSendTaskHandle = NULL;
static TaskHandle_t xQRecvTaskHandle = NULL;

static QueueHandle_t xQHandle = NULL;	//Queue handle

static StaticQueue_t xQStatic;
static uint8_t xQStorage[10*sizeof(uint32_t)];

/******************************************************************************
 ** \brief Reset the sub timer counter value to 0.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] sub_timer  Sub timer index.
 ** \param [in] wait_rld   Whether wait for sub timer counter reset to 0.
 *****************************************************************************/
static void vQueueSendTask(void * pvParameters)
{
	static uint32_t ulValueToSend = 0;

	while(1)
	{
		if(xQHandle != NULL)
		{
			if(xQueueSend(xQHandle, (void *)&ulValueToSend, (TickType_t)0) != pdPASS)
			{
				dprintf(INFO, "FreeRTOS test send queue fail!\r\n");
			}
			else
			{
				ulValueToSend++;
				dprintf(INFO, "FreeRTOS test send queue success!\r\n");
			}
			dprintf(INFO, "FreeRTOS test QueueSendTask free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xQSendTaskHandle));
			vTaskDelay(5);
		}
		else
		{
			vTaskDelete(xQSendTaskHandle);
		}
	}
}

static void vQueueRecvTask(void * pvParameters)
{
	static uint32_t ulRecvValue;

	while(1)
	{
		if(xQueueReceive(xQHandle, (void *)&ulRecvValue, portMAX_DELAY) == pdPASS)
		{
			dprintf(INFO, "FreeRTOS test received queue success: %d!\r\n", ulRecvValue);
		}

		dprintf(INFO, "FreeRTOS test QueueRecvTask free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xQRecvTaskHandle));
	}
}

void queue_test_init(void)
{
#ifdef STATIC_TEST
	xQHandle = xQueueCreateStatic(10, sizeof(uint32_t), xQStorage, &xQStatic);
#else
	xQHandle = xQueueCreate(10, sizeof(uint32_t));
#endif
}

void queue_test_main(void)
{
#ifdef STATIC_TEST
	xQSendTaskHandle = xTaskCreateStatic (
											vQueueSendTask,				/* The function that implements the task. */
											"Queue Send Task",			/* Text name for the task. */
											configQSendTaskStackSize,	/* Stack depth in words. */
											NULL,						/* Task parameters. */
											configMAX_PRIORITIES - 2,	/* Priority and mode (user in this case). */
											xQSendTaskStack,			/* Used as the task's stack. */
											&xQSendTaskBuffer			/* Used to hold the task's data structure. */
					 					 );

	xQRecvTaskHandle = xTaskCreateStatic (
											vQueueRecvTask,				/* The function that implements the task. */
											"Queue Recv Task",			/* Text name for the task. */
											configQRecvTaskStackSize,	/* Stack depth in words. */
											NULL,						/* Task parameters. */
											configMAX_PRIORITIES - 2,	/* Priority and mode (user in this case). */
											xQRecvTaskStack,			/* Used as the task's stack. */
											&xQRecvTaskBuffer			/* Used to hold the task's data structure. */
					 					 );
#else
	xTaskCreate (
				  	vQueueSendTask,				/* The function that implements the task. */
				  	"Queue Send Task",			/* Text name for the task. */
				  	configQSendTaskStackSize,	/* Stack depth in words. */
				  	NULL,						/* Task parameters. */
				  	configMAX_PRIORITIES - 2, 	/* Priority and mode (user in this case). */
				  	&xQSendTaskHandle,			/* Task handle. */
				);

	xTaskCreate (
				  	vQueueRecvTask,				/* The function that implements the task. */
				  	"Queue Recv Task",			/* Text name for the task. */
				  	configQRecvTaskStackSize,	/* Stack depth in words. */
				  	NULL,						/* Task parameters. */
				  	configMAX_PRIORITIES - 2, 	/* Priority and mode (user in this case). */
				  	&xQRecvTaskHandle,			/* Task handle. */
				);
#endif
}






