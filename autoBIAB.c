#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include "rs232.h"
//#include "sysfs_helper.h"
//#include "tempController.h"



GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
float f_kettleTemp = 212;
bool isInitialized = false;

//struct _pid tempPID;



void updateTempLabel(void)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleTemp);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblKettleTemp,sTemp);
    
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



void btnStartClicked(GtkWidget *widget, gpointer data)
{
	gtk_label_set_text(lblKettleTemp,"button pressed");
	
    //tc_Start();
	isInitialized = true;	
}

void ardIFC_open()
{
	char mode[] = {'8','N','1',0};
	
}

int main(int argc, char *argv[])
{
	
	//pid_init(&tempPID,71,212);
	//pid_tune(&tempPID,200,40,1,1);
	
	GtkWidget *window;
	GtkWidget *button;
	gtk_init(&argc,&argv);
	
	
	
	gtkBuilder = gtk_builder_new();
	gtk_builder_add_from_file(gtkBuilder,"autoBIAB.glade",NULL);
	window = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"winMain"));
	button = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnStart"));
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button,"clicked",G_CALLBACK(btnStartClicked),NULL);
	
	gtk_widget_show_all(window);
	
	//tc_Init(212,16.16, 0.14, 480.10);
	//tc_RegisterCB(heatControlCB);
	
	gtk_main();
	//tc_Stop();
	//tc_UnregisterCB();
	return 0;
    
}
