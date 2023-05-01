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
	queue_macInt_id = osMessageQueueNew(4,sizeof(struct queueMsg_t),&queue_macInt_attr);
	//Declacration
	struct queueMsg_t queueMsgR;					// queue message recive
	struct queueMsg_t queueMsgS;					// queue message send
	struct queueMsg_t queueMsgT;					// queue message token
	struct queueMsg_t queueMsgM;					// queue message Message
	dataStruct *dataStrct;								//data
	const uint8_t NbErrorACKMAX=10;				//nombre of resend ack error MAX
	uint8_t NbErrorACK =0;								//nombre of resend ack error 
	bool first = true;
	//initalisation
	queueMsgS.type=TOKEN_LIST;
	osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,0);
	for (;;)														  // loop until doomsday
	{
		//wait a Message
		osMessageQueueGet(queue_macS_id,&queueMsgR,NULL,osWaitForever);
		switch(queueMsgR.type)
		{
			case NEW_TOKEN:
				queueMsgS.type = TO_PHY;
				dataStrct = osMemoryPoolAlloc(memPool,0);
				//check if memorry full
				if(dataStrct == NULL)
				{
					while(true){}
				}
				//modift du token
				//
				dataStrct->token.addressToken=0xFF;
				
				for(int i = 0;i<15;i++)
				{
					dataStrct->token.user[i]=0x00;
				}
				//dit si je suis connect ou pas
					dataStrct->token.user[gTokenInterface.myAddress]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
				//
				queueMsgS.anyPtr=dataStrct;
				if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
				{
				osMemoryPoolFree(memPool,queueMsgS.anyPtr);
				}
				break;
			case TOKEN:
			//chose data
			queueMsgT.anyPtr=queueMsgR.anyPtr;
			dataStrct=queueMsgT.anyPtr;
			//Check if update token list
			bool update =false;
			for(int i = 0;i<15;i++)
				{
					if(dataStrct->token.user[i]!=gTokenInterface.station_list[i]&&i!=gTokenInterface.myAddress)
					{
						update=true;
						gTokenInterface.station_list[i]=dataStrct->token.user[i];
					}
				}
				//Update my token if change
				if(gTokenInterface.connected!=((dataStrct->token.user[gTokenInterface.myAddress]>>CHAT_SAPI)&0x01)||first)
				{
					first = false;
					update=true;
					if(gTokenInterface.connected)
					{
						dataStrct->token.user[gTokenInterface.myAddress]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
						gTokenInterface.station_list[gTokenInterface.myAddress]=(1 << TIME_SAPI) | (1<<CHAT_SAPI);
					}
					else
					{
						dataStrct->token.user[gTokenInterface.myAddress]=(1 << TIME_SAPI) | (0<<CHAT_SAPI);
						gTokenInterface.station_list[gTokenInterface.myAddress]=(1 << TIME_SAPI) | (0<<CHAT_SAPI);
					}
				}
				//If update the liste
				if(update)
				{
					queueMsgS.type=TOKEN_LIST;
					queueMsgS.anyPtr=NULL;
					if(osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,0)!=osOK)
					{
						osMemoryPoolFree(memPool,queueMsgS.anyPtr);
					}
				}
				
				//ckeck if as a message to send
				if((osMessageQueueGet(queue_macInt_id,&queueMsgS,NULL,0)==osOK))
				{
					//save the message interne
					queueMsgM.type = TO_PHY;
					queueMsgM.anyPtr = osMemoryPoolAlloc(memPool,0);
					if(queueMsgM.anyPtr == NULL)
					{
						while(true){}
					}
					*((dataStruct*)queueMsgM.anyPtr)=*((dataStruct*)queueMsgS.anyPtr);
					//send message to phisique
					if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
					{
						osMemoryPoolFree(memPool,queueMsgS.anyPtr);
					}
				}
				else
				{//return token
					//send to phy
					queueMsgS.type = TO_PHY;
					queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
					if(queueMsgS.anyPtr == NULL)
					{
						while(true){}
					}
					 *((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgT.anyPtr);
					if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
					{
						osMemoryPoolFree(memPool,queueMsgS.anyPtr);
					}
					
					osMemoryPoolFree(memPool,queueMsgT.anyPtr);
				}
				break;
			case DATABACK:
				//check the read bit
				dataStrct = queueMsgR.anyPtr;
				int test =(dataStrct->fram.dataAndStatus.data[dataStrct->fram.lenght]&0x02);
				if(test==2)
				{
						//check the ACK
					if(dataStrct->fram.dataAndStatus.data[dataStrct->fram.lenght]&0x01==1)
					{
						//send the token  
						queueMsgS.type = TO_PHY;
						queueMsgS.anyPtr=queueMsgT.anyPtr;
						if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
						{
							osMemoryPoolFree(memPool,queueMsgS.anyPtr);
						}
						//reset error nb
						NbErrorACK=0;
						//delet save message
						osMemoryPoolFree(memPool,queueMsgM.anyPtr);
					}
					else
					{
						if(NbErrorACK<NbErrorACKMAX)
						{
							//incr error nb
							NbErrorACK++;
							//resend the message
							queueMsgS.type = TO_PHY;
							queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
							if(queueMsgS.anyPtr == NULL)
							{
								while(true){}
							}
							*((dataStruct*)queueMsgS.anyPtr)=*((dataStruct*)queueMsgM.anyPtr);
							if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
							{
								osMemoryPoolFree(memPool,queueMsgS.anyPtr);
							}	
						}
						else
						{
							//reset error nb
							NbErrorACK=0;
							//send a indication erro at the LCD
							queueMsgS.type = MAC_ERROR;
							queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
							if(queueMsgS.anyPtr == NULL)
							{
								while(true){}
							}
							queueMsgS.addr=dataStrct->fram.contolFram.source>>3+1;
							sprintf((char*)queueMsgS.anyPtr,"Error CRC for : %d \n",(dataStrct->fram.contolFram.destination>>3)+1);
							//(char*)queueMsgS.anyPtr="Error CRC for : "+strcpy(dataStrct->fram.contolFram.destination>>3);
							if(osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,0)!=osOK)
							{
								osMemoryPoolFree(memPool,queueMsgS.anyPtr);
							}
							//send the token delet message
							queueMsgS.type = TO_PHY;
							queueMsgS.anyPtr=queueMsgT.anyPtr;
							if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
							{
								osMemoryPoolFree(memPool,queueMsgS.anyPtr);
							}
						}
					}
				}
				else
				{
					//send a indication erro at the LCD
					queueMsgS.type = MAC_ERROR;
					queueMsgS.anyPtr=osMemoryPoolAlloc(memPool,0);
					if(queueMsgS.anyPtr == NULL)
					{
						while(true){}
					}
					queueMsgS.addr=(dataStrct->fram.contolFram.source>>3)+1;
					sprintf((char*)queueMsgS.anyPtr,"The Cible %d not found\n",(dataStrct->fram.contolFram.destination>>3)+1);
					//(char*)queueMsgS.anyPtr="The Cible "+dataStrct->fram.contolFram.destination>>3+"not found";
					if(osMessageQueuePut(queue_lcd_id,&queueMsgS,osPriorityNormal,0)!=osOK)
					{
						osMemoryPoolFree(memPool,queueMsgS.anyPtr);
					}
					//send the token delet message
					queueMsgS.type = TO_PHY;
					queueMsgS.anyPtr=queueMsgT.anyPtr;
					if(osMessageQueuePut(queue_phyS_id,&queueMsgS,osPriorityNormal,0)!=osOK)
					{
						osMemoryPoolFree(memPool,queueMsgS.anyPtr);
					}
					//reset error nb
					NbErrorACK=0;
					//delet save message
					osMemoryPoolFree(memPool,queueMsgM.anyPtr);
				}
				//Delete the recive
				osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				break;
			case START :
				gTokenInterface.connected=true;
				break;
			case STOP:
				gTokenInterface.connected=false;
				break;
			case DATA_IND:
				//creat the message to save
				queueMsgS.type = TO_PHY;
				dataStrct=osMemoryPoolAlloc(memPool,0);
				if(dataStrct == NULL)
				{
					while(true){}
				}
				//source
				dataStrct->fram.contolFram.source=(gTokenInterface.myAddress)<<3|queueMsgR.sapi;
				//destination
				dataStrct->fram.contolFram.destination=queueMsgR.addr<<3|queueMsgR.sapi;
				//lenght
				dataStrct->fram.lenght = strlen(queueMsgR.anyPtr);		
				//message
				dataStrct->fram.dataAndStatus=*((dataFram*)queueMsgR.anyPtr);
				//satatus
				uint8_t checksum=0;
				for(int i = 0;i<dataStrct->fram.lenght+3;i++)
				{
					checksum+=((uint8_t*)(dataStrct))[i];
				}
				dataStrct->fram.dataAndStatus.data[dataStrct->fram.lenght]=checksum<<2;
				//ckeck brodcat
				if(queueMsgR.addr==BROADCAST_ADDRESS)
				{
					dataStrct->fram.dataAndStatus.data[dataStrct->fram.lenght]|=3;
				}
				//send into intern queue
				queueMsgS.anyPtr=dataStrct;
				if(osMessageQueuePut(queue_macInt_id,&queueMsgS,osPriorityNormal,0)!=osOK)
				{
					osMemoryPoolFree(memPool,queueMsgS.anyPtr);
				}
				//delete la memoire du message recu
				osMemoryPoolFree(memPool,queueMsgR.anyPtr);
				break;
			default :
				break;
		}
	}
}
