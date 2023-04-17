#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

void MacReceiver(void *argument)
{
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	union dataStruct *data;								//data
	for (;;)															// loop until doomsday
	{
		//Recive a message 
		osMessageQueueGet(queue_macR_id,&queueMsgR,NULL,osWaitForever);
		//check type
		switch(queueMsgR.type)
		{
			case FROM_PHY:
				data=queueMsgR.anyPtr;
			//check if is a token
				if(data.isToken==1)
				{
					queueMsgS.type = TOKEN;
					queueMsgS.anyPtr =queueMsgR.anyPtr;
					osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
					osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				}
				break;
			default :

				break;
		}
	}
}
