#ifndef stepconfirm_h
#define stepconfirm_h

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


void step_confirm_show(GtkWindow *parent,char * title, char * message);

#endif
