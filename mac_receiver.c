#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

void MacReceiver(void *argument)
{
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	struct queueMsg_t queueMsgI;					// queue message Indic
	dataStruct *dataR;										//data revie
	dataStruct *dataS;										//data send
	dataFram	*dataI;											//dataI Indic
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
					//send token of sender
					queueMsgS.type = TOKEN;
					queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
					 *((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
					osMessageQueuePut(queue_macS_id,&queueMsgS,osPriorityNormal,osWaitForever);
					osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				}
				else
				{
					//check if Fiel Source is my
					if(dataR->fram.contolFram.source>>3==gTokenInterface.myAddress)
					{
						//check auto message
						if(dataR->fram.contolFram.destination>>3==gTokenInterface.myAddress||dataR->fram.contolFram.destination>>3==BROADCAST_ADDRESS)
						{
							//check the checksum
							uint8_t checksum=0;
							for(int i = 0;i<dataR->fram.lenght+3;i++)
							{
								checksum+=((uint8_t*)(dataR))[i];
							}
							if((checksum&0x3F)==dataR->fram.dataAndStatus.data[dataR->fram.lenght]>>2)
							{
								//READ and ACK are true
								dataR->fram.dataAndStatus.data[dataR->fram.lenght]=dataR->fram.dataAndStatus.data[dataR->fram.lenght]|0x03;
								//send indication 
								queueMsgI.type=DATA_IND;
								queueMsgI.anyPtr=osMemoryPoolAlloc(memPool,0);
								dataI=queueMsgI.anyPtr;
								//soucre addr
								queueMsgI.addr=dataR->fram.contolFram.source>>3;
								//source SAPI
								queueMsgI.sapi=dataR->fram.contolFram.source&0x07;
								//data
								*((dataFram*)queueMsgI.anyPtr)=dataR->fram.dataAndStatus;
								dataI->data[dataR->fram.lenght]='\0';
								//check if a time or a chat
								switch(queueMsgI.sapi)
								{
									case CHAT_SAPI :
										osMessageQueuePut(queue_chatR_id,&queueMsgI,osPriorityNormal,osWaitForever);
										break;
									case TIME_SAPI:
										osMessageQueuePut(queue_timeR_id,&queueMsgI,osPriorityNormal,osWaitForever);
										break;
									default:
										break;
								}
							}
							else
							{
								//READ is true and ACK is false
								dataR=queueMsgR.anyPtr;
								dataR->fram.dataAndStatus.data[dataR->fram.lenght]=dataR->fram.dataAndStatus.data[dataR->fram.lenght]|2;
							}
						}
						
						//send your message of mac_sender
						queueMsgS.type = DATABACK;
						queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
						*((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
						osMessageQueuePut(queue_macS_id,&queueMsgS,osPriorityNormal,osWaitForever);
						osMemoryPoolFree(memPool,queueMsgR.anyPtr);
					}
					else
					{
						//check if a message for my or all
						if(dataR->fram.contolFram.destination>>3==gTokenInterface.myAddress||dataR->fram.contolFram.destination>>3==BROADCAST_ADDRESS)
						{
							//controle the checksum
							uint8_t checksum=0;
							for(int i = 0;i<dataR->fram.lenght+3;i++)
							{
								checksum+=((uint8_t*)(dataR))[i];
							}
							if((checksum&0x3F)==dataR->fram.dataAndStatus.data[dataR->fram.lenght]>>2)
							{
								//send your message of phy_sender
								queueMsgS.type = TO_PHY;
								queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
								dataS=queueMsgS.anyPtr;
								*((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
								//READ and ACK are true
								dataS->fram.dataAndStatus.data[dataS->fram.lenght]=dataS->fram.dataAndStatus.data[dataS->fram.lenght]|3;
								osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
								
								//send indication 
								queueMsgI.type=DATA_IND;
								queueMsgI.anyPtr=osMemoryPoolAlloc(memPool,0);
								dataI=queueMsgI.anyPtr;
								//soucre addr
								queueMsgI.addr=dataS->fram.contolFram.source>>3;
								//source SAPI
								queueMsgI.sapi=dataS->fram.contolFram.source&0x07;
								//data
								*((dataFram*)queueMsgI.anyPtr)=dataS->fram.dataAndStatus;
								dataI->data[dataS->fram.lenght]='\0';
								//check if a time or a chat
								switch(queueMsgI.sapi)
								{
									case CHAT_SAPI :
										osMessageQueuePut(queue_chatR_id,&queueMsgI,osPriorityNormal,osWaitForever);
										break;
									case TIME_SAPI:
										osMessageQueuePut(queue_timeR_id,&queueMsgI,osPriorityNormal,osWaitForever);
										break;
									default:
										break;
								}
							}
							else
							{
								//send your message of phy_sender
								queueMsgS.type = TO_PHY;
								queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
								dataS=queueMsgS.anyPtr;
								*((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
								//READ is true and ACK is false
								dataS->fram.dataAndStatus.data[dataS->fram.lenght]=dataS->fram.dataAndStatus.data[dataS->fram.lenght]|2;
								osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
							}
							osMemoryPoolFree(memPool,queueMsgR.anyPtr);
						}
						else
						{
							//check the seeAll not use
							if(true/*!gTokenInterface.seeAll*/)
							{
								//send the message of phy_send
								queueMsgS.type = TO_PHY;
								queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
								*((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgR.anyPtr);
								osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,osWaitForever);
								osMemoryPoolFree(memPool,queueMsgR.anyPtr);
							}
						}
					}
				}
				break;
			default :
				break;
		}
	}
}
