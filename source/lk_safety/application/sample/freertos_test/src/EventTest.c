
/*! ****************************************************************************
* .INCLUDES
********************************************************************************/
#include <debug.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/*! ****************************************************************************
* .DEFINES[local]
********************************************************************************/
#define EVENT_SEND_TASK_STACK_SIZE	256	//Task stack size of words
#define EVENT_RECV_TASK_STACK_SIZE	256	//Task stack size of words

#define STATIC_TEST

/*! ****************************************************************************
* .DATA_DEFINITIONS[all]
********************************************************************************/
static StackType_t xEventSendTaskStack[EVENT_SEND_TASK_STACK_SIZE];	//Task stack
static StackType_t xEventRecvTaskStack[EVENT_RECV_TASK_STACK_SIZE];	//Task stack

static PRIVILEGED_DATA StaticTask_t xEventSendTaskBuffer;	//Task Control Block
static PRIVILEGED_DATA StaticTask_t xEventRecvTaskBuffer;	//Task Control Block

static StaticEventGroup_t xEventGroupBuffer;
static EventGroupHandle_t xEventGroup;

static TaskHandle_t xEventSendTaskHandle = NULL;
static TaskHandle_t xEventRecvTaskHandle = NULL;


/******************************************************************************
 ** \brief Reset the sub timer counter value to 0.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] sub_timer  Sub timer index.
 ** \param [in] wait_rld   Whether wait for sub timer counter reset to 0.
 *****************************************************************************/
static void event_send_task(void * pvParameters)
{
	EventBits_t uxBits;

	while(1)
	{
		// Set bit 0 and bit 4 in xEventGroup.
		uxBits = xEventGroupSetBits(xEventGroup,	// The event group being updated.
									0x1 | 0x04 );	// The bits being set.

		if( ( uxBits & ( 0x1 | 0x04 ) ) == ( 0x1 | 0x04 ) )
		{
			// Both bit 0 and bit 4 remained set when the function returned.
			dprintf(INFO, "FreeRTOS test event set returned because both bits were set!\r\n");
		}
		else if( ( uxBits & 0x1 ) != 0 )
		{
			// Bit 0 remained set when the function returned, but bit 4 was
			// cleared.  It might be that bit 4 was cleared automatically as a
			// task that was waiting for bit 4 was removed from the Blocked
			// state.
			dprintf(INFO, "FreeRTOS test event set returned because bit 0 were set!\r\n");
		}
		else if( ( uxBits & 0x04 ) != 0 )
		{
			// Bit 4 remained set when the function returned, but bit 0 was
			// cleared.  It might be that bit 0 was cleared automatically as a
			// task that was waiting for bit 0 was removed from the Blocked
			// state.
			dprintf(INFO, "FreeRTOS test event set returned because bit 2 were set!\r\n");
		}
		else
		{
			// Neither bit 0 nor bit 4 remained set.  It might be that a task
			// was waiting for both of the bits to be set, and the bits were
			// cleared as the task left the Blocked state.
			dprintf(INFO, "FreeRTOS test event set returned without bit set!\r\n");
		}

		dprintf(INFO, "FreeRTOS test EventSendTask free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xEventSendTaskHandle));

		vTaskDelay(5);
	}
}

static void event_recv_task(void * pvParameters)
{
	EventBits_t uxBits;

	while(1)
	{
		uxBits = xEventGroupWaitBits(
						xEventGroup,	// The event group being tested.
						0x1 | 0x04,		// The bits within the event group to wait for.
						pdTRUE,			// BIT_0 and BIT_4 should be cleared before returning.
						pdFALSE,		// Don't wait for both bits, either bit will do.
						portMAX_DELAY );// Wait ticks.

		if( ( uxBits & ( 0x1 | 0x04 ) ) == ( 0x1 | 0x04 ) )
		{
			// xEventGroupWaitBits() returned because both bits were set.
			dprintf(INFO, "FreeRTOS test event wait returned because both bits were set!\r\n");
		}
		else if( ( uxBits & 0x1 ) != 0 )
		{
			// xEventGroupWaitBits() returned because just BIT_0 was set.
			dprintf(INFO, "FreeRTOS test event wait returned because bit 0 were set!\r\n");
		}
		else if( ( uxBits & 0x04 ) != 0 )
		{
			// xEventGroupWaitBits() returned because just BIT_4 was set.
			dprintf(INFO, "FreeRTOS test event wait returned because bit 2 were set!\r\n");
		}
		else
		{
			// xEventGroupWaitBits() returned because xTicksToWait ticks passed
			// without either BIT_0 or BIT_4 becoming set.
			dprintf(INFO, "FreeRTOS test event wait returned without bit set!\r\n");
		}

		dprintf(INFO, "FreeRTOS test EventRecvTask free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xEventRecvTaskHandle));
	}
}

void event_test_init(void)
{
#ifdef STATIC_TEST
	xEventGroup = xEventGroupCreateStatic(&xEventGroupBuffer);
#else
	xEventGroup = xEventGroupCreate();
#endif
}

void event_test_main(void)
{
#ifdef STATIC_TEST
	xEventSendTaskHandle = xTaskCreateStatic(
								&event_send_task,			/* The function that implements the task. */
								"EventSendTask",			/* Text name for the task. */
								EVENT_SEND_TASK_STACK_SIZE,	/* Stack depth in words. */
								NULL,						/* Task parameters. */
								configMAX_PRIORITIES - 4,	/* Priority and mode (user in this case). */
								xEventSendTaskStack,		/* Used as the task's stack. */
								&xEventSendTaskBuffer		/* Used to hold the task's data structure. */
					 		);

	xEventRecvTaskHandle = xTaskCreateStatic(
								&event_recv_task,			/* The function that implements the task. */
								"EventRecvTask",			/* Text name for the task. */
								EVENT_RECV_TASK_STACK_SIZE,	/* Stack depth in words. */
								NULL,						/* Task parameters. */
								configMAX_PRIORITIES - 4,	/* Priority and mode (user in this case). */
								xEventRecvTaskStack,		/* Used as the task's stack. */
								&xEventRecvTaskBuffer		/* Used to hold the task's data structure. */
					 		);
#else
	xTaskCreate (
					&event_send_task,			/* The function that implements the task. */
					"EventSendTask",			/* Text name for the task. */
					EVENT_SEND_TASK_STACK_SIZE,	/* Stack depth in words. */
					NULL,						/* Task parameters. */
					configMAX_PRIORITIES - 4,	/* Priority and mode (user in this case). */
					&xEventSendTaskHandle,		/* Task handle. */
				);

	xTaskCreate (
					&event_recv_task,			/* The function that implements the task. */
					"EventRecvTask",			/* Text name for the task. */
					EVENT_RECV_TASK_STACK_SIZE,	/* Stack depth in words. */
					NULL,						/* Task parameters. */
					configMAX_PRIORITIES - 4,	/* Priority and mode (user in this case). */
					&xEventRecvTaskHandle,		/* Task handle. */
				);
#endif
}


