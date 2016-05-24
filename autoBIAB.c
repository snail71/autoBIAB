#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>

#include "utils.h"
#include "equipment.h"
#include "recipe.h"
#include "controller.h"




GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
GtkLabel *lblHeatLevel;
GtkLabel *lblStatus;

GtkWidget *buttonStart;
GtkWidget *buttonStop;
GtkWidget *buttonPause;

//GtkLabel *lblButtonStartStop;
GtkLabel *lblRecipeName;
//float f_kettleTemp = 212;
//float f_kettleHeatLevel = 0;
float f_setpoint = 152;
bool isBrewing = false;




struct recipe brewRecipe;
struct equipment brewEquipment;
struct brew_callbacks_t brewCallbacks;



void updateRecipeLabel(bool recipeLoaded)
{	
	if(recipeLoaded == true)
		gtk_label_set_text(lblRecipeName,brewRecipe.name);    
	else
		gtk_label_set_text(lblRecipeName,"Recipe does not exist!"); 
}

void updateTempLabel(float f_kettleTemp)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleTemp);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblKettleTemp,sTemp);
    
}

void updateHeatLevelLabel(float f_kettleHeatLevel)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleHeatLevel);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblHeatLevel,sTemp);    
}


//void start_temp_control()
//{
//	isRunningTempControl = true;
//	gtk_label_set_text(lblButtonStartStop,"Stop");
//	ardIFC_open();
//}

//void stop_temp_control()
//{
//	isRunningTempControl = false;
//	gtk_label_set_text(lblButtonStartStop,"Start");
//	gtk_label_set_text(lblKettleTemp,"---");
//	gtk_label_set_text(lblHeatLevel,"---");
//	ardIFC_close();
//}

void update_brew_state_cb(enum brew_state_t state)
{
	switch(state)
	{
		case BREW_PAUSED:
			gtk_widget_set_sensitive(buttonStart,true);
			gtk_widget_set_sensitive(buttonStop,true);
			gtk_widget_set_sensitive(buttonPause,false);
			gtk_label_set_text(lblStatus,"Paused");
			break;
		case BREW_RUNNING:
			gtk_widget_set_sensitive(buttonStart,false);
			gtk_widget_set_sensitive(buttonStop,true);
			gtk_widget_set_sensitive(buttonPause,true);
			gtk_label_set_text(lblStatus,"Running");
			break;
		case BREW_STOPPED:
			gtk_label_set_text(lblKettleTemp,"---");	
			gtk_label_set_text(lblHeatLevel,"---");
			gtk_widget_set_sensitive(buttonStart,true);
			gtk_widget_set_sensitive(buttonStop,false);
			gtk_widget_set_sensitive(buttonPause,false);
			gtk_label_set_text(lblStatus,"Stopped");
			break;
	}
}

void btnStartClicked(GtkWidget *widget, gpointer data)
{
	start_brew(&brewCallbacks,&brewRecipe,&brewEquipment);		    
}

void btnStopClicked(GtkWidget *widget, gpointer data)
{
	stop_brew();
}

void btnPauseClicked(GtkWidget *widget, gpointer data)
{
	pause_brew();
	
}
void idle_cb()
{
	
}

void update_temp_cb(float temp)
{
	updateTempLabel(temp);
}

void fill_kettle_cb(float fillTo, float current)
{
}

int main(int argc, char *argv[])
{
	GtkSettings *settings = gtk_settings_get_default();
	g_object_set(settings,"gtk-button-images",true,NULL);
	bool recipeLoaded = false;
	load_equipment_file("equipment.xml",&brewEquipment);	
	recipeLoaded = load_recipe_file("recipe.xml",&brewRecipe,&brewEquipment);
	
	brewCallbacks.fillcb = fill_kettle_cb;
	brewCallbacks.idlecb = idle_cb;
	brewCallbacks.tempcb = update_temp_cb;
	brewCallbacks.statecb = update_brew_state_cb;
	GtkWidget *window;
	
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
	buttonStart = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnStart"));
	buttonStop = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnStop"));
	buttonPause = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnPause"));
	gtk_widget_set_sensitive(buttonStart,recipeLoaded);
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	lblHeatLevel = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblHeatLevel"));
	lblStatus = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStatus"));
	//lblButtonStartStop = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblButtonStartStop"));
	lblRecipeName = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblRecipeName"));
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(buttonStart,"clicked",G_CALLBACK(btnStartClicked),NULL);
	g_signal_connect(buttonStop,"clicked",G_CALLBACK(btnStopClicked),NULL);
	g_signal_connect(buttonPause,"clicked",G_CALLBACK(btnPauseClicked),NULL);
	gtk_window_move(mainWin,0,0);
	gtk_widget_show_all(window);
	
	updateRecipeLabel(recipeLoaded);	
	
	gtk_main();
	
//	if(isRunningTempControl)
//		stop_temp_control();
	return 0;
    
}
