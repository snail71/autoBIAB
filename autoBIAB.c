#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include "rs232.h"
//#include "sysfs_helper.h"
//#include "tempController.h"

#define ARD_PORT 3/*ttyS3 on J12 pins 4(RXD) and 6(TXD)*/

GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
GtkLabel *lblHeatLevel;
GtkLabel *lblButtonStartStop;
float f_kettleTemp = 212;
float f_kettleHeatLevel = 0;
float f_setpoint = 152;
bool isRunningTempControl = false;
bool ardPortOpen = false;
gint tmrArdIFC = 0;



//struct _pid tempPID;

void log_data()
{
	FILE *fp;
	//char buffer[255];
	char dt[255];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(dt,"%d-%d-%d %d:%d:%d",tm.tm_year + 1900,tm.tm_mon + 1, tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	
	//int bytes = sprintf(buffer,"%s,%3.2f,%3.2f,%3.2f\n",dt,f_setpoint,f_kettleTemp,f_kettleHeatLevel);
	//printf("%s",buffer);
	//int fd = open("session.log",O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
	//write(fd,buffer,bytes);
	//close(fd);
	fp = fopen("session.log","a");
	fprintf(fp,"%s,%3.2f,%3.2f,%3.2f\n",dt,f_setpoint,f_kettleTemp,f_kettleHeatLevel);
	fclose(fp);
}

void updateTempLabel(void)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleTemp);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblKettleTemp,sTemp);
    
}

void updateHeatLevelLabel(void)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleHeatLevel);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblHeatLevel,sTemp);
    
}



//void heatControlCB(float temp, bool heating)
//{
//	if(!isInitialized)
//	    return;
//	printf("GUI Callback: %f\n",temp);
//   f_kettleTemp = temp;
//    //gtk_label_set_text(lblKettleTemp,"callback");
//    updateTempLabel();	
//    
//}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void parse_ard_buffer(unsigned char * data)
{
	char ** lines;
	char ** args;
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
					f_kettleTemp = atof(*(args + 1));
					updateTempLabel();
				}
				else if(strcmp(*(args + 0),"SP") == 0)
				{
				
				}
				else if(strcmp(*(args + 0),"HL") == 0)
				{
					f_kettleHeatLevel = atof(*(args + 1));
					updateHeatLevelLabel();
				}
			
				free(*(args + 1));
				free(args);
			}
			free(*(lines + i));
		}
		
		free(lines);
	}
}

bool polltemp = true;

gboolean ardIFC_cb(gpointer data)
{
	unsigned char buf[4096];
	
	int inbytes = RS232_PollComport(ARD_PORT,buf,4095);
	if(inbytes > 0)
	{
		printf("Data rec\n");
		buf[inbytes] = 0; // null terminate string
		parse_ard_buffer(buf);
	}
	
	if(polltemp == true)
	{
		// Poll temperature
		printf("Polling temp\n");
		RS232_SendBuf(ARD_PORT,(unsigned char *)"READTEMP:1\n",11);
		polltemp = false;
	}
	else
	{	
		// Poll heat level
		printf("Polling hl\n");
		RS232_SendBuf(ARD_PORT,(unsigned char *)"READHL:1\n",9);
		polltemp = true;
	}
	
	log_data();
	
	return TRUE;
}

void ardIFC_open()
{
	if(ardPortOpen)
		return;
	char mode[] = {'8','N','1',0};
	if(RS232_OpenComport(ARD_PORT,9600,mode))
	{
		printf("Failed to open ard port\n");
		ardPortOpen = false;
		return ;
	}
	ardPortOpen = true;
	tmrArdIFC = g_timeout_add(500,ardIFC_cb,NULL);
	RS232_SendBuf(ARD_PORT,(unsigned char *)"HEATENABLE:1\n",13);
	return ;
}

void ardIFC_close()
{
	if(ardPortOpen)
	{
		RS232_SendBuf(ARD_PORT,(unsigned char *)"HEATENABLE:0\n",13);
		RS232_CloseComport(ARD_PORT);
		ardPortOpen = false;
		g_source_remove(tmrArdIFC);
	}
}

void start_temp_control()
{
	isRunningTempControl = true;
	gtk_label_set_text(lblButtonStartStop,"Stop");
	ardIFC_open();
}

void stop_temp_control()
{
	isRunningTempControl = false;
	gtk_label_set_text(lblButtonStartStop,"Start");
	gtk_label_set_text(lblKettleTemp,"---");
	gtk_label_set_text(lblHeatLevel,"---");
	ardIFC_close();
}

void btnStartClicked(GtkWidget *widget, gpointer data)
{
	
	if(isRunningTempControl)
	{
		stop_temp_control();
		
	}
	else
	{
		start_temp_control();
		
	}
    
}

int main(int argc, char *argv[])
{
	
	//pid_init(&tempPID,71,212);
	//pid_tune(&tempPID,200,40,1,1);
	
	GtkWidget *window;
	GtkWidget *button;
	GtkWindow *mainWin;
	
	gtk_init(&argc,&argv);
	
	
	
	gtkBuilder = gtk_builder_new();
	gtk_builder_add_from_file(gtkBuilder,"autoBIAB.glade",NULL);
	window = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"winMain"));
	mainWin = GTK_WINDOW(gtk_builder_get_object(gtkBuilder,"winMain"));
	if(!window)
	{
		printf("Error creating window from GLADE\n");
		return -1;
	}
	button = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnStart"));
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	lblHeatLevel = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblHeatLevel"));
	lblButtonStartStop = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblButtonStartStop"));
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button,"clicked",G_CALLBACK(btnStartClicked),NULL);
	gtk_window_move(mainWin,0,0);
	gtk_widget_show_all(window);
	
	//tc_Init(212,16.16, 0.14, 480.10);
	//tc_RegisterCB(heatControlCB);
	
	gtk_main();
	//tc_Stop();
	//tc_UnregisterCB();
	if(isRunningTempControl)
		stop_temp_control();
	return 0;
    
}
