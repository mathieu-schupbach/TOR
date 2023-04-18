#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include <main.h>

void MacSender(void *argument)
{
	//Declaration d'une queue intern
	
	const osMessageQueueAttr_t queue_macInt_attr = {
	.name = "macInt"  	
	};
	osMessageQueueId_t queue_macInt_id;
	queue_macInt_id = osMessageQueueNew(6,sizeof(struct queueMsg_t),&queue_macInt_attr);
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	struct queueMsg_t queueMsgT;					// queue message token
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
				
				for(int i = 0;i<15;i++)
				{
					data->token.user[i]=0x00;
				}
					data->token.user[7]=0x0A;
				//
				queueMsgS.anyPtr=data;
				osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
				break;
			case TOKEN:
			//chose data
			queueMsgR.anyPtr=data;
			//Check if update token list
			bool update =false;
			for(int i = 0;i<15;i++)
				{
					if(data->token.user[i]!=gTokenInterface.station_list[i])
					{
						update=true;
						gTokenInterface.station_list[i]=data->token.user[i];
					}
				}
				//If update crete the queue
				if(update)
				{
					queueMsgS.type=TOKEN_LIST;
					osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,osWaitForever);
				}
				//ckeck if as a message to send
				if(osMessageQueueGet(queue_macInt_id,&queueMsgR,NULL,0)==osOK)
				{
					queueMsgS.type = TO_PHY;

					queueMsgS.anyPtr=queueMsgR.anyPtr;
					data=queueMsgS.anyPtr;
					//data->fram.contolFram=data->fram.contolFram or 0x03
					osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
					osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				}
				break;
			case DATABACK:
				break;
			case START :
				break;
			case STOP:
				break;
			case DATA_IND:
				queueMsgS.type = TO_PHY;
				
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
