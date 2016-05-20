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
#include "utils.h"
#include "recipe.h"


#define ARD_PORT 3/*ttyS3 on J12 pins 4(RXD) and 6(TXD)*/

GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
GtkLabel *lblHeatLevel;
GtkLabel *lblButtonStartStop;
GtkLabel *lblRecipeName;
float f_kettleTemp = 212;
float f_kettleHeatLevel = 0;
float f_setpoint = 152;
bool isRunningTempControl = false;
bool ardPortOpen = false;
gint tmrArdIFC = 0;
bool polltemp = true;
struct recipe brewRecipe;

void updateRecipeLabel(bool recipeLoaded)
{	
	if(recipeLoaded == true)
		gtk_label_set_text(lblRecipeName,brewRecipe.name);    
	else
		gtk_label_set_text(lblRecipeName,"Recipe does not exist!"); 
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
	
	log_data(f_setpoint ,f_kettleTemp,f_kettleHeatLevel);
	
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
	bool recipeLoaded = false;
		
	recipeLoaded = load_recipe_file("recipe.xml",&brewRecipe);
	
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
	gtk_widget_set_sensitive(button,recipeLoaded);
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	lblHeatLevel = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblHeatLevel"));
	lblButtonStartStop = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblButtonStartStop"));
	lblRecipeName = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblRecipeName"));
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button,"clicked",G_CALLBACK(btnStartClicked),NULL);
	gtk_window_move(mainWin,0,0);
	gtk_widget_show_all(window);
	
	updateRecipeLabel(recipeLoaded);	
	
	gtk_main();
	
	if(isRunningTempControl)
		stop_temp_control();
	return 0;
    
}
