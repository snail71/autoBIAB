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
					callbacks->tempcb(atof(*(args + 1)));
					//f_kettleTemp = atof(*(args + 1));
					
				}
				else if(strcmp(*(args + 0),"SP") == 0)
				{
				
				}
				else if(strcmp(*(args + 0),"HL") == 0)
				{
					//f_kettleHeatLevel = atof(*(args + 1));
					//updateHeatLevelLabel();
				}
				else if(strcmp(*(args + 0),"PC") == 0)
				{
					pulseCount = atoi(*(args +1));
					recipeSettings->steps[CTL_FILL].volumeCompleted = pulseCount > 0 ? (float)pulseCount / (float)equipSettings->flowTicksPerGallon : 0;
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
gboolean ardIFC_cb(gpointer data)
{
	char outbuf[255];
	int len ;
	
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
					len = sprintf(outbuf,"FILL:%d\n",(int)(recipeSettings->steps[CTL_FILL].volumeTotal * (float)equipSettings->flowTicksPerGallon));
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)outbuf,len);
					recipeSettings->steps[CTL_FILL].startTime	= time(NULL);
					g_MachineState ++;
					break;
				case CTL_FILL:
					RS232_SendBuf(equipSettings->serialPort,(unsigned char *)"READFILL:1\n",11);
					if(recipeSettings->steps[CTL_FILL].volumeCompleted >= recipeSettings->steps[CTL_FILL].volumeTotal)
					{
						recipeSettings->steps[CTL_FILL].atSetpoint = true;
						recipeSettings->steps[CTL_FILL].endTime	= time(NULL);
						g_MachineState ++;
					}
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
	tmrArdIFC = g_timeout_add(50,ardIFC_cb,NULL);
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
