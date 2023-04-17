#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include <main.h>

void MacSender(void *argument)
{
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	union dataStruct *data;								//data
	for (;;)														  // loop until doomsday
	{
		//wait a Message
		osMessageQueueGet(queue_macS_id,&queueMsgR,NULL,osWaitForever);
		switch(queueMsgR.type)
		{
			case NEW_TOKEN:
				queueMsgS.type = TO_PHY;
				data = osMemoryPoolAlloc(memPool,0);
				//modift du token
				//
				data->token.addressToken=0xFF;
				
				for(int i = 0;i<16;i++)
				{
					data->token.user[i].userByte=0;
				}
					data->token.user[8].b1=1;
					data->token.user[8].b3=1;
				//
				queueMsgS.anyPtr=data;
				osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
				break;
			default :
				queueMsgS.type = MAC_ERROR;
				data = osMemoryPoolAlloc(memPool,0);
				//modift du token
				//
			
			
				//
				queueMsgS.anyPtr=data;
				osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,osWaitForever);
				break;
		}
	}
}
