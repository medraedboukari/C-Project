#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include "coach.h"

// ADD COACH
void on_btadd_clicked(GtkButton *button, gpointer user_data);

// MODIFY COACH
void on_btmodify_clicked(GtkButton *button, gpointer user_data);

// RADIO BUTTONS
void on_radiobuttonman_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobuttonwomen_toggled(GtkToggleButton *togglebutton, gpointer user_data);

#endif

