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

void step_confirm_show(GtkWindow *parent, char * title, char * message)
{
	GtkWidget *msgDialog;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;	
	
	msgDialog = gtk_message_dialog_new(parent,
										flags,
										GTK_MESSAGE_INFO,
										GTK_BUTTONS_OK,
										"%s: %s",
										title,
										message);
	gtk_dialog_run(GTK_DIALOG(msgDialog));
	gtk_widget_destroy(msgDialog);
}


