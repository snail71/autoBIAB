#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include "controller.h"
#include "equipment.h"
#include "rs232.h"
#include "utils.h"

//#define ARD_PORT 3/*ttyS3 on J12 pins 4(RXD) and 6(TXD)*/

bool ardPortOpen = false;
gint tmrArdIFC = 0;
struct equipment *equipSettings;
struct recipe 	*recipeSettings;
struct brew_callbacks_t *callbacks;
bool pollTemp = true;
float f_kettle_temp = 0;

gboolean ardIFC_cb(gpointer data);

void parse_ard_buffer(unsigned char * data)
{
	char ** lines;
	char ** args;
	int pulseCount  = 0;
	
	lines = str_split((char*)data,'\n');
	
	if(lines)
	{
		int i;
		for(i=0; *(lines+i);i++)
		{
			printf("%s\n",*(lines + i));
			args = str_split(*(lines+i),':');
			if(args)
			{
				if(strcmp(*(args + 0),"TEMP") == 0)
				{					
					//f_kettle_temp = atof(*(args + 1));
					f_kettle_temp =  175;
					callbacks->tempcb(f_kettle_temp);
				}
				else if(strcmp(*(args + 0),"SP") == 0)
				{
					callbacks->heatlvlcb(atof(*(args + 1)));
				}
				else if(strcmp(*(args + 0),"HL") == 0)
				{					
					callbacks->heatlvlcb(atof(*(args + 1)));					
				}
				else if(strcmp(*(args + 0),"PC") == 0)
				{
					//pulseCount = atoi(*(args +1));
					pulseCount = 1040;
					recipeSettings->steps[CTL_FILL].volumeCompleted = pulseCount > 0 ? (float)pulseCount / (float)equipSettings->flowTicksPerGallon : 0;
					recipeSettings->steps[CTL_FILL].countsCompleted = pulseCount;
					callbacks->fillcb(recipeSettings->steps[CTL_FILL].volumeTotal,recipeSettings->steps[CTL_FILL].volumeCompleted );
				}
				free(*(args + 1));
				free(args);
			}
			free(*(lines + i));
		}
		
		free(lines);
	}
}

unsigned char buf[4096];
int inbuffercount = 0;
bool awaitingResponse = false;

void delayMS(int ms)
{
	struct timespec tv = {0,ms * 1000};
	nanosleep(&tv,NULL);
}

int step_time_remaining(struct brewStep *step)
{
	
	time_t endtime = (time_t)(step->startTime + ((int)step->duration * 60));
	
	return endtime - time(NULL);
	
	
	
}

void step_message(char * title, char * msg)
{
	g_source_remove(tmrArdIFC);
	callbacks->stepmsgcb(title,msg);
	tmrArdIFC = g_timeout_add(250,ardIFC_cb,NULL);
}

gboolean ardIFC_cb(gpointer data)
{
	char outbuf[255];
	int len ;
	time_t timetoend;
	time_t now;
	int timeremaining = 0;
	
	int inbytes = RS232_PollComport(equipSettings->serialPort,&buf[inbuffercount],4095);
	if(inbytes > 0)
	{
		printf("Data rec\n");
		//buf[inbytes] = 0; // null terminate string
		inbuffercount += inbytes;
		if(buf[inbuffercount-1] == '\n')
		{
			buf[inbuffercount] = 0;
			parse_ard_buffer(buf);
			inbuffercount = 0;
			awaitingResponse = false;
		}
	}
	
	if(pollTemp == true && awaitingResponse == false)
	{
		// Poll temperature
		printf("Polling temp\n");
		RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READTEMP:1\n",11);
		pollTemp = false;
		awaitingResponse = true;
	}
	else if(awaitingResponse == false)// do control state stuff
	{	
		
		if(brew_state == BREW_RUNNING)
		{
			switch(g_MachineState)
			{
				case CTL_IDLE :					
					step_message("Ready to fill?","Press OK");
					len = sprintf(outbuf,"FILL:%d\n",(int)(recipeSettings->steps[CTL_FILL].countsNeeded));
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
					recipeSettings->steps[CTL_FILL].startTime	= time(NULL);
					
					g_MachineState ++;
					break;
				case CTL_FILL:
					
					if(recipeSettings->steps[CTL_FILL].countsCompleted >= recipeSettings->steps[CTL_FILL].countsNeeded)
					{						
						recipeSettings->steps[CTL_FILL].atSetpoint = true;
						recipeSettings->steps[CTL_FILL].endTime	= time(NULL);
						
						len = sprintf(outbuf,"SETTEMP:%3.2f\n",recipeSettings->steps[CTL_STRIKE].setpoint);
						RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
						delayMS(25);
						len = sprintf(outbuf,"HEATENABLE:%d\n",1);
						RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
						delayMS(25);
						recipeSettings->steps[CTL_STRIKE].startTime	= time(NULL);
						callbacks->heatcb(recipeSettings->steps[CTL_STRIKE].description,0,0,recipeSettings->steps[CTL_STRIKE].setpoint);
						g_MachineState ++;
						
					}
					else
					{
						printf("Polling fill\n");
						RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READFILL:1\n",11);
						awaitingResponse = true;
					}
					break;
				case CTL_STRIKE:
					
					if(f_kettle_temp >= recipeSettings->steps[CTL_STRIKE].setpoint)
					{
						recipeSettings->steps[CTL_STRIKE].endTime	= time(NULL);
						g_MachineState ++;
					}
					else
					{
						printf("Polling hl\n");
						RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READHL:1\n",9);
						awaitingResponse = true;
					}
					break;
				case CTL_GRAININ:
					recipeSettings->steps[CTL_GRAININ].startTime	= time(NULL);
					step_message("Drop in grain bucket","Press OK");
					recipeSettings->steps[CTL_GRAININ].endTime	= time(NULL);
					
					len = sprintf(outbuf,"SETTEMP:%3.2f\n",recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint);
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
					recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].atSetpoint = false;
					
					g_MachineState ++;					
					break;
				case CTL_MASH:
					
					if(recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].atSetpoint == false)
					{
						callbacks->heatcb(recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].description,
						recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].duration,
						0,
						recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint);
						
						if(f_kettle_temp >= recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint)
						{
							recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].atSetpoint = true;
							recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].startTime = time(NULL);
						}
					}
					else
					{
						timeremaining = step_time_remaining(&recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep]);
						//sprintf(outbuf,"Remaining: %d",timeremaining);
						//step_message("Step time",outbuf);
						timetoend = recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].startTime + 
								(recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].duration * 60);
							now = time(NULL);
						
						callbacks->heatcb(recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].description,
						recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].duration,
						(int)timeremaining/*(timetoend - now)*/,
						recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint);
						
						if(/*(timetoend-now)*/timeremaining <= 0)						
						{
							recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].endTime = time(NULL);
							recipeSettings->mashCurrentStep++;
							if(recipeSettings->mashCurrentStep >= recipeSettings->mashStepCount)
							{
								g_MachineState = CTL_GRAINOUT;
								break;	
							}
							else
							{
								//callbacks->heatcb(recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].description,0,0,recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint);
								len = sprintf(outbuf,"SETTEMP:%3.2f\n",recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].setpoint);
								RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
								recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].atSetpoint = false;
								delayMS(25);
							}
						}	
					}
					
					printf("Polling hl\n");
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READHL:1\n",9);
					//RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READSP:1\n",9);
					awaitingResponse = true;
					break;
				case CTL_GRAINOUT:
					recipeSettings->steps[CTL_GRAINOUT].startTime	= time(NULL);
					step_message("Remove grain bucket","Press OK");
					recipeSettings->steps[CTL_GRAINOUT].endTime	= time(NULL);
					
					len = sprintf(outbuf,"SETTEMP:%3.2f\n",recipeSettings->steps[CTL_BOIL].setpoint);
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
					recipeSettings->steps[CTL_BOIL].atSetpoint = false;
					callbacks->heatcb(recipeSettings->steps[CTL_BOIL].description,0,0,recipeSettings->steps[CTL_BOIL].setpoint);
					g_MachineState ++;
					break;
				case CTL_BOIL:
					if(recipeSettings->steps[CTL_BOIL].atSetpoint == false)
					{
						callbacks->heatcb(recipeSettings->steps[CTL_BOIL].description,
						(int)recipeSettings->steps[CTL_BOIL].duration,
						0,
						recipeSettings->steps[CTL_BOIL].setpoint);
						if(f_kettle_temp >= recipeSettings->steps[CTL_BOIL].setpoint)
						{
							recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].atSetpoint = true;
							recipeSettings->steps[CTL_MASH + recipeSettings->mashCurrentStep].startTime = time(NULL);
						}
					}
					else
					{
						//timetoend = recipeSettings->steps[CTL_BOIL].startTime + 
						//	(recipeSettings->steps[CTL_BOIL].duration * 60);
						//now = time(NULL);
						timeremaining = step_time_remaining(&recipeSettings->steps[CTL_BOIL]);
						callbacks->heatcb(recipeSettings->steps[CTL_BOIL].description,
						(int)recipeSettings->steps[CTL_BOIL].duration,
						/*(int)((timetoend - now) / 60) * 100*/timeremaining,
						recipeSettings->steps[CTL_BOIL].setpoint);
						
						if(timeremaining <= 0)
						{
							recipeSettings->steps[CTL_BOIL].endTime = time(NULL);
							
							g_MachineState ++;
							break;	
							
							
						}	
					}
					
					printf("Polling hl\n");
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READHL:1\n",9);
					awaitingResponse = true;
					break;
				case CTL_END:
					break;
					
			}
		}
		
		pollTemp = true;
		
	}
	
	//log_data(f_setpoint ,f_kettleTemp,f_kettleHeatLevel);
	
	return TRUE;
}

void ardIFC_open()
{
	if(ardPortOpen)
		return;
	char mode[] = {'8','N','1',0};
	if(RS232_OpenComport(equipSettings->serialPort,9600,mode))
	{
		printf("Failed to open ard port\n");
		ardPortOpen = false;
		return ;
	}
	ardPortOpen = true;
	awaitingResponse = false;
	pollTemp = true;
	tmrArdIFC = g_timeout_add(250,ardIFC_cb,NULL);
	//RS232_SendBuf(ARD_PORT,(unsigned char *)"HEATENABLE:1\n",13);
	return ;
}

void ardIFC_close()
{
	if(ardPortOpen)
	{
		//RS232_SendBuf(ARD_PORT,(unsigned char *)"HEATENABLE:0\n",13);
		RS232_CloseComport(equipSettings->serialPort);
		ardPortOpen = false;
		g_source_remove(tmrArdIFC);
	}
}

void pause_brew()
{
	switch(brew_state)
	{
		case BREW_STOPPED:
		case BREW_PAUSED:
			return;					
		case BREW_RUNNING:			
			brew_state = BREW_PAUSED;	
			callbacks->statecb(brew_state);
			break;
	}	
}

void stop_brew()
{
	switch(brew_state)
	{
		case BREW_STOPPED:
			return;
		case BREW_PAUSED:			
		case BREW_RUNNING:
			ardIFC_close();
			brew_state = BREW_STOPPED;	
			g_MachineState = CTL_IDLE;
			callbacks->statecb(brew_state);
			callbacks->idlecb();
			break;
	}	
	
}

void start_brew(struct brew_callbacks_t *brewCB, struct recipe *rec, struct equipment *equip)
{
	switch(brew_state)
	{
		case BREW_STOPPED:
			equipSettings = equip;
			recipeSettings = rec;
			callbacks = brewCB;
			g_MachineState = CTL_IDLE;
			ardIFC_open();			
			break;
		case BREW_PAUSED:
			break;
		case BREW_RUNNING:
			return;
	}
	brew_state = BREW_RUNNING;	
	callbacks->statecb(brew_state);
}
