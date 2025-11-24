#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>
#include <stdlib.h>  // for atoi
#include <string.h>
#include <ctype.h>   // for isdigit and isalpha
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "coach.h"

// Global variable for gender (1 = Male, 2 = Female)
int gender = 1;

// Check if number
int is_number(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        if (!isdigit(str[i]))
            return 0;
    return 1;
}

// Check if letters
int is_letters(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalpha(str[i]))
            return 0;
    return 1;
}

// -----------------------------
//        ADD COACH
// -----------------------------
void on_btadd_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry_id, *entry_name, *entry_first_name, *entry_phone_number;
    GtkWidget *spin_day, *spin_month, *spin_year;
    GtkWidget *combo_center;

    Coach c;

    entry_id           = lookup_widget(window, "entryid");
    entry_name         = lookup_widget(window, "entryname2");
    entry_first_name   = lookup_widget(window, "entryfirstname");
    entry_phone_number = lookup_widget(window, "entryphonenumber");
    spin_day           = lookup_widget(window, "spinbuttonday");
    spin_month         = lookup_widget(window, "spinbuttonmonth");
    spin_year          = lookup_widget(window, "spinbuttonyear");
    combo_center       = lookup_widget(window, "entrycentre2");

    const gchar *id_text    = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const gchar *name_text  = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const gchar *fname_text = gtk_entry_get_text(GTK_ENTRY(entry_first_name));
    const gchar *phone_text = gtk_entry_get_text(GTK_ENTRY(entry_phone_number));

    // ---- CONTROLE DE SAISIE ----
    if (!is_number(id_text))
    {
        g_print("Error: ID must contain only numbers.\n");
        return;
    }
    if (!is_letters(name_text))
    {
        g_print("Error: Last name must contain only letters.\n");
        return;
    }
    if (!is_letters(fname_text))
    {
        g_print("Error: First name must contain only letters.\n");
        return;
    }
    if (!is_number(phone_text))
    {
        g_print("Error: Phone number must contain only numbers.\n");
        return;
    }

    // Copy data
    c.id = atoi(id_text);
    strcpy(c.lastName, name_text);
    strcpy(c.firstName, fname_text);
    strcpy(c.phoneNumber, phone_text);

    c.dateOfBirth.day   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));

    if (gender == 1)
        strcpy(c.gender, "Male");
    else
        strcpy(c.gender, "Female");

    GtkEntry *entry_in_combo =
        GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *selected_center =
        gtk_entry_get_text(entry_in_combo);

    strcpy(c.center, selected_center);

    if (addCoach("coach.txt", c))
        g_print("Coach added successfully.\n");
    else
        g_print("Error adding coach.\n");
}


// -----------------------------
//       MODIFY COACH
// -----------------------------
void on_btmodify_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry_id, *entry_name, *entry_first_name, *entry_phone_number;
    GtkWidget *spin_day, *spin_month, *spin_year;
    GtkWidget *combo_center;

    Coach c;

    entry_id           = lookup_widget(window, "entryid");
    entry_name         = lookup_widget(window, "entryname2");
    entry_first_name   = lookup_widget(window, "entryfirstname");
    entry_phone_number = lookup_widget(window, "entryphonenumber");
    spin_day           = lookup_widget(window, "spinbuttonday");
    spin_month         = lookup_widget(window, "spinbuttonmonth");
    spin_year          = lookup_widget(window, "spinbuttonyear");
    combo_center       = lookup_widget(window, "entrycentre2");

    const gchar *id_text    = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const gchar *name_text  = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const gchar *fname_text = gtk_entry_get_text(GTK_ENTRY(entry_first_name));
    const gchar *phone_text = gtk_entry_get_text(GTK_ENTRY(entry_phone_number));

    // ---- CONTROLE DE SAISIE IDENTIQUE ----
    if (!is_number(id_text))
    {
        g_print("Error: ID must contain only numbers.\n");
        return;
    }
    if (!is_letters(name_text))
    {
        g_print("Error: Last name must contain only letters.\n");
        return;
    }
    if (!is_letters(fname_text))
    {
        g_print("Error: First name must contain only letters.\n");
        return;
    }
    if (!is_number(phone_text))
    {
        g_print("Error: Phone number must contain only numbers.\n");
        return;
    }

    // Copy data into coach struct
    c.id = atoi(id_text);
    strcpy(c.lastName, name_text);
    strcpy(c.firstName, fname_text);
    strcpy(c.phoneNumber, phone_text);

    c.dateOfBirth.day   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));

    if (gender == 1)
        strcpy(c.gender, "Male");
    else
        strcpy(c.gender, "Female");

    GtkEntry *entry_in_combo =
        GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *selected_center =
        gtk_entry_get_text(entry_in_combo);

    strcpy(c.center, selected_center);

    // ---- CORRECTION ICI ----
    if (modifyCoach("coach.txt", c.id, c))
        g_print("Coach modified successfully.\n");
    else
        g_print("Error modifying coach.\n");
}


// -----------------------------
//       RADIO BUTTONS
// -----------------------------
void on_radiobuttonman_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    if (gtk_toggle_button_get_active(togglebutton))
        gender = 1;
}

void on_radiobuttonwomen_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    if (gtk_toggle_button_get_active(togglebutton))
        gender = 2;
}

