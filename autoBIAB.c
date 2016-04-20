#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>

GtkBuilder *gtkBuilder;
GtkLabel *lblKettleTemp;
float f_kettleTemp = 212;

void updateTempLabel(void)
{
	char sTemp [15];
	sprintf(sTemp,"%3.2f",f_kettleTemp);
	
	gtk_label_set_text(lblKettleTemp,sTemp);
    
}

gboolean state_timer_cb(gpointer data)
{
	updateTempLabel();
	return TRUE; 	
}



int main(int argc, char *argv[])
{
	
	GtkWidget *window;
	gtk_init(&argc,&argv);
	
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
