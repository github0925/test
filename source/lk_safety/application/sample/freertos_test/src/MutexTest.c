
/*! ****************************************************************************
* .INCLUDES
********************************************************************************/
#include <debug.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"


/*! ****************************************************************************
* .DEFINES[local]
********************************************************************************/
#define MUTEX_0_TASK_STACK_SIZE	256	//Task stack size of words
#define MUTEX_1_TASK_STACK_SIZE	256	//Task stack size of words

#define STATIC_TEST

/*! ****************************************************************************
* .DATA_DEFINITIONS[all]
********************************************************************************/
static StackType_t xMutex0TaskStack[MUTEX_0_TASK_STACK_SIZE];	//Task stack
static StackType_t xMutex1TaskStack[MUTEX_1_TASK_STACK_SIZE];	//Task stack

static PRIVILEGED_DATA StaticTask_t xMutex0TaskBuffer;	//TCB
static PRIVILEGED_DATA StaticTask_t xMutex1TaskBuffer;	//TCB

SemaphoreHandle_t xSemaphore;
StaticSemaphore_t xMutexBuffer;

static TaskHandle_t xMutex0TaskHandle = NULL;
static TaskHandle_t xMutex1TaskHandle = NULL;


/******************************************************************************
 ** \brief Reset the sub timer counter value to 0.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] sub_timer  Sub timer index.
 ** \param [in] wait_rld   Whether wait for sub timer counter reset to 0.
 *****************************************************************************/
static void mutex_0_task(void * pvParameters)
{
	while(1)
	{
		xSemaphoreTake(xSemaphore, portMAX_DELAY);
		dprintf(INFO, "FreeRTOS test mutex_0_task take mutex!\n");
		xSemaphoreGive(xSemaphore);
		dprintf(INFO, "FreeRTOS test mutex_0_task give mutex!\n");
		dprintf(INFO, "FreeRTOS test Mutex0Task free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xMutex0TaskHandle));
		vTaskDelay(5);
	}
}

static void mutex_1_task(void * pvParameters)
{
	while(1)
	{
		xSemaphoreTake(xSemaphore, portMAX_DELAY);
		dprintf(INFO, "FreeRTOS test mutex_1_task take mutex!\n");
		xSemaphoreGive(xSemaphore);
		dprintf(INFO, "FreeRTOS test mutex_1_task give mutex!\n");
		dprintf(INFO, "FreeRTOS test Mutex1Task free stack size is %d!\r\n",(int32_t)uxTaskGetStackHighWaterMark(xMutex1TaskHandle));
		vTaskDelay(5);
	}
}

void mutex_test_init(void)
{
#ifdef STATIC_TEST
	xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);
#else
	xSemaphore = xSemaphoreCreateMutex();
#endif
}

void mutex_test_main(void)
{
#ifdef STATIC_TEST
	xMutex0TaskHandle = xTaskCreateStatic(
								mutex_0_task,			    /* The function that implements the task. */
								"Mutex0Task",			    /* Text name for the task. */
								MUTEX_0_TASK_STACK_SIZE,	/* Stack depth in words. */
								NULL,						/* Task parameters. */
								configMAX_PRIORITIES - 3, 	/* Priority and mode (user in this case). */
								xMutex0TaskStack,		    /* Used as the task's stack. */
								&xMutex0TaskBuffer		    /* Used to hold the task's data structure. */
							);

	xMutex1TaskHandle = xTaskCreateStatic(
								mutex_1_task, 			    /* The function that implements the task. */
								"Mutex1Task",			    /* Text name for the task. */
								MUTEX_1_TASK_STACK_SIZE,    /* Stack depth in words. */
								NULL,						/* Task parameters. */
								configMAX_PRIORITIES - 3, 	/* Priority and mode (user in this case). */
								xMutex1TaskStack,		    /* Used as the task's stack. */
								&xMutex1TaskBuffer 	        /* Used to hold the task's data structure. */
							);
#else
	xTaskCreate (
					mutex_0_task,			    /* The function that implements the task. */
					"Mutex0Task",			    /* Text name for the task. */
					MUTEX_0_TASK_STACK_SIZE,	/* Stack depth in words. */
					NULL,						/* Task parameters. */
					configMAX_PRIORITIES - 3, 	/* Priority and mode (user in this case). */
					&xMutex0TaskHandle,		    /* Task handle. */
				);

	xTaskCreate (
					mutex_1_task, 			    /* The function that implements the task. */
					"Mutex1Task",			    /* Text name for the task. */
					MUTEX_1_TASK_STACK_SIZE,    /* Stack depth in words. */
					NULL,						/* Task parameters. */
					configMAX_PRIORITIES - 3, 	/* Priority and mode (user in this case). */
					&xMutex1TaskHandle,		    /* Task handle. */
				);
#endif
}


