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
	union dataStruct *dataStrct;								//data
	struct dataFram	 *data     ;
	//initalisation
	gTokenInterface.myAddress=8;
	queueMsgS.type=TOKEN_LIST;
	osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,osWaitForever);
	for (;;)														  // loop until doomsday
	{
		//wait a Message
		osMessageQueueGet(queue_macS_id,&queueMsgR,NULL,osWaitForever);
		switch(queueMsgR.type)
		{
			case NEW_TOKEN:
				queueMsgS.type = TO_PHY;
				dataStrct = osMemoryPoolAlloc(memPool,0);
				//modift du token
				//
				dataStrct->token.addressToken=0xFF;
				
				for(int i = 0;i<15;i++)
				{
					dataStrct->token.user[i]=0x00;
				}
				//dit si je suis connect ou pas
					dataStrct->token.user[gTokenInterface.myAddress-1]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
				//
				queueMsgS.anyPtr=dataStrct;
				osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
				break;
			case TOKEN:
			//chose data
			queueMsgT.anyPtr=queueMsgT.anyPtr;
			dataStrct=queueMsgT.anyPtr;
			//Check if update token list
			bool update =false;
			for(int i = 0;i<15;i++)
				{
					if(dataStrct->token.user[i]!=gTokenInterface.station_list[i]&&i!=gTokenInterface.myAddress-1)
					{
						update=true;
						gTokenInterface.station_list[i]=dataStrct->token.user[i];
					}
				}
				//Update the token
				if(gTokenInterface.connected)
				{
					dataStrct->token.user[gTokenInterface.myAddress-1]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
					gTokenInterface.station_list[gTokenInterface.myAddress-1]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
				}
				else
				{
					dataStrct->token.user[gTokenInterface.myAddress-1]=(1 << TIME_SAPI) | (0<<CHAT_SAPI);
					gTokenInterface.station_list[gTokenInterface.myAddress-1]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
				}
				//If update the liste
				if(update)
				{
					queueMsgS.type=TOKEN_LIST;
					osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,osWaitForever);
				}
				//ckeck if as a message to send
				if(osMessageQueueGet(queue_macInt_id,&queueMsgR,NULL,0)==osOK)
				{
					//send message to phisique
					osMessageQueuePut(queue_phyS_id,&queueMsgR,osPriorityNormal,osWaitForever);
					osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				}
				else
				{//return token
					//send to phy
					queueMsgT.type = TO_PHY;
					osMessageQueuePut(queue_phyS_id,&queueMsgT,osPriorityNormal,osWaitForever);
				}
				osMemoryPoolFree(memPool,queueMsgT.anyPtr);
				break;
			case DATABACK:
				break;
			case START :
				break;
			case STOP:
				break;
			case DATA_IND:
				data=queueMsgR.anyPtr;
				int lenght;
				//data=*(queueMsgR.anyPtr);
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
