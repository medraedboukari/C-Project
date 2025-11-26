#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "coach.h"

// ----------------------------------------------------
//                  GLOBALS
// ----------------------------------------------------
int gender = 1;  // 1 = Male, 2 = Female

// ----------------------------------------------------
//             LABEL CLEAR FUNCTION
// ----------------------------------------------------
gboolean clear_label(gpointer data)
{
    GtkLabel *label = GTK_LABEL(data);
    gtk_label_set_text(label, "");
    return G_SOURCE_REMOVE;
}

// ----------------------------------------------------
//         INPUT VALIDATION FUNCTIONS
// ----------------------------------------------------
int is_number(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        if (!isdigit(str[i]))
            return 0;
    return 1;
}

int is_letters(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalpha(str[i]))
            return 0;
    return 1;
}

// ----------------------------------------------------
//                     ADD COACH
// ----------------------------------------------------
void on_btadd_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entry_id           = lookup_widget(window, "entryid");
    GtkWidget *entry_name         = lookup_widget(window, "entryname2");
    GtkWidget *entry_first_name   = lookup_widget(window, "entryfirstname");
    GtkWidget *entry_phone_number = lookup_widget(window, "entryphonenumber");
    GtkWidget *spin_day           = lookup_widget(window, "spinbuttonday");
    GtkWidget *spin_month         = lookup_widget(window, "spinbuttonmonth");
    GtkWidget *spin_year          = lookup_widget(window, "spinbuttonyear");
    GtkWidget *combo_center       = lookup_widget(window, "entrycentre2");
    GtkWidget *label_status       = lookup_widget(window, "label98");

    const gchar *id_text    = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const gchar *name_text  = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const gchar *fname_text = gtk_entry_get_text(GTK_ENTRY(entry_first_name));
    const gchar *phone_text = gtk_entry_get_text(GTK_ENTRY(entry_phone_number));

    // ----- VALIDATION -----
    if (!is_number(id_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  ID must be numbers!"); g_timeout_add(3000, clear_label, label_status); return; }
    if (!is_letters(name_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  Last name only letters!"); g_timeout_add(3000, clear_label, label_status); return; }
    if (!is_letters(fname_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  First name only letters!"); g_timeout_add(3000, clear_label, label_status); return; }
    if (!is_number(phone_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  Phone must be numbers!"); g_timeout_add(3000, clear_label, label_status); return; }

    Coach c;
    c.id = atoi(id_text);
    strcpy(c.lastName, name_text);
    strcpy(c.firstName, fname_text);
    strcpy(c.phoneNumber, phone_text);
    c.dateOfBirth.day   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));
    if (gender == 1) strcpy(c.gender, "Male");
    else strcpy(c.gender, "Female");

    GtkEntry *entry_combo = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    strcpy(c.center, gtk_entry_get_text(entry_combo));

    if (addCoach("coach.txt", c))
        gtk_label_set_text(GTK_LABEL(label_status), "  Add done!");
    else
        gtk_label_set_text(GTK_LABEL(label_status), "  Add error!");
    g_timeout_add(3000, clear_label, label_status);

    // Reset entries and spin buttons
    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    gtk_entry_set_text(GTK_ENTRY(entry_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_first_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_phone_number), "");
    gtk_entry_set_text(entry_combo, "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_day), 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_month), 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_year), 2000);
}

// ----------------------------------------------------
//                     MODIFY COACH
// ----------------------------------------------------
void on_btmodify_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entry_id           = lookup_widget(window, "entryid");
    GtkWidget *entry_name         = lookup_widget(window, "entryname2");
    GtkWidget *entry_first_name   = lookup_widget(window, "entryfirstname");
    GtkWidget *entry_phone_number = lookup_widget(window, "entryphonenumber");
    GtkWidget *spin_day           = lookup_widget(window, "spinbuttonday");
    GtkWidget *spin_month         = lookup_widget(window, "spinbuttonmonth");
    GtkWidget *spin_year          = lookup_widget(window, "spinbuttonyear");
    GtkWidget *combo_center       = lookup_widget(window, "entrycentre2");
    GtkWidget *label_status       = lookup_widget(window, "label99");

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    if (!is_number(id_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  ID must be numbers!"); g_timeout_add(3000, clear_label, label_status); return; }
    int id = atoi(id_text);

    Coach existing = searchCoach("coach.txt", id);
    if (existing.id == -1) { gtk_label_set_text(GTK_LABEL(label_status), "  Coach not found!"); g_timeout_add(3000, clear_label, label_status); return; }

    const gchar *name_text  = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const gchar *fname_text = gtk_entry_get_text(GTK_ENTRY(entry_first_name));
    const gchar *phone_text = gtk_entry_get_text(GTK_ENTRY(entry_phone_number));
    GtkEntry *entry_combo = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));

    Coach c = existing;

    // Ne modifier que si le champ n'est pas vide
    if (strlen(name_text) > 0) strcpy(c.lastName, name_text);
    if (strlen(fname_text) > 0) strcpy(c.firstName, fname_text);
    if (strlen(phone_text) > 0) strcpy(c.phoneNumber, phone_text);
    if (gtk_entry_get_text(entry_combo)[0] != '\0') strcpy(c.center, gtk_entry_get_text(entry_combo));

    // Date: modifier seulement si changé
    int day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    int month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    int year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));
    if (day != 1) c.dateOfBirth.day = day;
    if (month != 1) c.dateOfBirth.month = month;
    if (year != 2000) c.dateOfBirth.year = year;

    if (gender == 1) strcpy(c.gender, "Male");
    else strcpy(c.gender, "Female");

    if (modifyCoach("coach.txt", c.id, c))
        gtk_label_set_text(GTK_LABEL(label_status), "  Modify done!");
    else
        gtk_label_set_text(GTK_LABEL(label_status), "  Modify error!");
    g_timeout_add(3000, clear_label, label_status);

    // Reset entries (spin buttons gardent valeurs existantes)
    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    gtk_entry_set_text(GTK_ENTRY(entry_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_first_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_phone_number), "");
    gtk_entry_set_text(entry_combo, "");
}

// ----------------------------------------------------
//                     DELETE COACH
// ----------------------------------------------------
void on_btdelete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry_id = lookup_widget(window, "entryid");
    GtkWidget *label_status = lookup_widget(window, "label95");

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    if (!is_number(id_text)) { gtk_label_set_text(GTK_LABEL(label_status), "  ID must be numbers!"); g_timeout_add(3000, clear_label, label_status); return; }
    int id = atoi(id_text);

    if (deleteCoach("coach.txt", id))
        gtk_label_set_text(GTK_LABEL(label_status), "  Delete done!");
    else
        gtk_label_set_text(GTK_LABEL(label_status), "  Delete error!");
    g_timeout_add(3000, clear_label, label_status);

    // Reset entries
    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
}

// ----------------------------------------------------
//                     RADIO BUTTONS
// ----------------------------------------------------
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

