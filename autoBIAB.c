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
#include "stepconfirm.h"




GtkBuilder *gtkBuilder;
GtkWindow *mainWin;
GtkLabel *lblRecipeName;
GtkLabel *lblKettleTemp;
GtkLabel *lblHeatLevel;
GtkLabel *lblStatus;
GtkLabel *lblStepName;
GtkLabel *lblVolDurFrame;
GtkLabel *lblRemainingFrame;
GtkLabel *lblStepDuration;
GtkLabel *lblStepRemaining;
GtkLabel *lblStepSetpoint;

GtkWidget *buttonStart;
GtkWidget *buttonStop;
GtkWidget *buttonPause;
GtkWidget *btnConfig;
GtkWidget *btnOff;


//GtkLabel *lblButtonStartStop;

//float f_kettleTemp = 212;
//float f_kettleHeatLevel = 0;
//float f_setpoint = 152;
//bool isBrewing = false;




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
	sprintf(sTemp,"%3.0f",f_kettleHeatLevel);
	printf("Setting temp label: %s\n",sTemp);
	gtk_label_set_text(lblHeatLevel,sTemp);    
}

void update_brew_state_cb(enum brew_state_t state)
{
	switch(state)
	{
		case BREW_PAUSED:
			gtk_widget_set_sensitive(buttonStart,true);
			gtk_widget_set_sensitive(buttonStop,true);
			gtk_widget_set_sensitive(buttonPause,false);
			gtk_widget_set_sensitive(btnConfig,false);
			gtk_widget_set_sensitive(btnOff,false);
			gtk_label_set_text(lblStatus,"Paused");
			break;
		case BREW_RUNNING:
			gtk_widget_set_sensitive(buttonStart,false);
			gtk_widget_set_sensitive(buttonStop,true);
			gtk_widget_set_sensitive(buttonPause,true);
			gtk_widget_set_sensitive(btnConfig,false);
			gtk_widget_set_sensitive(btnOff,false);
			gtk_label_set_text(lblStatus,"Running");
			break;
		case BREW_STOPPED:
			gtk_label_set_text(lblKettleTemp,"---");	
			gtk_label_set_text(lblHeatLevel,"---");
			gtk_widget_set_sensitive(buttonStart,true);
			gtk_widget_set_sensitive(buttonStop,false);
			gtk_widget_set_sensitive(buttonPause,false);
			gtk_widget_set_sensitive(btnConfig,true);
			gtk_widget_set_sensitive(btnOff,true);
			gtk_label_set_text(lblStatus,"Stopped");
			break;
	}
}

void update_heat_step_cb(char * description, int duration, int remaining, float setpoint)
{
	char buf[255];
	int secs = 0;
	int mins = 0;
	
	if(remaining > 0)
	{
		mins = (int)remaining / 60;
		secs = (int)remaining % 60;
	}
	gtk_label_set_text(lblVolDurFrame,"DURATION");	
	gtk_label_set_text(lblRemainingFrame,"REMAINING");
	gtk_label_set_text(lblStepName,description);
	sprintf(buf,"%3.2f",setpoint);
	gtk_label_set_text(lblStepSetpoint,buf);
	sprintf(buf,"%3d",duration);
	gtk_label_set_text(lblStepDuration,buf);
	sprintf(buf,"%3d:%02d",mins,secs);
	gtk_label_set_text(lblStepRemaining,buf);
}



void btnStartClicked(GtkWidget *widget, gpointer data)
{
	start_brew(&brewCallbacks,&brewRecipe,&brewEquipment);	
}

void btnStopClicked(GtkWidget *widget, gpointer data)
{
	stop_brew();
}

void btnOffClicked(GtkWidget *widget, gpointer data)
{
	system("sudo halt");
	
}

void btnPauseClicked(GtkWidget *widget, gpointer data)
{
	pause_brew();
	
}

void idle_cb()
{
	gtk_label_set_text(lblStepName,"DONE");	
	gtk_label_set_text(lblHeatLevel,"---");
	gtk_label_set_text(lblStepSetpoint,"---");
	gtk_label_set_text(lblStepDuration,"---");
	gtk_label_set_text(lblStepRemaining,"---");
}

void prompt_step_msg(char * title, char * msg)
{
	step_confirm_show(mainWin,title,msg);  
}

void update_temp_cb(float temp)
{
	updateTempLabel(temp);
}

void update_hl_cb(float temp)
{
	updateHeatLevelLabel(temp);
	
}

void fill_kettle_cb(float fillTo, float current)
{
	char buf[255];
	gtk_label_set_text(lblStepName,"FILL");
	gtk_label_set_text(lblVolDurFrame,"VOLUME");	
	gtk_label_set_text(lblRemainingFrame,"CURRENT");
	sprintf(buf,"%1.2f",fillTo);
	gtk_label_set_text(lblStepDuration,buf);
	sprintf(buf,"%1.2f",current);
	gtk_label_set_text(lblStepRemaining,buf);
}

int main(int argc, char *argv[])
{
	
	bool recipeLoaded = false;
	load_equipment_file("equipment.xml",&brewEquipment);	
	recipeLoaded = load_recipe_file("recipe.xml",&brewRecipe,&brewEquipment);
	
	brewCallbacks.fillcb = fill_kettle_cb;
	brewCallbacks.idlecb = idle_cb;
	brewCallbacks.tempcb = update_temp_cb;
	brewCallbacks.statecb = update_brew_state_cb;
	brewCallbacks.heatcb = update_heat_step_cb;
	brewCallbacks.heatlvlcb = update_hl_cb;
	brewCallbacks.stepmsgcb = prompt_step_msg;
	GtkWidget *window;
	
	
	
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
	btnOff = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnOff"));
	btnConfig = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"btnConfig"));
	gtk_widget_set_sensitive(buttonStart,recipeLoaded);
	lblKettleTemp = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblKettleTemp"));
	lblHeatLevel = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblHeatLevel"));
	lblStatus = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStatus"));
	lblStepName = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStepName"));
	lblVolDurFrame = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblVolDurFrame"));
	lblRemainingFrame = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblRemainingFrame"));
	lblStepDuration = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStepDuration"));
	lblStepRemaining = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStepRemaining"));
	lblStepSetpoint = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblStepSetpoint"));
	//lblButtonStartStop = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblButtonStartStop"));
	lblRecipeName = GTK_LABEL(gtk_builder_get_object(gtkBuilder,"lblRecipeName"));
	g_object_unref(G_OBJECT(gtkBuilder));
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(buttonStart,"clicked",G_CALLBACK(btnStartClicked),NULL);
	g_signal_connect(buttonStop,"clicked",G_CALLBACK(btnStopClicked),NULL);
	g_signal_connect(buttonPause,"clicked",G_CALLBACK(btnPauseClicked),NULL);
	g_signal_connect(btnOff,"clicked",G_CALLBACK(btnOffClicked),NULL);
	gtk_window_move(mainWin,0,0);
	gtk_widget_show_all(window);
	
	updateRecipeLabel(recipeLoaded);	
	
	gtk_main();
	
//	if(isRunningTempControl)
//		stop_temp_control();
	return 0;
    
}
