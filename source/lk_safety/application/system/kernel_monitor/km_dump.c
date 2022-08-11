/* Standard includes. */
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#if (SDRV_KERNEL_MONITOR == 1 && configUSE_TRACE_FACILITY == 1 && configGENERATE_RUN_TIME_STATS == 1)

static inline const char* sdrvReturnTaskState(eTaskState state)
{
	switch(state)
	{
	case eRunning:
		return "InRun";
	case eReady:
		return "Ready";
	case eBlocked:
		return "Block";
	case eSuspended:
		return "Suspd";
	case eDeleted:
		return "Wfdlt";
	case eInvalid:
		return "Invad";
	default: return "Error";
	}
}

static void dumpSystemStates(void)
{
    TaskStatus_t* pTskStas = NULL;

    uint32_t uTotalTime = 0;
    uint32_t index = 0;
    UBaseType_t  uTskNum = uxTaskGetNumberOfTasks();
    pTskStas = malloc(uTskNum * sizeof(TaskStatus_t));
    TaskStatus_t* pEntry = pTskStas;

    SKTrace("%-20s State CurrPri BasePri Time  StackBase   HighWater  CPU TCB(Create Seq)\n","Task Name");
    if(pTskStas != NULL){
        uTskNum = uxTaskGetSystemState((TaskStatus_t*)pTskStas, (UBaseType_t)uTskNum,&uTotalTime);
        for(index = 0; index<uTskNum; index++){
            SKTrace("%-20s %-5s %-7d %-7d %-5d %-8p  0x%-7x %3d%% %-8p(%-3d)\r\n",
            pTskStas->pcTaskName,
            sdrvReturnTaskState(pTskStas->eCurrentState),
            (uint32_t)pTskStas->uxCurrentPriority,
            (uint32_t)pTskStas->uxBasePriority,
            (uint32_t)pTskStas->ulRunTimeCounter/(1000),
            pTskStas->pxStackBase,
            pTskStas->usStackHighWaterMark,
            (uint32_t)100UL*pTskStas->ulRunTimeCounter/uTotalTime,
            pTskStas->xHandle,
            (uint32_t)pTskStas->xTaskNumber
            );
            pTskStas++;
        }
    }
    free(pEntry);
}

void sdvrTaskStateInfo(void)
{
    SKTrace("------------State abbr.------------\n");
	SKTrace("InRun   - Current running task\n");
	SKTrace("Ready   - Next ready task\n");
	SKTrace("Block   - Task was blocked\n");
	SKTrace("Suspd   - Task was suspended\n");
	SKTrace("Wfdlt   - task was deleted but not recycled\n");
	SKTrace("Invad   - Unexpected state\n");
	SKTrace("Error   - Unexpected state\n");
	SKTrace("Deleted - Task was deleted and recycled\n");
	SKTrace("-----------------------------------\n");
	SKTrace("Use top command to show task stats\n");
}

void sdrvDumpTaskInfo( void )
{
    dumpSystemStates();
}

#endif
