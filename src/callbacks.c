#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "coach.h"

int gender = 1; // 1 = Male, 2 = Female

// ===========================================
// CLEAR LABEL
// ===========================================
gboolean clear_label(gpointer data)
{
    gtk_label_set_text(GTK_LABEL(data), "");
    return FALSE;
}

// ===========================================
// VALIDATION INPUT
// ===========================================
int is_number(const char *str)
{
    if (str == NULL || str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isdigit(str[i])) return 0;
    return 1;
}

int is_letters(const char *str)
{
    if (str == NULL || str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalpha(str[i]) && str[i] != ' ') return 0;
    return 1;
}

// =========================================================
// CREATE COLUMNS (only once)
// =========================================================
void create_columns(GtkTreeView *treeview)
{
    if (g_list_length(gtk_tree_view_get_columns(treeview)) > 0)
        return;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    const char *titles[9] = {"ID", "Last Name", "First Name", "Day", "Month", "Year", "Center", "Phone", "Gender"};

    for (int i = 0; i < 9; i++)
    {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
    }
}

// =========================================================
// LOAD coaches.txt → TreeView
// =========================================================
void loadCoaches(GtkTreeView *treeview, const char *filename)
{
    // Vérifier si des colonnes existent déjà
    if (g_list_length(gtk_tree_view_get_columns(treeview)) > 0)
    {
        // Si déjà chargé, on recharge juste les données
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
        if (store)
        {
            gtk_list_store_clear(store);
        }
        else
        {
            return;
        }
    }
    else
    {
        // Créer les colonnes si elles n'existent pas - 9 COLONNES pour coaches
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        const char *titles[9] = {"ID", "Last Name", "First Name", "Day", "Month", "Year", "Center", "Phone", "Gender"};
        
        for (int i = 0; i < 9; i++)
        {
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_append_column(treeview, column);
        }
    }

    GtkListStore *store;
    GtkTreeIter iter;

    store = gtk_list_store_new(9,
                               G_TYPE_INT,    // ID
                               G_TYPE_STRING, // Last Name
                               G_TYPE_STRING, // First Name
                               G_TYPE_INT,    // Day
                               G_TYPE_INT,    // Month
                               G_TYPE_INT,    // Year
                               G_TYPE_STRING, // Center
                               G_TYPE_STRING, // Phone
                               G_TYPE_STRING  // Gender
    );

    FILE *f = fopen(filename, "r");
    if (f)
    {
        Coach c;
        char gender_str[10];
        
        while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s",
                      &c.id, c.lastName, c.firstName,
                      &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                      c.center, c.phoneNumber, gender_str) == 9)
        {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, c.id,
                               1, c.lastName,
                               2, c.firstName,
                               3, c.dateOfBirth.day,
                               4, c.dateOfBirth.month,
                               5, c.dateOfBirth.year,
                               6, c.center,
                               7, c.phoneNumber,
                               8, gender_str,
                               -1);
        }
        fclose(f);
    }
    else
    {
        printf("ERROR: Cannot open file %s\n", filename);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, -1,
                           1, "coach.txt not found",
                           2, "",
                           3, 0,
                           4, 0,
                           5, 0,
                           6, "",
                           7, "",
                           8, "",
                           -1);
    }

    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
}

// =========================================================
// REFRESH TREEVIEW
// ===========================================================
void refresh_tree(GtkWidget *treeview)
{
    loadCoaches(GTK_TREE_VIEW(treeview), "coach.txt");
}

// =========================================================
// LOAD COURSES → TreeView (7 COLONNES)
// =========================================================
void loadCourses(GtkTreeView *treeview, const char *filename)
{
    // Vérifier si des colonnes existent déjà
    if (g_list_length(gtk_tree_view_get_columns(treeview)) > 0)
    {
        // Si déjà chargé, on recharge juste les données
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
        if (store)
        {
            gtk_list_store_clear(store);
        }
        else
        {
            return;
        }
    }
    else
    {
        // Créer les colonnes si elles n'existent pas - 7 COLONNES
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        const char *titles[7] = {"ID", "Name", "Type", "Center", "Time", "Equipment", "Capacity"};
        
        for (int i = 0; i < 7; i++)
        {
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_append_column(treeview, column);
        }
    }

    GtkListStore *store;
    GtkTreeIter iter;

    store = gtk_list_store_new(7,
                               G_TYPE_STRING, // ID
                               G_TYPE_STRING, // Name
                               G_TYPE_STRING, // Type
                               G_TYPE_STRING, // Center
                               G_TYPE_STRING, // Time
                               G_TYPE_STRING, // Equipment
                               G_TYPE_STRING  // Capacity
    );

    FILE *f = fopen(filename, "r");
    if (f)
    {
        char line[256];
        // Lire l'en-tête et l'ignorer
        if (fgets(line, sizeof(line), f) != NULL)
        {
            // DEBUG: Afficher l'en-tête
            printf("DEBUG: Skipping header: %s", line);
            
            // Lire les données
            while (fgets(line, sizeof(line), f))
            {
                // Supprimer le saut de ligne
                line[strcspn(line, "\n")] = '\0';
                
                // DEBUG: Afficher la ligne lue
                printf("DEBUG: Reading line: %s\n", line);
                
                char course_id[50], course_name[50], course_type[50];
                char course_center[50], course_time[50], equipment[50], capacity[50];
                
                // Utiliser sscanf pour parser la ligne - 7 CHAMPS
                int fields = sscanf(line, "%49s %49s %49s %49s %49s %49s %49s", 
                                   course_id, course_name, course_type, 
                                   course_center, course_time, equipment, capacity);
                
                if (fields == 7)
                {
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                                       0, course_id,
                                       1, course_name,
                                       2, course_type,
                                       3, course_center,
                                       4, course_time,
                                       5, equipment,
                                       6, capacity,
                                       -1);
                    
                    // DEBUG: Afficher ce qui a été chargé
                    printf("DEBUG: Successfully loaded course: ID=%s, Name=%s, Capacity=%s\n",
                           course_id, course_name, capacity);
                }
                else
                {
                    printf("DEBUG: Error parsing line - expected 7 fields, got %d\n", fields);
                    printf("DEBUG: Line was: %s\n", line);
                }
            }
        }
        fclose(f);
    }
    else
    {
        // Fichier non trouvé, ajouter un message
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, "ERROR",
                           1, "courses.txt not found",
                           2, "",
                           3, "",
                           4, "",
                           5, "",
                           6, "",
                           -1);
    }

    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
}

// =========================================================
// REFRESH COURSE TREEVIEW
// =========================================================
void refresh_course_tree(GtkWidget *treeview)
{
    loadCourses(GTK_TREE_VIEW(treeview), "courses.txt");
}

// =========================================================
// ADD COACH
// ===========================================================
void on_btadd_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *spin_day = lookup_widget(win, "spinbuttonday");
    GtkWidget *spin_month = lookup_widget(win, "spinbuttonmonth");
    GtkWidget *spin_year = lookup_widget(win, "spinbuttonyear");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    GtkWidget *label = lookup_widget(win, "label98");

    if (!entryid || !entryname || !entryfirst || !entryphone || !spin_day || !spin_month || !spin_year || !combo_center || !label)
        return;

    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(entryname));
    const gchar *fname = gtk_entry_get_text(GTK_ENTRY(entryfirst));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entryphone));

    // Validation
    if (!is_number(id)) { gtk_label_set_text(GTK_LABEL(label), "ID must be numbers"); return; }
    if (!is_letters(name)) { gtk_label_set_text(GTK_LABEL(label), "Name must be letters"); return; }
    if (!is_letters(fname)) { gtk_label_set_text(GTK_LABEL(label), "First name must be letters"); return; }
    if (!is_number(phone)) { gtk_label_set_text(GTK_LABEL(label), "Phone must be numbers"); return; }
    
    // Check if ID already exists
    FILE *f = fopen("coach.txt", "r");
    if (f) {
        int existing_id;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            sscanf(buffer, "%d", &existing_id);
            if (existing_id == atoi(id)) {
                fclose(f);
                gtk_label_set_text(GTK_LABEL(label), "ID already exists!");
                return;
            }
        }
        fclose(f);
    }

    Coach c;
    c.id = atoi(id);
    strncpy(c.lastName, name, sizeof(c.lastName)-1);
    c.lastName[sizeof(c.lastName)-1] = '\0';
    strncpy(c.firstName, fname, sizeof(c.firstName)-1);
    c.firstName[sizeof(c.firstName)-1] = '\0';
    strncpy(c.phoneNumber, phone, sizeof(c.phoneNumber)-1);
    c.phoneNumber[sizeof(c.phoneNumber)-1] = '\0';

    c.dateOfBirth.day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));

    GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *center_text = gtk_entry_get_text(entry);
    if (center_text == NULL || strlen(center_text) == 0) {
        gtk_label_set_text(GTK_LABEL(label), "Please select a center!");
        return;
    }
    strncpy(c.center, center_text, sizeof(c.center)-1);
    c.center[sizeof(c.center)-1] = '\0';

    c.gender = gender; // int

    if (addCoach("coach.txt", c))
        gtk_label_set_text(GTK_LABEL(label), "Coach added successfully!");
    else
        gtk_label_set_text(GTK_LABEL(label), "Error adding coach!");

    g_timeout_add(2000, clear_label, label);

    // Clear fields
    gtk_entry_set_text(GTK_ENTRY(entryid), "");
    gtk_entry_set_text(GTK_ENTRY(entryname), "");
    gtk_entry_set_text(GTK_ENTRY(entryfirst), "");
    gtk_entry_set_text(GTK_ENTRY(entryphone), "");
    gtk_entry_set_text(entry, "");

    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
}

// =========================================================
// MODIFY COACH
// ===========================================================
void on_btmodify_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *spin_day = lookup_widget(win, "spinbuttonday");
    GtkWidget *spin_month = lookup_widget(win, "spinbuttonmonth");
    GtkWidget *spin_year = lookup_widget(win, "spinbuttonyear");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    GtkWidget *label = lookup_widget(win, "label99");

    if (!entryid || !entryname || !entryfirst || !entryphone || !spin_day || !spin_month || !spin_year || !combo_center || !label)
        return;

    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
    if (!is_number(id)) { gtk_label_set_text(GTK_LABEL(label), "ID must be numbers"); return; }

    Coach c;
    c.id = atoi(id);

    strncpy(c.lastName, gtk_entry_get_text(GTK_ENTRY(entryname)), sizeof(c.lastName)-1);
    c.lastName[sizeof(c.lastName)-1] = '\0';
    strncpy(c.firstName, gtk_entry_get_text(GTK_ENTRY(entryfirst)), sizeof(c.firstName)-1);
    c.firstName[sizeof(c.firstName)-1] = '\0';
    strncpy(c.phoneNumber, gtk_entry_get_text(GTK_ENTRY(entryphone)), sizeof(c.phoneNumber)-1);
    c.phoneNumber[sizeof(c.phoneNumber)-1] = '\0';

    c.dateOfBirth.day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));

    GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *center_text = gtk_entry_get_text(entry);
    if (center_text == NULL || strlen(center_text) == 0) {
        gtk_label_set_text(GTK_LABEL(label), "Please select a center!");
        return;
    }
    strncpy(c.center, center_text, sizeof(c.center)-1);
    c.center[sizeof(c.center)-1] = '\0';

    c.gender = gender; // int

    if (modifyCoach("coach.txt", c.id, c))
        gtk_label_set_text(GTK_LABEL(label), "Coach modified successfully!");
    else
        gtk_label_set_text(GTK_LABEL(label), "Error! ID not found!");

    g_timeout_add(2000, clear_label, label);

    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
}

// =========================================================
// DELETE COACH
// ===========================================================
void on_btdelete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *label = lookup_widget(win, "label95");

    if (!entryid || !label) return;

    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
    if (!is_number(id)) { gtk_label_set_text(GTK_LABEL(label), "ID must be numbers"); return; }

    if (deleteCoach("coach.txt", atoi(id)))
        gtk_label_set_text(GTK_LABEL(label), "Coach deleted successfully!");
    else
        gtk_label_set_text(GTK_LABEL(label), "Error! ID not found!");

    g_timeout_add(2000, clear_label, label);

    // Clear fields
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    
    if (entryname) gtk_entry_set_text(GTK_ENTRY(entryname), "");
    if (entryfirst) gtk_entry_set_text(GTK_ENTRY(entryfirst), "");
    if (entryphone) gtk_entry_set_text(GTK_ENTRY(entryphone), "");
    if (entryid) gtk_entry_set_text(GTK_ENTRY(entryid), "");
    if (combo_center) {
        GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
        gtk_entry_set_text(entry, "");
    }

    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
}

// =========================================================
// SEARCH COACH - CORRIGÉ (syntaxe fixée)
// ===========================================================
void on_btsearch_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entryname = lookup_widget(win, "entryname1");
    GtkWidget *combo = lookup_widget(win, "entrycentre1");

    if (!entryname || !combo) return;

    const gchar *name = gtk_entry_get_text(GTK_ENTRY(entryname));
    GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));
    const gchar *center = gtk_entry_get_text(entry);
    
    // Nettoyer les chaînes de recherche
    gchar *clean_name = NULL;
    gchar *clean_center = NULL;
    
    if (name && strlen(name) > 0) {
        clean_name = g_strdup(name);
        // Supprimer les espaces inutiles
        g_strstrip(clean_name);
    }
    
    if (center && strlen(center) > 0) {
        clean_center = g_strdup(center);
        // Supprimer les espaces inutiles
        g_strstrip(clean_center);
    }
    
    // DEBUG: Afficher ce qu'on recherche
    printf("DEBUG SEARCH: Searching for name='%s', center='%s'\n", 
           clean_name ? clean_name : "(empty)", 
           clean_center ? clean_center : "(empty)");
    
    // Vérifier qu'au moins un critère est fourni
    if ((!clean_name || strlen(clean_name) == 0) && 
        (!clean_center || strlen(clean_center) == 0)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_WARNING,
                                        GTK_BUTTONS_OK,
                                        "Please enter a name or select a center to search!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (clean_name) g_free(clean_name);
        if (clean_center) g_free(clean_center);
        return;
    }
    
    // Vérifier le contenu du fichier pour debug
    FILE *debug_f = fopen("coach.txt", "r");
    if (debug_f) {
        printf("DEBUG FILE CONTENT:\n");
        char line[256];
        while (fgets(line, sizeof(line), debug_f)) {
            printf("  %s", line);
        }
        fclose(debug_f);
    }

    Coach c = searchCoach("coach.txt", 
                         clean_name ? clean_name : "", 
                         clean_center ? clean_center : "");
    
    // DEBUG: Afficher le résultat
    if (c.id != -1) {
        printf("DEBUG FOUND: Coach found! ID=%d, Name=%s %s, Center='%s'\n", 
               c.id, c.firstName, c.lastName, c.center);
    } else {
        printf("DEBUG NOT FOUND: No coach found with criteria\n");
    }
    
    // Nettoyer la mémoire
    if (clean_name) g_free(clean_name);
    if (clean_center) g_free(clean_center);
    
    GtkWidget *dialog;

    if (c.id != -1)
    {
        char gender_str[10];
        strcpy(gender_str, (c.gender == 1) ? "Male" : "Female");
        
        // Créer le message avec les informations nettoyées
        gchar *message = g_strdup_printf(
            "Coach Found:\n\nID: %d\nName: %s %s\nDate of Birth: %d/%d/%d\nCenter: %s\nPhone: %s\nGender: %s",
            c.id, c.firstName, c.lastName, 
            c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
            c.center, c.phoneNumber, gender_str);
        
        dialog = gtk_message_dialog_new(GTK_WINDOW(win),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_OK,
                                        "%s", message);
        
        g_free(message);
    }
    else
        dialog = gtk_message_dialog_new(GTK_WINDOW(win),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_WARNING,
                                        GTK_BUTTONS_OK,
                                        "Coach not found!\n\nMake sure:\n1. The center name is correct\n2. Try without extra spaces\n3. Check file 'coach.txt' exists");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// =========================================================
// RADIO BUTTONS
// ===========================================================
void on_radiobuttonman_toggled(GtkToggleButton *t, gpointer data)
{
    if (gtk_toggle_button_get_active(t)) gender = 1;
}

void on_radiobuttonwomen_toggled(GtkToggleButton *t, gpointer data)
{
    if (gtk_toggle_button_get_active(t)) gender = 2;
}

// =========================================================
// ROW ACTIVATED
// ===========================================================
void on_treeviewmanage_row_activated(GtkTreeView *treeview,
                                     GtkTreePath *path,
                                     GtkTreeViewColumn *column,
                                     gpointer userdata)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);

    int id, day, month, year;
    char lname[30], fname[30], center[30], phone[20];
    char gender_str[10];

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter,
                           0, &id,
                           1, lname,
                           2, fname,
                           3, &day,
                           4, &month,
                           5, &year,
                           6, center,
                           7, phone,
                           8, gender_str,
                           -1);

        GtkWidget *window = lookup_widget(GTK_WIDGET(treeview), "raed_manage_coach");

        if (!window) return;

        GtkWidget *entryid = lookup_widget(window, "entryid");
        GtkWidget *entryname = lookup_widget(window, "entryname2");
        GtkWidget *entryfirst = lookup_widget(window, "entryfirstname");
        GtkWidget *entryphone = lookup_widget(window, "entryphonenumber");
        GtkWidget *spin_day = lookup_widget(window, "spinbuttonday");
        GtkWidget *spin_month = lookup_widget(window, "spinbuttonmonth");
        GtkWidget *spin_year = lookup_widget(window, "spinbuttonyear");
        GtkWidget *combo_center = lookup_widget(window, "entrycentre2");
        GtkWidget *radiobuttonman = lookup_widget(window, "radiobuttonman");
        GtkWidget *radiobuttonwomen = lookup_widget(window, "radiobuttonwomen");

        if (entryid) gtk_entry_set_text(GTK_ENTRY(entryid), g_strdup_printf("%d", id));
        if (entryname) gtk_entry_set_text(GTK_ENTRY(entryname), lname);
        if (entryfirst) gtk_entry_set_text(GTK_ENTRY(entryfirst), fname);
        if (entryphone) gtk_entry_set_text(GTK_ENTRY(entryphone), phone);
        
        if (spin_day) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_day), day);
        if (spin_month) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_month), month);
        if (spin_year) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_year), year);
        
        if (combo_center) {
            GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
            gtk_entry_set_text(entry, center);
        }
        
        if (strcmp(gender_str, "Male") == 0) {
            gender = 1;
            if (radiobuttonman) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobuttonman), TRUE);
            if (radiobuttonwomen) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobuttonwomen), FALSE);
        } else {
            gender = 2;
            if (radiobuttonman) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobuttonman), FALSE);
            if (radiobuttonwomen) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobuttonwomen), TRUE);
        }
    }
}

// =========================================================
// REFRESH BUTTON
// ===========================================================
void on_refresh_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
}

// =========================================================
// COURSE TREEVIEW ROW ACTIVATED (7 COLONNES)
// =========================================================
void on_treeview13_row_activated(GtkTreeView *treeview,
                                 GtkTreePath *path,
                                 GtkTreeViewColumn *column,
                                 gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);

    char course_id[50], course_name[50], course_type[50];
    char course_center[50], course_time[50], equipment[50], capacity[50];

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter,
                           0, course_id,
                           1, course_name,
                           2, course_type,
                           3, course_center,
                           4, course_time,
                           5, equipment,
                           6, capacity,
                           -1);

        // Afficher un message avec le cours sélectionné
        printf("DEBUG: Course selected: %s - %s at %s (Equipment: %s, Capacity: %s)\n", 
               course_id, course_name, course_time, equipment, capacity);
    }
}

// =========================================================
// BUTTON 40 CLICKED (Load Courses)
// =========================================================
void on_button40_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    // Trouver le treeview13
    GtkWidget *treeview13 = lookup_widget(win, "treeview13");
    if (!treeview13)
    {
        printf("Error: treeview13 not found!\n");
        return;
    }
    
    // Charger les cours (7 colonnes)
    loadCourses(GTK_TREE_VIEW(treeview13), "courses.txt");
    
    // Afficher un message de confirmation
    GtkWidget *label_status = lookup_widget(win, "label201");
    if (label_status)
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Courses loaded successfully! (7 columns)");
        g_timeout_add(2000, clear_label, label_status);
    }
    else
    {
        printf("Warning: Status label (label201) not found!\n");
    }
    
    printf("DEBUG: Courses loaded into treeview13 (7 columns)\n");
}

// =========================================================
// BUTTON 39 CLICKED (Choose/Assign Coach to Course) - CAPACITY CONTROL REMOVED
// =========================================================
// =========================================================
// BUTTON 39 CLICKED (Choose/Assign Coach to Course) - DATE AFTER PHONE
// =========================================================
// =========================================================
// BUTTON 39 CLICKED (Choose/Assign Coach to Course) - UTF-8 FIXED
// =========================================================
void on_button39_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    // Récupérer tous les widgets nécessaires
    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *entryname = lookup_widget(win, "entryname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *spin_day = lookup_widget(win, "spinbuttonday");
    GtkWidget *spin_month = lookup_widget(win, "spinbuttonmonth");
    GtkWidget *spin_year = lookup_widget(win, "spinbuttonyear");
    GtkWidget *treeview13 = lookup_widget(win, "treeview13");
    GtkWidget *label_status = lookup_widget(win, "label201");

    if (!entryid || !entryname || !entryphone || !spin_day || !spin_month || !spin_year || !treeview13 || !label_status)
    {
        printf("Error: Some widgets not found!\n");
        return;
    }

    // Validation des entrées
    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entryid));
    const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entryname));
    const gchar *phone_text = gtk_entry_get_text(GTK_ENTRY(entryphone));

    if (!id_text || strlen(id_text) == 0 || !is_number(id_text))
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Invalid Coach ID!");
        g_timeout_add(3000, clear_label, label_status);
        return;
    }

    if (!name_text || strlen(name_text) == 0 || !is_letters(name_text))
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Invalid coach name!");
        g_timeout_add(3000, clear_label, label_status);
        return;
    }

    if (!phone_text || strlen(phone_text) == 0 || !is_number(phone_text))
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Invalid phone number!");
        g_timeout_add(3000, clear_label, label_status);
        return;
    }

    // Vérifier si un cours est sélectionné
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview13));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Please select a course first!");
        g_timeout_add(3000, clear_label, label_status);
        return;
    }

    // Récupérer les informations du cours sélectionné - Méthode plus robuste
    gchar *course_id = NULL;
    gchar *course_name = NULL;
    gchar *course_type = NULL;
    gchar *course_center = NULL;
    gchar *course_time = NULL;
    gchar *equipment = NULL;
    gchar *capacity_text = NULL;
    
    gtk_tree_model_get(model, &iter,
                       0, &course_id,
                       1, &course_name,
                       2, &course_type,
                       3, &course_center,
                       4, &course_time,
                       5, &equipment,
                       6, &capacity_text,
                       -1);

    printf("\n=== DEBUG - COURSE INFO FROM TREEVIEW (RAW) ===\n");
    printf("Course ID: '%s'\n", course_id ? course_id : "(NULL)");
    printf("Course Name: '%s'\n", course_name ? course_name : "(NULL)");
    printf("Course Type: '%s'\n", course_type ? course_type : "(NULL)");
    printf("Course Center: '%s'\n", course_center ? course_center : "(NULL)");
    printf("Course Time: '%s'\n", course_time ? course_time : "(NULL)");
    printf("Equipment: '%s'\n", equipment ? equipment : "(NULL)");
    printf("Capacity: '%s'\n", capacity_text ? capacity_text : "(NULL)");
    printf("===============================================\n\n");

    // Vérifier si ce n'est pas un message d'erreur
    if (course_id && strcmp(course_id, "ERROR") == 0)
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Please load courses first!");
        g_timeout_add(3000, clear_label, label_status);
        
        // Libérer la mémoire
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }

    // Vérifier que les champs ne sont pas vides
    if (!course_id || strlen(course_id) == 0 || 
        !course_name || strlen(course_name) == 0 || 
        !course_center || strlen(course_center) == 0 || 
        !course_time || strlen(course_time) == 0)
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Invalid course data selected!");
        g_timeout_add(3000, clear_label, label_status);
        
        // Libérer la mémoire
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }

    int coach_id = atoi(id_text);
    int day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    int month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    int year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));
    
    // Nettoyer les chaînes UTF-8
    gchar *clean_course_id = g_strdup(course_id ? course_id : "");
    gchar *clean_course_name = g_strdup(course_name ? course_name : "");
    gchar *clean_course_center = g_strdup(course_center ? course_center : "");
    gchar *clean_course_time = g_strdup(course_time ? course_time : "");
    
    if (clean_course_id) g_strstrip(clean_course_id);
    if (clean_course_name) g_strstrip(clean_course_name);
    if (clean_course_center) g_strstrip(clean_course_center);
    if (clean_course_time) g_strstrip(clean_course_time);
    
    printf("\n=== DEBUG - ASSIGNMENT TO SAVE (CLEANED) ===\n");
    printf("Coach ID: %d\n", coach_id);
    printf("Coach Name: '%s'\n", name_text);
    printf("Coach Phone: '%s'\n", phone_text);
    printf("Date: %d/%d/%d\n", day, month, year);
    printf("Course ID: '%s'\n", clean_course_id);
    printf("Course Name: '%s'\n", clean_course_name);
    printf("Course Center: '%s'\n", clean_course_center);
    printf("Course Time: '%s'\n", clean_course_time);
    printf("==========================================\n\n");

    // Sauvegarder dans le fichier avec le bon format:
    // CoachID|NomCoach|Phone|Jour|Mois|Année|CourseID|CourseNom|Centre|Heure
    FILE *f = fopen("coach_assignment.txt", "a");
    if (!f)
    {
        gtk_label_set_text(GTK_LABEL(label_status), "Error creating assignment file!");
        g_timeout_add(3000, clear_label, label_status);
        
        // Libérer la mémoire
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        g_free(clean_course_id);
        g_free(clean_course_name);
        g_free(clean_course_center);
        g_free(clean_course_time);
        return;
    }

    // Écrire avec le format: ID|Nom|Phone|Jour|Mois|Année|CourseID|CourseName|Centre|Heure
    int written = fprintf(f, "%d|%s|%s|%d|%d|%d|%s|%s|%s|%s\n",
            coach_id,           // ID du coach
            name_text,          // Nom du coach
            phone_text,         // Téléphone du coach
            day,                // Jour
            month,              // Mois
            year,               // Année
            clean_course_id,    // ID du cours
            clean_course_name,  // Nom du cours
            clean_course_center,// Centre
            clean_course_time); // Heure
    
    if (written < 0) {
        printf("ERROR: Failed to write to file!\n");
    } else {
        printf("DEBUG: Written %d characters to file\n", written);
    }
    
    fclose(f);

    // Vérifier le contenu du fichier
    printf("=== DEBUG - FILE CONTENT AFTER WRITE ===\n");
    FILE *check = fopen("coach_assignment.txt", "r");
    if (check) {
        char line[256];
        int line_num = 1;
        while (fgets(line, sizeof(line), check)) {
            // Supprimer le saut de ligne pour l'affichage
            line[strcspn(line, "\n")] = '\0';
            printf("Line %d: %s\n", line_num, line);
            
            // Afficher chaque caractère en hexadécimal pour debug
            printf("  Hex: ");
            for (int i = 0; i < strlen(line) && i < 50; i++) {
                printf("%02X ", (unsigned char)line[i]);
            }
            printf("\n");
            
            line_num++;
        }
        fclose(check);
    }
    printf("=======================================\n\n");

    // Succès
    gtk_label_set_text(GTK_LABEL(label_status), "Coach successfully assigned to course!");
    
    // Effacer les champs
    gtk_entry_set_text(GTK_ENTRY(entryid), "");
    gtk_entry_set_text(GTK_ENTRY(entryname), "");
    gtk_entry_set_text(GTK_ENTRY(entryphone), "");
    
    g_timeout_add(3000, clear_label, label_status);
    
    // Libérer toute la mémoire
    g_free(course_id);
    g_free(course_name);
    g_free(course_type);
    g_free(course_center);
    g_free(course_time);
    g_free(equipment);
    g_free(capacity_text);
    g_free(clean_course_id);
    g_free(clean_course_name);
    g_free(clean_course_center);
    g_free(clean_course_time);
    
    printf("DEBUG: Coach %d assigned to course %s (%s) on %d/%d/%d\n", 
           coach_id, clean_course_id, clean_course_name, day, month, year);
}
