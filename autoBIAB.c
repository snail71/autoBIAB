#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include "sysfs_helper.h"

#define W1_DEVICE_PATH "/sys/bus/w1/devices"

GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
float f_kettleTemp = 212;

char * sSensor = NULL;

void updateTempLabel(void)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleTemp);
	
	gtk_label_set_text(lblKettleTemp,sTemp);
    
}

gboolean read_sensor(void)
{
	int bytes;
    char devicePath[255];
    char buffer[255];
    char sdata[6] = {0,0,0,0,0,0};
    char *tok;
    memset(devicePath,0,sizeof(devicePath));
    sprintf(devicePath,"%s/%s/w1_slave",W1_DEVICE_PATH,sSensor);
    //printf("Reading sensor - path=[%s]\n",devicePath);
    bytes = read_sysfs_file(devicePath,buffer,sizeof(buffer));
    if(bytes == -1)
    {
		printf("Error reading sensor data\n");
        return FALSE;
	}
    //printf("Sensor buffer = %s\n",buffer);
    tok = strtok(buffer,"\n");
    if(strstr(tok,"YES") == NULL)
    {
		printf("Sensor data not ready\n");
		return FALSE;
	}
    if(tok != NULL)
    {
		tok = strtok(NULL,"\n");
		strncpy(sdata,strstr(tok,"t=")+2,5);
		f_kettleTemp = (((atof(sdata) * .001)* 9) / 5) + 32;
	}	
	return TRUE;
}

gboolean state_timer_cb(gpointer data)
{
	if(sSensor != NULL)
	{
		read_sensor();
	    updateTempLabel();
	    
	}
	return TRUE; 	
}



gboolean find_sensor(void)
{
	char **dirs;
	size_t count;
	int i;
	count = list_dir(W1_DEVICE_PATH,&dirs);
	for(i = 0; i < count; i++)
	{
		if(strcmp(dirs[i],"w1_bus_master1") != 0)
		{
		    sSensor = (char*)calloc(sizeof(dirs[i]),sizeof(char));
		    strcpy(sSensor,dirs[i]);
		    printf("Temp sensor located [%s]\n",sSensor);	
		    return TRUE;
		}
		//printf("Device file [%s]\n",dirs[i]);
	}
	printf("Temp sensor NOT located\n");
	return FALSE;
	
	
}

int main(int argc, char *argv[])
{
	
	GtkWidget *window;
	gtk_init(&argc,&argv);
	
	find_sensor();
	
	gtkBuilder = gtk_builder_new();
	gtk_builder_add_from_file(gtkBuilder,"autoBIAB.glade",NULL);
	window = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"winMain"));
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_timeout_add(3000,state_timer_cb,NULL);
	gtk_widget_show(window);
	gtk_main();
	return 0;
    
}
