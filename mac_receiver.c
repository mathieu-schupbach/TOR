#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

void MacReceiver(void *argument)
{
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	dataStruct *dataR;							//data revie
	dataStruct *dataS;							//data send
	for (;;)															// loop until doomsday
	{
		//Recive a message 
		osMessageQueueGet(queue_macR_id,&queueMsgR,NULL,osWaitForever);
		//check type
		switch(queueMsgR.type)
		{
			case FROM_PHY:
			dataR=queueMsgR.anyPtr;
			//check if is a token
				if(dataR->token.addressToken==TOKEN_TAG)
				{
					queueMsgS.type = TOKEN;
					queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
					 *((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
					osMessageQueuePut(queue_macS_id,&queueMsgS,osPriorityNormal,osWaitForever);
					osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				}
				break;
			default :
				break;
		}
	}
}
