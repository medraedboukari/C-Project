#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <stddef.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "coach.h"
#include "equipment.h"
#include "course.h"

// =========================================================
// USER DATABASE STRUCTURE
// =========================================================
typedef struct {
    char *username;
    char *password;
    char *description;
    GtkWidget* (*create_window_func)(void);
} UserCredentials;

// User database - all your users
UserCredentials users[] = {
    {"raedcoach", "raed123", "Manage Coach", create_raed_manage_coach},
    {"raedcoach", "raed2025", "Reserve Material", create_raed_reserve_materiel},
    {"admin", "admin123", "Admin", create_windowadmin},
    {"trainer", "trainer123", "Trainer", create_windowtrainer},
    {"yomna", "yomna123", "Course Managment", create_yomna_course_managment},
    {"yomna", "yomna2025", "Choose Course", create_yomna_choose_course},
    {NULL, NULL, NULL, NULL} // End marker
};

// =========================================================
// VARIABLES GLOBALES
// =========================================================
int gender = 1; // 1 = Male, 2 = Female
int time_c = 0; // Pour les cours
int equipment_c = 0; // Pour les cours

// Variables pour équipements
GtkWidget *current_window = NULL;
char selected_role[20] = "";
int remember_me = 0;

// Variables pour stocker les valeurs courantes (équipements)
char current_id[20] = "";
char current_name[50] = "";
char current_centre[50] = "";
char current_type[50] = "";
int current_quantity = 1;
int current_day = 1;
int current_month = 1;
int current_year = 2024;
char current_state[20] = "Good";
int search_by_name_eq = 0; // Renommé pour éviter conflit avec cours
int search_by_centre_eq = 0; // Renommé pour éviter conflit avec cours
char current_user_id[20];

// Static variables for tree stores
static GtkTreeStore *store_management = NULL;
static GtkTreeStore *store_selection = NULL;
static GtkTreeStore *store_search = NULL;

// =========================================================
// FONCTIONS UTILITAIRES GÉNÉRALES
// =========================================================

// Fonction helper pour lire les données
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    const char **data = (const char **)userp;
    size_t room = size * nmemb;
    
    if(!data || !*data || **data == '\0')
        return 0;
    
    size_t len = strlen(*data);
    if(room < len)
        len = room;
    
    memcpy(ptr, *data, len);
    *data += len;
    
    return len;
}

// =========================================================
// AFFICHER UN MESSAGE DANS UNE POP-UP
// =========================================================
void show_popup_message(GtkWidget *parent, const char *title, const char *message, GtkMessageType type) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent ? GTK_WINDOW(parent) : NULL,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "%s", message);
    
    if (title) {
        gtk_window_set_title(GTK_WINDOW(dialog), title);
    }
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// =========================================================
// AFFICHER UNE POP-UP DE CONFIRMATION (YES/NO)
// =========================================================
gboolean show_confirmation_dialog(GtkWidget *parent, const char *title, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent ? GTK_WINDOW(parent) : NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "%s", message);
    
    if (title) {
        gtk_window_set_title(GTK_WINDOW(dialog), title);
    }
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    return (response == GTK_RESPONSE_YES);
}

// ===========================================
// VALIDATION INPUT - AVEC CONTRÔLE MAX 8 CHIFFRES
// ===========================================
int is_number(const char *str) {
    if (str == NULL || str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isdigit(str[i])) return 0;
    return 1;
}

int is_valid_id(const char *str) {
    if (!is_number(str)) return 0;
    if (strlen(str) > 8) return 0;  // Maximum 8 chiffres
    if (strlen(str) == 0) return 0; // Ne peut pas être vide
    return 1;
}

int is_valid_phone(const char *str) {
    if (!is_number(str)) return 0;
    if (strlen(str) != 8) return 0;  // Doit être exactement 8 chiffres (format Tunisien)
    return 1;
}

int is_letters(const char *str) {
    if (str == NULL || str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalpha(str[i]) && str[i] != ' ') return 0;
    return 1;
}

// ===========================================
// CLEAR LABEL
// ===========================================
gboolean clear_label(gpointer data) {
    gtk_label_set_text(GTK_LABEL(data), "");
    return FALSE;
}

// =========================================================
// FONCTION DE DEBUG: Afficher le contenu du fichier coach.txt
// =========================================================
void debug_coach_file() {
    printf("\n=== DEBUG coach.txt ===\n");
    FILE *f = fopen("coach.txt", "r");
    if (!f) {
        printf("ERROR: Cannot open coach.txt (errno: %d)\n", errno);
        return;
    }
    
    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        line[strcspn(line, "\n")] = '\0';
        printf("Line %d: %s\n", line_num, line);
        
        // Essayer de parser pour vérifier le format
        int id, day, month, year;
        char lastName[30], firstName[30], center[30], phone[20], gender[10], specialty[30];
        
        int fields = sscanf(line, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                           &id, lastName, firstName, &day, &month, &year,
                           center, phone, gender, specialty);
        
        printf("  Parsed %d fields: ID=%d, Name=%s %s, Specialty=%s\n", 
               fields, id, firstName, lastName, specialty);
    }
    fclose(f);
    printf("=== END DEBUG ===\n\n");
}

// =========================================================
// FONCTION DE DEBUG: Afficher le contenu du fichier d'assignation
// =========================================================
void debug_assignment_file() {
    printf("\n=== DEBUG coach_assignment.txt ===\n");
    FILE *f = fopen("coach_assignment.txt", "r");
    if (!f) {
        printf("File does not exist or cannot be opened\n");
        return;
    }
    
    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        line[strcspn(line, "\n")] = '\0';
        printf("Line %d: %s\n", line_num, line);
    }
    fclose(f);
    printf("=== END DEBUG ===\n\n");
}

// ===========================================
// SEND EMAIL TO ADMIN - FOR ADD/MODIFY/DELETE ACTIONS
// ===========================================
int send_email_to_admin(const char *subject, const char *body, const char *action_type) {
    printf("\n=== DEBUG EMAIL TO ADMIN START (%s) ===\n", action_type);
    printf("Subject: %s\n", subject);
    
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        // Configuration Gmail SMTP
        curl_easy_setopt(curl, CURLOPT_USERNAME, "topgym3b2@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "geutzwptyybqiffa");
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<topgym3b2@gmail.com>");
        
        // Debug SMTP
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        // Destinataire - ADMINISTRATION SEULEMENT
        recipients = curl_slist_append(recipients, "<raedboukari2018@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Construire le message pour l'administration
        char message[2048];
        const char *action_emoji = "📋";
        const char *action_title = "SYSTEM NOTIFICATION";
        
        if (strcmp(action_type, "add") == 0) {
            action_emoji = "➕";
            action_title = "NEW COACH ADDED";
        } else if (strcmp(action_type, "modify") == 0) {
            action_emoji = "✏️";
            action_title = "COACH MODIFIED";
        } else if (strcmp(action_type, "delete") == 0) {
            action_emoji = "🗑️";
            action_title = "COACH DELETED";
        }
        
        snprintf(message, sizeof(message),
                 "To: Administrator <raedboukari2018@gmail.com>\r\n"
                 "From: TopGym System <topgym3b2@gmail.com>\r\n"
                 "Subject: %s\r\n"
                 "Content-Type: text/plain; charset=UTF-8\r\n"
                 "\r\n"
                 "%s %s %s\r\n"
                 "══════════════════════════════════════════\r\n"
                 "\r\n"
                 "%s\r\n"
                 "\r\n"
                 "📍 This is an automated notification from the TopGym management system.\r\n"
                 "📍 Database has been updated accordingly.\r\n"
                 "\r\n"
                 "📊 System Status: Action completed\r\n"
                 "✅ Action: %s\r\n"
                 "\r\n"
                 "No further action required. This email is for your information only.\r\n"
                 "\r\n"
                 "Best regards,\r\n"
                 "🏋️ TopGym Automated System 🏋️\r\n"
                 "\r\n"
                 ".\r\n",
                 subject, action_emoji, action_title, action_emoji, body, action_type);
        
        printf("DEBUG: Email message size: %ld bytes\n", strlen(message));
        
        const char *email_data = message;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        printf("DEBUG: Starting email transmission to admin...\n");
        
        res = curl_easy_perform(curl);
        
        printf("DEBUG: curl_easy_perform returned: %d\n", res);
        if(res != CURLE_OK) {
            fprintf(stderr, "EMAIL ERROR: %s\n", curl_easy_strerror(res));
        } else {
            printf("✅ SUCCESS: Email sent to admin successfully!\n");
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    } else {
        printf("ERROR: Failed to initialize curl\n");
    }
    
    curl_global_cleanup();
    printf("=== DEBUG EMAIL TO ADMIN END ===\n\n");
    return (res == CURLE_OK);
}

// ===========================================
// SEND EMAIL TO COACH - FOR ASSIGNMENT ONLY (with copy to admin)
// ===========================================
int send_email_to_coach(const char *coach_email, const char *subject, const char *body) {
    printf("\n=== DEBUG EMAIL TO COACH START ===\n");
    printf("Coach Email: %s\n", coach_email);
    printf("Subject: %s\n", subject);
    
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        // Gmail SMTP Configuration
        curl_easy_setopt(curl, CURLOPT_USERNAME, "topgym3b2@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "geutzwptyybqiffa");
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<topgym3b2@gmail.com>");
        
        // Debug SMTP
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        // Destinataire - LE COACH ET L'ADMIN EN COPIE
        char coach_email_full[256];
        if (coach_email && strlen(coach_email) > 0) {
            snprintf(coach_email_full, sizeof(coach_email_full), "<%s>", coach_email);
        } else {
            snprintf(coach_email_full, sizeof(coach_email_full), "<coach@topgym.com>");
        }
        
        recipients = curl_slist_append(recipients, coach_email_full);
        recipients = curl_slist_append(recipients, "<raedboukari2018@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Construire le message pour le coach (avec admin en copie)
        char message[2048];
        snprintf(message, sizeof(message),
                 "To: Coach %s\r\n"
                 "Cc: Administrator <raedboukari2018@gmail.com>\r\n"
                 "From: TopGym Management <topgym3b2@gmail.com>\r\n"
                 "Subject: %s\r\n"
                 "Content-Type: text/plain; charset=UTF-8\r\n"
                 "\r\n"
                 "🏋️ HELLO DEAR COACH! 🏋️\r\n"
                 "══════════════════════════════════════════\r\n"
                 "\r\n"
                 "📋 ASSIGNMENT CONFIRMATION\r\n"
                 "══════════════════════════════════════════\r\n"
                 "\r\n"
                 "%s\r\n"
                 "\r\n"
                 "📍 IMPORTANT INFORMATION:\r\n"
                 "• Please arrive 15 minutes before the class\r\n"
                 "• Bring your personal equipment if needed\r\n"
                 "• Contact the center if unavailable\r\n"
                 "\r\n"
                 "📞 CENTER CONTACT:\r\n"
                 "• Phone: +216 72 152 458\r\n"
                 "• Email: contact@topgym.com\r\n"
                 "\r\n"
                 "Thank you for your commitment!\r\n"
                 "\r\n"
                 "Best regards,\r\n"
                 "TopGym Management Team\r\n"
                 "🏆 Your success is our priority!\r\n"
                 "\r\n"
                 ".\r\n",
                 coach_email, subject, body);
        
        printf("DEBUG: Email message size: %ld bytes\n", strlen(message));
        
        const char *email_data = message;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        printf("DEBUG: Starting email transmission to coach (with admin copy)...\n");
        
        res = curl_easy_perform(curl);
        
        printf("DEBUG: curl_easy_perform returned: %d\n", res);
        if(res != CURLE_OK) {
            fprintf(stderr, "EMAIL ERROR: %s\n", curl_easy_strerror(res));
        } else {
            printf("✅ SUCCESS: Email sent to coach (with admin copy) successfully!\n");
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    } else {
        printf("ERROR: Failed to initialize curl\n");
    }
    
    curl_global_cleanup();
    printf("=== DEBUG EMAIL TO COACH END ===\n\n");
    return (res == CURLE_OK);
}

// =========================================================
// Mettre à jour le coach dans le fichier
// =========================================================
void update_coach_in_file(int coach_id, int column_index, const char *new_value) {
    printf("\n=== DEBUG update_coach_in_file START ===\n");
    printf("Updating coach ID %d, column %d, new value: %s\n", coach_id, column_index, new_value);
    
    FILE *f = fopen("coach.txt", "r");
    FILE *f2 = fopen("coach_temp.txt", "w");
    
    if (!f || !f2) {
        printf("ERROR: Cannot open files\n");
        if (f) fclose(f);
        if (f2) fclose(f2);
        return;
    }
    
    Coach c;
    char gender_str[10];
    char specialty[30];
    int updated = 0;
    
    while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, gender_str, specialty) == 10) {
        
        if (c.id == coach_id) {
            updated = 1;
            
            // Mettre à jour le champ approprié
            switch(column_index) {
                case 1: // ID
                    c.id = atoi(new_value);
                    break;
                case 2: // Last Name
                    strncpy(c.lastName, new_value, sizeof(c.lastName)-1);
                    c.lastName[sizeof(c.lastName)-1] = '\0';
                    break;
                case 3: // First Name
                    strncpy(c.firstName, new_value, sizeof(c.firstName)-1);
                    c.firstName[sizeof(c.firstName)-1] = '\0';
                    break;
                case 4: // Day
                    c.dateOfBirth.day = atoi(new_value);
                    break;
                case 5: // Month
                    c.dateOfBirth.month = atoi(new_value);
                    break;
                case 6: // Year
                    c.dateOfBirth.year = atoi(new_value);
                    break;
                case 7: // Center
                    strncpy(c.center, new_value, sizeof(c.center)-1);
                    c.center[sizeof(c.center)-1] = '\0';
                    break;
                case 8: // Phone
                    if (!is_valid_phone(new_value)) {
                        printf("WARNING: Invalid phone number: %s\n", new_value);
                    }
                    strncpy(c.phoneNumber, new_value, sizeof(c.phoneNumber)-1);
                    c.phoneNumber[sizeof(c.phoneNumber)-1] = '\0';
                    break;
                case 9: // Gender
                    strncpy(gender_str, new_value, sizeof(gender_str)-1);
                    gender_str[sizeof(gender_str)-1] = '\0';
                    break;
                case 10: // Specialty
                    strncpy(specialty, new_value, sizeof(specialty)-1);
                    specialty[sizeof(specialty)-1] = '\0';
                    break;
            }
        }
        
        fprintf(f2, "%d %s %s %d %d %d %s %s %s %s\n",
                c.id, c.lastName, c.firstName,
                c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                c.center, c.phoneNumber, gender_str, specialty);
    }
    
    fclose(f);
    fclose(f2);
    
    if (updated) {
        remove("coach.txt");
        rename("coach_temp.txt", "coach.txt");
        printf("SUCCESS: Coach updated in file\n");
        
        // Envoyer un email de notification
        char email_body[512];
        const char *column_names[] = {"", "ID", "Last Name", "First Name", "Day", "Month", "Year", 
                                      "Center", "Phone", "Gender", "Specialty"};
        
        snprintf(email_body, sizeof(email_body),
                 "Coach information modified via TreeView:\n"
                 "• Coach ID: %d\n"
                 "• Field modified: %s\n"
                 "• New value: %s\n"
                 "• Timestamp: %s",
                 coach_id, column_names[column_index], new_value, __DATE__);
        
        send_email_to_admin("Coach Modified via TreeView", email_body, "modify");
    } else {
        remove("coach_temp.txt");
        printf("WARNING: Coach not found in file\n");
    }
    
    printf("=== DEBUG update_coach_in_file END ===\n\n");
}

// =========================================================
// FONCTIONS POUR CHECKBOX DANS TREEVIEW (COACH SEULEMENT)
// =========================================================

// Fonction appelée quand on clique sur une checkbox
void on_cell_toggled_coach(GtkCellRendererToggle *cell, gchar *path_str, gpointer data) {
    if (!cell || !path_str || !data) return;
    
    GtkWidget *treeview = GTK_WIDGET(data);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return;
    
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    if (!path) return;
    
    GtkTreeIter iter;
    gboolean active;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &active, -1);
        active = !active;
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, active, -1);
    }
    
    gtk_tree_path_free(path);
}

// Fonction appelée quand on modifie une cellule
void on_cell_edited_coach(GtkCellRendererText *cell, gchar *path_str, gchar *new_text, gpointer data) {
    if (!cell || !path_str || !data) return;
    
    GtkWidget *treeview = GTK_WIDGET(data);
    GtkWidget *window = gtk_widget_get_toplevel(treeview);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return;
    
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    if (!path) {
        printf("ERROR: Invalid path string: %s\n", path_str);
        return;
    }
    
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_path_free(path);
        return;
    }
    
    // Récupérer l'ID du coach
    int coach_id = -1;
    gtk_tree_model_get(model, &iter, 1, &coach_id, -1);
    
    if (coach_id <= 0) {
        gtk_tree_path_free(path);
        return;
    }
    
    // Demander confirmation
    if (!show_confirmation_dialog(window, "Confirm Modification", 
                                 "Do you want to save changes to this coach?")) {
        gtk_tree_path_free(path);
        return;
    }
    
    // Trouver quelle colonne a été modifiée
    int column_index = -1;
    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
    if (columns) {
        GList *current = columns;
        int col_num = 0;
        while (current) {
            GtkTreeViewColumn *col = current->data;
            if (col) {
                GList *cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(col));
                if (cells) {
                    GList *cell_current = cells;
                    while (cell_current) {
                        if (GTK_IS_CELL_RENDERER_TEXT(cell_current->data) && 
                            cell_current->data == GTK_CELL_RENDERER(cell)) {
                            column_index = col_num;
                            break;
                        }
                        cell_current = cell_current->next;
                    }
                    g_list_free(cells);
                }
            }
            if (column_index != -1) break;
            current = current->next;
            col_num++;
        }
        g_list_free(columns);
    }
    
    if (column_index == -1) {
        printf("ERROR: Could not determine which column was edited\n");
        gtk_tree_path_free(path);
        return;
    }
    
    printf("DEBUG: Editing column %d for coach ID %d\n", column_index, coach_id);
    
    // Mettre à jour la TreeView
    if (column_index == 1) { // ID (doit être un entier)
        if (!is_number(new_text)) {
            show_popup_message(window, "Error", "ID must be a number!", GTK_MESSAGE_ERROR);
            gtk_tree_path_free(path);
            return;
        }
        int new_id = atoi(new_text);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, column_index, new_id, -1);
    } else if (column_index >= 4 && column_index <= 6) { // Day, Month, Year (entiers)
        if (!is_number(new_text)) {
            show_popup_message(window, "Error", "Day, Month and Year must be numbers!", GTK_MESSAGE_ERROR);
            gtk_tree_path_free(path);
            return;
        }
        int value = atoi(new_text);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, column_index, value, -1);
    } else {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, column_index, new_text, -1);
    }
    
    // Mettre à jour le fichier
    update_coach_in_file(coach_id, column_index, new_text);
    
    gtk_tree_path_free(path);
}

// Fonction pour initialiser les colonnes avec checkbox et édition
void create_columns_with_checkbox_coach(GtkTreeView *treeview, const char **titles, int num_columns) {
    if (!treeview || !titles || num_columns <= 0) {
        printf("ERROR: Invalid parameters in create_columns_with_checkbox_coach\n");
        return;
    }
    
    // Supprimer les colonnes existantes de manière sécurisée
    GList *columns = gtk_tree_view_get_columns(treeview);
    if (columns) {
        GList *current = columns;
        while (current != NULL) {
            GtkTreeViewColumn *col = current->data;
            if (col) {
                gtk_tree_view_remove_column(treeview, col);
            }
            current = current->next;
        }
        g_list_free(columns);
    }
    
    // Ajouter la colonne checkbox
    GtkCellRenderer *check_renderer = gtk_cell_renderer_toggle_new();
    if (!check_renderer) {
        printf("ERROR: Failed to create checkbox renderer\n");
        return;
    }
    
    GtkTreeViewColumn *check_column = gtk_tree_view_column_new_with_attributes(
        "Select", check_renderer, "active", 0, NULL);
    if (check_column) {
        gtk_tree_view_append_column(treeview, check_column);
        g_signal_connect(check_renderer, "toggled", G_CALLBACK(on_cell_toggled_coach), treeview);
    }
    
    // Ajouter les autres colonnes avec édition
    for (int i = 1; i < num_columns; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        if (!renderer) {
            printf("ERROR: Failed to create renderer for column %d\n", i);
            continue;
        }
        
        // Rendre les cellules éditables
        g_object_set(renderer, "editable", TRUE, NULL);
        g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited_coach), treeview);
        
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        if (column) {
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_append_column(treeview, column);
        }
    }
}

// Obtenir les IDs sélectionnés via les checkbox (coach seulement)
GList* get_selected_coach_ids(GtkWidget *treeview) {
    GList *selected_ids = NULL;
    
    if (!treeview || !GTK_IS_TREE_VIEW(treeview)) {
        printf("ERROR: Invalid treeview in get_selected_coach_ids\n");
        return NULL;
    }
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return NULL;
    
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
    
    while (valid) {
        gboolean selected = FALSE;
        int id = -1;
        
        gtk_tree_model_get(model, &iter, 0, &selected, 1, &id, -1);
        
        if (selected && id > 0) {
            selected_ids = g_list_append(selected_ids, GINT_TO_POINTER(id));
        }
        
        valid = gtk_tree_model_iter_next(model, &iter);
    }
    
    return selected_ids;
}

// =========================================================
// CREATE COLUMNS POUR COURS (sans checkbox et sans édition)
// =========================================================
void create_columns(GtkTreeView *treeview) {
    if (!treeview) return;
    
    if (g_list_length(gtk_tree_view_get_columns(treeview)) > 0)
        return;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    const char *titles[7] = {"ID", "Name", "Type", "Center", "Time", "Equipment", "Capacity"};

    for (int i = 0; i < 7; i++) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
    }
}

// =========================================================
// LOAD coaches.txt → TreeView - AVEC CHECKBOX ET ÉDITION
// =========================================================
void loadCoaches(GtkTreeView *treeview, const char *filename) {
    printf("\n=== DEBUG loadCoaches START ===\n");
    printf("Loading from: %s\n", filename);
    
    if (!treeview) {
        printf("ERROR: TreeView is NULL\n");
        return;
    }
    
    // Supprimer le modèle existant avant de créer un nouveau
    GtkTreeModel *old_model = gtk_tree_view_get_model(treeview);
    if (old_model) {
        g_object_unref(old_model);
    }
    
    // Créer le store avec checkbox (première colonne = booléen)
    GtkListStore *store = gtk_list_store_new(11,
                               G_TYPE_BOOLEAN,    // 0: Checkbox
                               G_TYPE_INT,        // 1: ID
                               G_TYPE_STRING,     // 2: Last Name
                               G_TYPE_STRING,     // 3: First Name
                               G_TYPE_INT,        // 4: Day
                               G_TYPE_INT,        // 5: Month
                               G_TYPE_INT,        // 6: Year
                               G_TYPE_STRING,     // 7: Center
                               G_TYPE_STRING,     // 8: Phone
                               G_TYPE_STRING,     // 9: Gender
                               G_TYPE_STRING);    // 10: Specialty

    FILE *f = fopen(filename, "r");
    if (f) {
        Coach c;
        char gender_str[10];
        char specialty[30];
        int line_count = 0;
        
        printf("DEBUG: File opened successfully\n");
        
        while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                      &c.id, c.lastName, c.firstName,
                      &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                      c.center, c.phoneNumber, gender_str, specialty) == 10) {
            line_count++;
            printf("DEBUG: Loaded coach %d: %s %s (Specialty: %s)\n", 
                   c.id, c.firstName, c.lastName, specialty);
            
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, FALSE,           // Checkbox non cochée par défaut
                               1, c.id,
                               2, c.lastName,
                               3, c.firstName,
                               4, c.dateOfBirth.day,
                               5, c.dateOfBirth.month,
                               6, c.dateOfBirth.year,
                               7, c.center,
                               8, c.phoneNumber,
                               9, gender_str,
                               10, specialty,
                               -1);
        }
        fclose(f);
        
        printf("DEBUG: Total coaches loaded: %d\n", line_count);
        
        if (line_count == 0) {
            printf("WARNING: File exists but no coaches loaded\n");
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, FALSE,
                               1, -1,
                               2, "No coaches found",
                               3, "",
                               4, 0,
                               5, 0,
                               6, 0,
                               7, "",
                               8, "",
                               9, "",
                               10, "",
                               -1);
        }
    } else {
        printf("ERROR: Cannot open file %s (errno: %d)\n", filename, errno);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, FALSE,
                           1, -1,
                           2, "coach.txt not found",
                           3, "",
                           4, 0,
                           5, 0,
                           6, 0,
                           7, "",
                           8, "",
                           9, "",
                           10, "",
                           -1);
    }

    // Définir le modèle avant de créer les colonnes
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    
    // Créer les colonnes avec checkbox et édition (UNIQUEMENT POUR COACH)
    const char *titles[11] = {"Select", "ID", "Last Name", "First Name", "Day", "Month", "Year", 
                              "Center", "Phone", "Gender", "Specialty"};
    create_columns_with_checkbox_coach(treeview, titles, 11);
    
    // NE PAS unref le store ici - il est géré par GTK maintenant
    // g_object_unref(store);
    
    printf("=== DEBUG loadCoaches END ===\n");
}

// =========================================================
// REFRESH TREEVIEW
// =========================================================
void refresh_tree(GtkWidget *treeview) {
    printf("\n=== DEBUG refresh_tree ===\n");
    if (!treeview) {
        printf("ERROR: Treeview is NULL\n");
        return;
    }
    loadCoaches(GTK_TREE_VIEW(treeview), "coach.txt");
}

// =========================================================
// LOAD COURSES → TreeView (7 COLONNES SANS CHECKBOX ET SANS ÉDITION)
// =========================================================
void loadCourses(GtkTreeView *treeview, const char *filename) {
    printf("\n=== DEBUG loadCourses START ===\n");
    
    if (!treeview) {
        printf("ERROR: TreeView is NULL\n");
        return;
    }
    
    // Créer le store
    GtkListStore *store = gtk_list_store_new(7,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING);

    FILE *f = fopen(filename, "r");
    if (f) {
        char line[256];
        // Lire l'en-tête
        if (fgets(line, sizeof(line), f) != NULL) {
            printf("DEBUG: Skipping header: %s", line);
            
            // Lire les données
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = '\0';
                
                char course_id[50], course_name[50], course_type[50];
                char course_center[50], course_time[50], equipment[50], capacity[50];
                
                int fields = sscanf(line, "%49s %49s %49s %49s %49s %49s %49s", 
                                   course_id, course_name, course_type, 
                                   course_center, course_time, equipment, capacity);
                
                if (fields == 7) {
                    GtkTreeIter iter;
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
                }
            }
        }
        fclose(f);
    } else {
        GtkTreeIter iter;
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

    // Définir le modèle
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    
    // Créer les colonnes si nécessaire
    if (g_list_length(gtk_tree_view_get_columns(treeview)) == 0) {
        create_columns(treeview);
    }
    
    // NE PAS unref le store ici
    // g_object_unref(store);
    
    printf("=== DEBUG loadCourses END ===\n");
}

// =========================================================
// REFRESH COURSE TREEVIEW
// =========================================================
void refresh_course_tree(GtkWidget *treeview) {
    if (!treeview) return;
    loadCourses(GTK_TREE_VIEW(treeview), "courses.txt");
}

// =========================================================
// Mettre à jour la capacité d'un cours
// =========================================================
int update_course_capacity(const char *course_id, int change_amount) {
    if (!course_id || strlen(course_id) == 0) {
        return 0;
    }
    
    FILE *f = fopen("courses.txt", "r");
    if (!f) {
        printf("ERROR: Cannot open courses.txt for reading\n");
        return 0;
    }
    
    char lines[100][256];
    int line_count = 0;
    char line[256];
    
    // Lire l'en-tête
    if (fgets(line, sizeof(line), f)) {
        strncpy(lines[line_count++], line, sizeof(lines[0])-1);
    }
    
    int updated = 0;
    
    // Lire les données
    while (fgets(line, sizeof(line), f) && line_count < 100) {
        char current_course_id[50], course_name[50], course_type[50];
        char course_center[50], course_time[50], equipment[50], capacity_text[50];
        
        line[strcspn(line, "\n")] = '\0';
        
        int fields = sscanf(line, "%49s %49s %49s %49s %49s %49s %49s", 
                           current_course_id, course_name, course_type, 
                           course_center, course_time, equipment, capacity_text);
        
        if (fields == 7) {
            gchar *clean_current_id = g_strdup(current_course_id);
            gchar *clean_search_id = g_strdup(course_id);
            
            if (clean_current_id && clean_search_id) {
                g_strstrip(clean_current_id);
                g_strstrip(clean_search_id);
                
                if (strcmp(clean_current_id, clean_search_id) == 0) {
                    int current_capacity = atoi(capacity_text);
                    int new_capacity = current_capacity + change_amount;
                    
                    if (new_capacity < 0) {
                        new_capacity = 0;
                    }
                    
                    snprintf(lines[line_count], sizeof(lines[line_count]),
                            "%s %s %s %s %s %s %d",
                            current_course_id, course_name, course_type,
                            course_center, course_time, equipment, new_capacity);
                    
                    printf("DEBUG: Updated capacity for course %s: %d -> %d\n",
                           current_course_id, current_capacity, new_capacity);
                    
                    updated = 1;
                } else {
                    strncpy(lines[line_count], line, sizeof(lines[line_count])-1);
                }
                
                g_free(clean_current_id);
                g_free(clean_search_id);
            } else {
                strncpy(lines[line_count], line, sizeof(lines[line_count])-1);
            }
        } else {
            strncpy(lines[line_count], line, sizeof(lines[line_count])-1);
        }
        
        line_count++;
    }
    
    fclose(f);
    
    if (updated) {
        FILE *f_out = fopen("courses.txt", "w");
        if (!f_out) {
            printf("ERROR: Cannot open courses.txt for writing\n");
            return 0;
        }
        
        for (int i = 0; i < line_count; i++) {
            fprintf(f_out, "%s\n", lines[i]);
        }
        
        fclose(f_out);
        printf("DEBUG: Successfully updated courses.txt\n");
        return 1;
    }
    
    return 0;
}

// =========================================================
// Compter les coachs assignés à un cours - CORRIGÉ
// =========================================================
int count_assigned_coaches(const char *course_id) {
    if (!course_id || strlen(course_id) == 0) {
        return 0;
    }
    
    FILE *f = fopen("coach_assignment.txt", "r");
    if (!f) {
        printf("DEBUG: coach_assignment.txt not found or empty\n");
        return 0;
    }
    
    int count = 0;
    char line[256];
    
    gchar *clean_search_id = g_strdup(course_id);
    if (clean_search_id) {
        g_strstrip(clean_search_id);
        printf("DEBUG: Searching for course ID: '%s'\n", clean_search_id);
        
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = '\0';
            
            // Parser la ligne selon le nouveau format
            // Format: coach_id|coach_name|coach_phone|specialty|day|month|year|course_id|course_name|course_center|course_time
            int parsed_coach_id, parsed_day, parsed_month, parsed_year;
            char parsed_coach_name[100], parsed_coach_phone[20], parsed_specialty[30];
            char parsed_course_id[10], parsed_course_name[50], parsed_course_center[30], parsed_course_time[20];
            
            int parsed_fields = sscanf(line, "%d|%99[^|]|%19[^|]|%29[^|]|%d|%d|%d|%9[^|]|%49[^|]|%29[^|]|%19[^|]",
                                      &parsed_coach_id, parsed_coach_name, parsed_coach_phone, parsed_specialty,
                                      &parsed_day, &parsed_month, &parsed_year,
                                      parsed_course_id, parsed_course_name, parsed_course_center, parsed_course_time);
            
            if (parsed_fields == 11) {
                if (strcmp(parsed_course_id, clean_search_id) == 0) {
                    count++;
                    printf("DEBUG: Found assignment #%d for course %s\n", count, parsed_course_id);
                }
            } else {
                // Essayer un parsing simple
                char *token;
                char *rest = line;
                int field = 0;
                
                while ((token = strtok_r(rest, "|", &rest))) {
                    field++;
                    if (field == 8) { // 8ème champ = course_id
                        if (strcmp(token, clean_search_id) == 0) {
                            count++;
                        }
                        break;
                    }
                }
            }
        }
        
        g_free(clean_search_id);
    }
    
    fclose(f);
    printf("DEBUG: Total assignments for course %s: %d\n", course_id, count);
    return count;
}

// =========================================================
// ADD COACH - AVEC CONTRÔLE MAX 8 CHIFFRES
// =========================================================
void on_btadd_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_btadd_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    if (!win) {
        printf("ERROR: Could not get toplevel window\n");
        show_popup_message(win, "Error", "Could not get toplevel window", GTK_MESSAGE_ERROR);
        return;
    }
    
    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *spin_day = lookup_widget(win, "spinbuttonday");
    GtkWidget *spin_month = lookup_widget(win, "spinbuttonmonth");
    GtkWidget *spin_year = lookup_widget(win, "spinbuttonyear");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    GtkWidget *combo_specialty = lookup_widget(win, "comboboxentry_spec");

    if (!entryid || !entryname || !entryfirst || !entryphone || !spin_day || 
        !spin_month || !spin_year || !combo_center || !combo_specialty) {
        printf("ERROR: Some widgets are NULL\n");
        show_popup_message(win, "Error", "Some widgets are missing", GTK_MESSAGE_ERROR);
        return;
    }
    
    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(entryname));
    const gchar *fname = gtk_entry_get_text(GTK_ENTRY(entryfirst));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entryphone));
    
    printf("DEBUG: Input values - ID: '%s', Name: '%s', First: '%s', Phone: '%s'\n", 
           id, name, fname, phone);
    
    // Validation avec contrôle max 8 chiffres
    if (!id || !is_valid_id(id)) { 
        show_popup_message(win, "Validation Error", "ID must be numbers only (max 8 digits)", GTK_MESSAGE_WARNING);
        return; 
    }
    if (!name || !is_letters(name)) { 
        show_popup_message(win, "Validation Error", "Name must be letters only", GTK_MESSAGE_WARNING);
        return; 
    }
    if (!fname || !is_letters(fname)) { 
        show_popup_message(win, "Validation Error", "First name must be letters only", GTK_MESSAGE_WARNING);
        return; 
    }
    if (!phone || !is_valid_phone(phone)) { 
        show_popup_message(win, "Validation Error", "Phone must be exactly 8 numbers (ex: 12345678)", GTK_MESSAGE_WARNING);
        return; 
    }
    
    // Check if ID already exists
    FILE *f = fopen("coach.txt", "r");
    if (f) {
        int existing_id;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            sscanf(buffer, "%d", &existing_id);
            if (existing_id == atoi(id)) {
                fclose(f);
                show_popup_message(win, "Error", "ID already exists!", GTK_MESSAGE_ERROR);
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
    
    // Centre
    GtkEntry *entry_center = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *center_text = gtk_entry_get_text(entry_center);
    if (center_text == NULL || strlen(center_text) == 0) {
        show_popup_message(win, "Error", "Please select a center!", GTK_MESSAGE_WARNING);
        return;
    }
    strncpy(c.center, center_text, sizeof(c.center)-1);
    c.center[sizeof(c.center)-1] = '\0';
    
    // Spécialité
    GtkEntry *entry_specialty = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_specialty)));
    const gchar *specialty_text = gtk_entry_get_text(entry_specialty);
    if (specialty_text == NULL || strlen(specialty_text) == 0) {
        show_popup_message(win, "Error", "Please select a specialty!", GTK_MESSAGE_WARNING);
        return;
    }
    strncpy(c.specialty, specialty_text, sizeof(c.specialty)-1);
    c.specialty[sizeof(c.specialty)-1] = '\0';
    
    c.gender = gender;
    
    printf("DEBUG: Attempting to add coach - ID: %d, Name: %s %s, Specialty: %s\n", 
           c.id, c.firstName, c.lastName, c.specialty);
    
    if (addCoach("coach.txt", c)) {
        show_popup_message(win, "Success", "Coach added successfully!", GTK_MESSAGE_INFO);
        printf("SUCCESS: Coach added to database\n");
        
        // Debug le fichier
        debug_coach_file();
        
        // Email
        char email_body[512];
        snprintf(email_body, sizeof(email_body),
                 "New Coach Information:\n"
                 "• ID: %d\n"
                 "• Name: %s %s\n"
                 "• Center: %s\n"
                 "• Phone: %s\n"
                 "• Gender: %s\n"
                 "• Specialty: %s\n"
                 "• Date of Birth: %d/%d/%d",
                 c.id, c.firstName, c.lastName,
                 c.center, c.phoneNumber,
                 (c.gender == 1) ? "Male" : "Female",
                 c.specialty,
                 c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year);
        
        send_email_to_admin("New Coach Added", email_body, "add");
        
    } else {
        show_popup_message(win, "Error", "Error adding coach!", GTK_MESSAGE_ERROR);
        printf("ERROR: Failed to add coach\n");
    }

    // Clear fields
    gtk_entry_set_text(GTK_ENTRY(entryid), "");
    gtk_entry_set_text(GTK_ENTRY(entryname), "");
    gtk_entry_set_text(GTK_ENTRY(entryfirst), "");
    gtk_entry_set_text(GTK_ENTRY(entryphone), "");
    gtk_entry_set_text(entry_center, "");
    gtk_entry_set_text(entry_specialty, "");

    // Refresh tree view
    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) {
        refresh_tree(tree);
    }
    
    printf("=== DEBUG on_btadd_clicked END ===\n\n");
}

// =========================================================
// MODIFY COACH - AVEC CONFIRMATION YES/NO ET CONTRÔLE MAX 8 CHIFFRES
// =========================================================
void on_btmodify_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_btmodify_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *spin_day = lookup_widget(win, "spinbuttonday");
    GtkWidget *spin_month = lookup_widget(win, "spinbuttonmonth");
    GtkWidget *spin_year = lookup_widget(win, "spinbuttonyear");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    GtkWidget *combo_specialty = lookup_widget(win, "comboboxentry_spec");

    if (!entryid || !entryname || !entryfirst || !entryphone || !spin_day || 
        !spin_month || !spin_year || !combo_center || !combo_specialty) {
        show_popup_message(win, "Error", "Some widgets are missing", GTK_MESSAGE_ERROR);
        return;
    }

    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
    if (!is_valid_id(id)) { 
        show_popup_message(win, "Validation Error", "ID must be numbers only (max 8 digits)", GTK_MESSAGE_WARNING);
        return; 
    }

    // Check if coach exists before showing confirmation
    int coach_id = atoi(id);
    int coach_exists = 0;
    FILE *check_file = fopen("coach.txt", "r");
    if (check_file) {
        int temp_id;
        char line[256];
        while (fgets(line, sizeof(line), check_file)) {
            if (sscanf(line, "%d", &temp_id) == 1 && temp_id == coach_id) {
                coach_exists = 1;
                break;
            }
        }
        fclose(check_file);
    }
    
    if (!coach_exists) {
        show_popup_message(win, "Error", "Coach ID not found!", GTK_MESSAGE_ERROR);
        return;
    }

    // Get old coach info before modification for email
    Coach old_coach;
    int found_old = 0;
    FILE *f = fopen("coach.txt", "r");
    if (f) {
        Coach temp;
        char gender_str[10];
        char specialty_str[30];
        while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                      &temp.id, temp.lastName, temp.firstName,
                      &temp.dateOfBirth.day, &temp.dateOfBirth.month, &temp.dateOfBirth.year,
                      temp.center, temp.phoneNumber, gender_str, specialty_str) == 10) {
            if (temp.id == coach_id) {
                old_coach = temp;
                old_coach.gender = (strcmp(gender_str, "Male") == 0) ? 1 : 2;
                strncpy(old_coach.specialty, specialty_str, sizeof(old_coach.specialty)-1);
                old_coach.specialty[sizeof(old_coach.specialty)-1] = '\0';
                found_old = 1;
                break;
            }
        }
        fclose(f);
    }

    Coach c;
    c.id = atoi(id);

    strncpy(c.lastName, gtk_entry_get_text(GTK_ENTRY(entryname)), sizeof(c.lastName)-1);
    c.lastName[sizeof(c.lastName)-1] = '\0';
    strncpy(c.firstName, gtk_entry_get_text(GTK_ENTRY(entryfirst)), sizeof(c.firstName)-1);
    c.firstName[sizeof(c.firstName)-1] = '\0';
    
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entryphone));
    if (!is_valid_phone(phone)) { 
        show_popup_message(win, "Validation Error", "Phone must be exactly 8 numbers (ex: 12345678)", GTK_MESSAGE_WARNING);
        return; 
    }
    strncpy(c.phoneNumber, phone, sizeof(c.phoneNumber)-1);
    c.phoneNumber[sizeof(c.phoneNumber)-1] = '\0';

    c.dateOfBirth.day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
    c.dateOfBirth.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
    c.dateOfBirth.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_year));

    // Centre
    GtkEntry *entry_center = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
    const gchar *center_text = gtk_entry_get_text(entry_center);
    if (center_text == NULL || strlen(center_text) == 0) {
        show_popup_message(win, "Error", "Please select a center!", GTK_MESSAGE_WARNING);
        return;
    }
    strncpy(c.center, center_text, sizeof(c.center)-1);
    c.center[sizeof(c.center)-1] = '\0';
    
    // Spécialité
    GtkEntry *entry_specialty = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_specialty)));
    const gchar *specialty_text = gtk_entry_get_text(entry_specialty);
    if (specialty_text == NULL || strlen(specialty_text) == 0) {
        show_popup_message(win, "Error", "Please select a specialty!", GTK_MESSAGE_WARNING);
        return;
    }
    strncpy(c.specialty, specialty_text, sizeof(c.specialty)-1);
    c.specialty[sizeof(c.specialty)-1] = '\0';

    c.gender = gender;

    // Show confirmation dialog
    gchar *confirmation_message = g_strdup_printf(
        "Are you sure you want to modify coach ID %d?\n\n"
        "New Information:\n"
        "• Name: %s %s\n"
        "• Center: %s\n"
        "• Phone: %s\n"
        "• Specialty: %s",
        c.id, c.firstName, c.lastName,
        c.center, c.phoneNumber, c.specialty);
    
    gboolean confirmed = show_confirmation_dialog(win, "Confirm Modification", confirmation_message);
    g_free(confirmation_message);
    
    if (!confirmed) {
        printf("DEBUG: Modification cancelled by user\n");
        return;
    }

    printf("DEBUG: Attempting to modify coach ID %d, Specialty: %s\n", c.id, c.specialty);
    
    if (modifyCoach("coach.txt", c.id, c)) {
        show_popup_message(win, "Success", "Coach modified successfully!", GTK_MESSAGE_INFO);
        printf("SUCCESS: Coach modified\n");
        
        debug_coach_file();
        
        // Email
        char email_body[768];
        if (found_old) {
            snprintf(email_body, sizeof(email_body),
                     "Coach Modified - Changes:\n\n"
                     "BEFORE MODIFICATION:\n"
                     "• ID: %d\n"
                     "• Name: %s %s\n"
                     "• Center: %s\n"
                     "• Phone: %s\n"
                     "• Gender: %s\n"
                     "• Specialty: %s\n"
                     "• Date of Birth: %d/%d/%d\n\n"
                     "AFTER MODIFICATION:\n"
                     "• ID: %d\n"
                     "• Name: %s %s\n"
                     "• Center: %s\n"
                     "• Phone: %s\n"
                     "• Gender: %s\n"
                     "• Specialty: %s\n"
                     "• Date of Birth: %d/%d/%d",
                     old_coach.id, old_coach.firstName, old_coach.lastName,
                     old_coach.center, old_coach.phoneNumber,
                     (old_coach.gender == 1) ? "Male" : "Female",
                     old_coach.specialty,
                     old_coach.dateOfBirth.day, old_coach.dateOfBirth.month, old_coach.dateOfBirth.year,
                     c.id, c.firstName, c.lastName,
                     c.center, c.phoneNumber,
                     (c.gender == 1) ? "Male" : "Female",
                     c.specialty,
                     c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year);
        } else {
            snprintf(email_body, sizeof(email_body),
                     "Coach Modified (old info not found):\n"
                     "• ID: %d\n"
                     "• Name: %s %s\n"
                     "• Center: %s\n"
                     "• Phone: %s\n"
                     "• Gender: %s\n"
                     "• Specialty: %s\n"
                     "• Date of Birth: %d/%d/%d",
                     c.id, c.firstName, c.lastName,
                     c.center, c.phoneNumber,
                     (c.gender == 1) ? "Male" : "Female",
                     c.specialty,
                     c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year);
        }
        
        send_email_to_admin("Coach Modified", email_body, "modify");
        
    } else {
        show_popup_message(win, "Error", "Error! ID not found!", GTK_MESSAGE_ERROR);
        printf("ERROR: Coach not found\n");
    }

    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
    
    printf("=== DEBUG on_btmodify_clicked END ===\n\n");
}

// =========================================================
// DELETE COACH - AVEC CONFIRMATION YES/NO ET SUPPORT CHECKBOX
// =========================================================
void on_btdelete_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_btdelete_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *treeview = lookup_widget(win, "treeviewmanage");

    if (!treeview) {
        show_popup_message(win, "Error", "Treeview widget not found", GTK_MESSAGE_ERROR);
        return;
    }

    // Vérifier d'abord les checkbox sélectionnées
    GList *selected_ids = get_selected_coach_ids(treeview);
    
    // Si aucune checkbox n'est cochée, vérifier le champ entryid
    if (selected_ids == NULL) {
        const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryid));
        if (id && strlen(id) > 0 && is_valid_id(id)) {
            selected_ids = g_list_append(NULL, GINT_TO_POINTER(atoi(id)));
        }
    }
    
    if (selected_ids == NULL) {
        show_popup_message(win, "Warning", 
            "Please select coaches to delete!\n"
            "You can either:\n"
            "1. Check coaches in the list\n"
            "2. Enter coach ID in the field", 
            GTK_MESSAGE_WARNING);
        return;
    }
    
    int count = g_list_length(selected_ids);
    gchar *confirmation_message = g_strdup_printf(
        "Are you sure you want to delete %d selected coach(es)?\n"
        "This action cannot be undone!", count);
    
    gboolean confirmed = show_confirmation_dialog(win, "Confirm Deletion", confirmation_message);
    g_free(confirmation_message);
    
    if (!confirmed) {
        printf("DEBUG: Deletion cancelled by user\n");
        g_list_free(selected_ids);
        return;
    }
    
    int deleted_count = 0;
    GList *iter = selected_ids;
    
    while (iter != NULL) {
        int coach_id = GPOINTER_TO_INT(iter->data);
        
        // Récupérer les infos du coach avant suppression pour l'email
        Coach deleted_coach;
        int found_coach = 0;
        FILE *f = fopen("coach.txt", "r");
        if (f) {
            Coach temp;
            char gender_str[10];
            char specialty_str[30];
            while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                          &temp.id, temp.lastName, temp.firstName,
                          &temp.dateOfBirth.day, &temp.dateOfBirth.month, &temp.dateOfBirth.year,
                          temp.center, temp.phoneNumber, gender_str, specialty_str) == 10) {
                if (temp.id == coach_id) {
                    deleted_coach = temp;
                    deleted_coach.gender = (strcmp(gender_str, "Male") == 0) ? 1 : 2;
                    strncpy(deleted_coach.specialty, specialty_str, sizeof(deleted_coach.specialty)-1);
                    deleted_coach.specialty[sizeof(deleted_coach.specialty)-1] = '\0';
                    found_coach = 1;
                    break;
                }
            }
            fclose(f);
        }
        
        if (deleteCoach("coach.txt", coach_id)) {
            deleted_count++;
            
            // Envoyer email pour chaque coach supprimé
            if (found_coach) {
                char email_body[512];
                snprintf(email_body, sizeof(email_body),
                         "Coach Deleted - Removed Information:\n"
                         "• ID: %d\n"
                         "• Name: %s %s\n"
                         "• Center: %s\n"
                         "• Phone: %s\n"
                         "• Gender: %s\n"
                         "• Specialty: %s\n"
                         "• Date of Birth: %d/%d/%d\n\n"
                         "This coach has been permanently removed from the system.",
                         deleted_coach.id, deleted_coach.firstName, deleted_coach.lastName,
                         deleted_coach.center, deleted_coach.phoneNumber,
                         (deleted_coach.gender == 1) ? "Male" : "Female",
                         deleted_coach.specialty,
                         deleted_coach.dateOfBirth.day, deleted_coach.dateOfBirth.month, deleted_coach.dateOfBirth.year);
                
                send_email_to_admin("Coach Deleted", email_body, "delete");
            }
        }
        
        iter = iter->next;
    }
    
    char result_msg[256];
    if (deleted_count > 0) {
        snprintf(result_msg, sizeof(result_msg), 
                "Successfully deleted %d out of %d coach(es)!", 
                deleted_count, count);
        show_popup_message(win, "Success", result_msg, GTK_MESSAGE_INFO);
        
        // Recharger la TreeView
        refresh_tree(treeview);
    } else {
        show_popup_message(win, "Error", "No coaches were deleted.", GTK_MESSAGE_ERROR);
    }
    
    g_list_free(selected_ids);
    
    // Clear fields
    gtk_entry_set_text(GTK_ENTRY(entryid), "");
    GtkWidget *entryname = lookup_widget(win, "entryname2");
    GtkWidget *entryfirst = lookup_widget(win, "entryfirstname");
    GtkWidget *entryphone = lookup_widget(win, "entryphonenumber");
    GtkWidget *combo_center = lookup_widget(win, "entrycentre2");
    GtkWidget *combo_specialty = lookup_widget(win, "comboboxentry_spec");
    
    if (entryname) gtk_entry_set_text(GTK_ENTRY(entryname), "");
    if (entryfirst) gtk_entry_set_text(GTK_ENTRY(entryfirst), "");
    if (entryphone) gtk_entry_set_text(GTK_ENTRY(entryphone), "");
    if (combo_center) {
        GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
        if (entry) gtk_entry_set_text(entry, "");
    }
    if (combo_specialty) {
        GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_specialty)));
        if (entry) gtk_entry_set_text(entry, "");
    }
    
    printf("=== DEBUG on_btdelete_clicked END ===\n\n");
}

// =========================================================
// CHECKBUTTON NAME TOGGLED
// =========================================================
void on_checkbutton_name_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    printf("DEBUG: Name filter %s\n", active ? "ACTIVATED" : "DEACTIVATED");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *entry_name = lookup_widget(win, "entryname1");
    
    if (entry_name) {
        gtk_widget_set_sensitive(entry_name, active);
    }
}

// =========================================================
// SEARCH COACH - MODIFIÉ pour afficher les résultats dans la TreeView
// =========================================================
void on_btsearch_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_btsearch_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entryname = lookup_widget(win, "entryname1");
    GtkWidget *combo = lookup_widget(win, "entrycentre1");
    GtkWidget *checkbutton_name = lookup_widget(win, "checkbutton_name");
    GtkWidget *checkbutton_center = lookup_widget(win, "checkbutton_center");
    GtkWidget *treeviewmanage = lookup_widget(win, "treeviewmanage");

    if (!entryname || !combo || !checkbutton_name || !checkbutton_center || !treeviewmanage) {
        printf("ERROR: Some widgets not found!\n");
        show_popup_message(win, "Error", "Some widgets are missing", GTK_MESSAGE_ERROR);
        return;
    }

    gboolean search_by_name = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_name));
    gboolean search_by_center = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_center));
    
    printf("DEBUG: Search filters - Name: %d, Center: %d\n", search_by_name, search_by_center);

    const gchar *name = NULL;
    const gchar *center = NULL;
    
    gchar *clean_name = NULL;
    gchar *clean_center = NULL;
    
    if (search_by_name) {
        name = gtk_entry_get_text(GTK_ENTRY(entryname));
        if (name && strlen(name) > 0) {
            clean_name = g_strdup(name);
            g_strstrip(clean_name);
            printf("DEBUG: Searching by name: '%s'\n", clean_name);
        } else {
            printf("DEBUG: Name filter active but name is empty\n");
            search_by_name = FALSE;
        }
    }
    
    if (search_by_center) {
        GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));
        center = gtk_entry_get_text(entry);
        if (center && strlen(center) > 0) {
            clean_center = g_strdup(center);
            g_strstrip(clean_center);
            printf("DEBUG: Searching by center: '%s'\n", clean_center);
        } else {
            printf("DEBUG: Center filter active but center is empty\n");
            search_by_center = FALSE;
        }
    }
    
    if (!search_by_name && !search_by_center) {
        // Si aucun filtre n'est activé, recharger tous les coachs
        refresh_tree(treeviewmanage);
        show_popup_message(win, "Info", "Showing all coaches", GTK_MESSAGE_INFO);
        
        if (clean_name) g_free(clean_name);
        if (clean_center) g_free(clean_center);
        return;
    }
    
    // Récupérer les valeurs de recherche
    const char *search_name = (search_by_name && clean_name && strlen(clean_name) > 0) ? clean_name : NULL;
    const char *search_center = (search_by_center && clean_center && strlen(clean_center) > 0) ? clean_center : NULL;
    
    // Ouvrir le fichier coach.txt et filtrer les résultats
    FILE *f = fopen("coach.txt", "r");
    if (!f) {
        printf("ERROR: Cannot open coach.txt\n");
        show_popup_message(win, "Error", "Cannot open coach database file", GTK_MESSAGE_ERROR);
        
        if (clean_name) g_free(clean_name);
        if (clean_center) g_free(clean_center);
        return;
    }
    
    // Créer le store avec checkbox
    GtkListStore *store = gtk_list_store_new(11,
                               G_TYPE_BOOLEAN,
                               G_TYPE_INT,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_INT,
                               G_TYPE_INT,
                               G_TYPE_INT,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING);
    
    GtkTreeIter iter;
    int found_count = 0;
    char line[256];
    
    // Lire tous les coachs et appliquer les filtres
    while (fgets(line, sizeof(line), f)) {
        Coach c;
        char gender_str[10];
        char specialty[30];
        
        int fields = sscanf(line, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                          &c.id, c.lastName, c.firstName,
                          &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                          c.center, c.phoneNumber, gender_str, specialty);
        
        if (fields == 10) {
            c.gender = (strcmp(gender_str, "Male") == 0) ? 1 : 2;
            strncpy(c.specialty, specialty, sizeof(c.specialty)-1);
            c.specialty[sizeof(c.specialty)-1] = '\0';
            
            // Appliquer les filtres
            gboolean match = TRUE;
            
            // Filtre par nom
            if (search_by_name && search_name) {
                char full_name[100];
                snprintf(full_name, sizeof(full_name), "%s %s", c.firstName, c.lastName);
                gchar *lower_full_name = g_ascii_strdown(full_name, -1);
                gchar *lower_search = g_ascii_strdown(search_name, -1);
                
                if (strstr(lower_full_name, lower_search) == NULL) {
                    match = FALSE;
                }
                
                g_free(lower_full_name);
                g_free(lower_search);
            }
            
            // Filtre par centre
            if (match && search_by_center && search_center) {
                gchar *lower_center = g_ascii_strdown(c.center, -1);
                gchar *lower_search_center = g_ascii_strdown(search_center, -1);
                
                if (strcmp(lower_center, lower_search_center) != 0) {
                    match = FALSE;
                }
                
                g_free(lower_center);
                g_free(lower_search_center);
            }
            
            // Si le coach correspond aux critères, l'ajouter à la TreeView
            if (match) {
                found_count++;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                   0, FALSE,           // Checkbox non cochée par défaut
                                   1, c.id,
                                   2, c.lastName,
                                   3, c.firstName,
                                   4, c.dateOfBirth.day,
                                   5, c.dateOfBirth.month,
                                   6, c.dateOfBirth.year,
                                   7, c.center,
                                   8, c.phoneNumber,
                                   9, gender_str,
                                   10, c.specialty,
                                   -1);
            }
        }
    }
    
    fclose(f);
    
    // Si aucun résultat n'est trouvé
    if (found_count == 0) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, FALSE,
                           1, -1,
                           2, "No coaches found",
                           3, "",
                           4, 0,
                           5, 0,
                           6, 0,
                           7, "",
                           8, "",
                           9, "",
                           10, "",
                           -1);
        
        gchar *message = g_strdup_printf("No coaches found with the specified criteria.");
        show_popup_message(win, "No Results", message, GTK_MESSAGE_INFO);
        g_free(message);
    } else {
        gchar *message = g_strdup_printf("Found %d coach(es) matching your criteria.", found_count);
        show_popup_message(win, "Search Results", message, GTK_MESSAGE_INFO);
        g_free(message);
    }
    
    // Mettre à jour la TreeView avec les résultats filtrés
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeviewmanage), GTK_TREE_MODEL(store));
    
    // Créer les colonnes avec checkbox et édition pour les résultats de recherche
    const char *titles[11] = {"Select", "ID", "Last Name", "First Name", "Day", "Month", "Year", 
                              "Center", "Phone", "Gender", "Specialty"};
    create_columns_with_checkbox_coach(GTK_TREE_VIEW(treeviewmanage), titles, 11);
    
    // NE PAS unref le store ici
    // g_object_unref(store);
    
    // Nettoyer la mémoire
    if (clean_name) g_free(clean_name);
    if (clean_center) g_free(clean_center);
    
    printf("DEBUG: Search complete. Found %d coach(es)\n", found_count);
    printf("=== DEBUG on_btsearch_clicked END ===\n\n");
}

// =========================================================
// RADIO BUTTONS
// =========================================================
void on_radiobuttonman_toggled(GtkToggleButton *t, gpointer data) {
    if (gtk_toggle_button_get_active(t)) gender = 1;
}

void on_radiobuttonwomen_toggled(GtkToggleButton *t, gpointer data) {
    if (gtk_toggle_button_get_active(t)) gender = 2;
}

// =========================================================
// ROW ACTIVATED - MODIFIÉ POUR LA SPÉCIALITÉ ET CHECKBOX
// =========================================================
void on_treeviewmanage_row_activated(GtkTreeView *treeview,
                                     GtkTreePath *path,
                                     GtkTreeViewColumn *column,
                                     gpointer userdata) {
    printf("\n=== DEBUG on_treeviewmanage_row_activated START ===\n");
    
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    
    if (!model) {
        printf("ERROR: Tree model is NULL\n");
        return;
    }

    gboolean checkbox_state;
    int id, day, month, year;
    gchar *lname = NULL;
    gchar *fname = NULL;
    gchar *center = NULL;
    gchar *phone = NULL;
    gchar *gender_str = NULL;
    gchar *specialty = NULL;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter,
                           0, &checkbox_state,
                           1, &id,
                           2, &lname,
                           3, &fname,
                           4, &day,
                           5, &month,
                           6, &year,
                           7, &center,
                           8, &phone,
                           9, &gender_str,
                           10, &specialty,
                           -1);

        printf("DEBUG: Row activated - ID: %d, Name: %s %s, Specialty: %s, Checkbox: %s\n", 
               id, fname ? fname : "NULL", lname ? lname : "NULL", 
               specialty ? specialty : "NULL", checkbox_state ? "TRUE" : "FALSE");

        // Inverser l'état de la checkbox
        checkbox_state = !checkbox_state;
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, checkbox_state, -1);

        GtkWidget *window = lookup_widget(GTK_WIDGET(treeview), "raed_manage_coach");

        if (!window) {
            printf("ERROR: Window not found\n");
            if (lname) g_free(lname);
            if (fname) g_free(fname);
            if (center) g_free(center);
            if (phone) g_free(phone);
            if (gender_str) g_free(gender_str);
            if (specialty) g_free(specialty);
            return;
        }

        // Ne remplir les champs que si le coach existe (ID > 0)
        if (id > 0) {
            GtkWidget *entryid = lookup_widget(window, "entryid");
            GtkWidget *entryname = lookup_widget(window, "entryname2");
            GtkWidget *entryfirst = lookup_widget(window, "entryfirstname");
            GtkWidget *entryphone = lookup_widget(window, "entryphonenumber");
            GtkWidget *spin_day = lookup_widget(window, "spinbuttonday");
            GtkWidget *spin_month = lookup_widget(window, "spinbuttonmonth");
            GtkWidget *spin_year = lookup_widget(window, "spinbuttonyear");
            GtkWidget *combo_center = lookup_widget(window, "entrycentre2");
            GtkWidget *combo_specialty = lookup_widget(window, "comboboxentry_spec");
            GtkWidget *radiobuttonman = lookup_widget(window, "radiobuttonman");
            GtkWidget *radiobuttonwomen = lookup_widget(window, "radiobuttonwomen");

            // Remplir les champs
            if (entryid) {
                gchar *id_str = g_strdup_printf("%d", id);
                gtk_entry_set_text(GTK_ENTRY(entryid), id_str);
                g_free(id_str);
            }
            
            if (entryname && lname) {
                gtk_entry_set_text(GTK_ENTRY(entryname), lname ? lname : "");
            }
            
            if (entryfirst && fname) {
                gtk_entry_set_text(GTK_ENTRY(entryfirst), fname ? fname : "");
            }
            
            if (entryphone && phone) {
                gtk_entry_set_text(GTK_ENTRY(entryphone), phone ? phone : "");
            }
            
            if (spin_day) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_day), day);
            if (spin_month) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_month), month);
            if (spin_year) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_year), year);
            
            if (combo_center && center) {
                GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_center)));
                if (entry) {
                    gtk_entry_set_text(entry, center ? center : "");
                }
            }
            
            if (combo_specialty && specialty) {
                GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_specialty)));
                if (entry) {
                    gtk_entry_set_text(entry, specialty ? specialty : "");
                }
            }
            
            if (gender_str) {
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
        
        if (lname) g_free(lname);
        if (fname) g_free(fname);
        if (center) g_free(center);
        if (phone) g_free(phone);
        if (gender_str) g_free(gender_str);
        if (specialty) g_free(specialty);
    }
    
    printf("=== DEBUG on_treeviewmanage_row_activated END ===\n");
}

// =========================================================
// REFRESH BUTTON
// =========================================================
void on_refresh_clicked(GtkButton *button, gpointer data) {
    printf("\n=== DEBUG on_refresh_clicked ===\n");
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tree = lookup_widget(win, "treeviewmanage");
    if (tree) refresh_tree(tree);
    
    show_popup_message(win, "Refresh", "Coach list refreshed successfully!", GTK_MESSAGE_INFO);
}

// =========================================================
// COURSE TREEVIEW ROW ACTIVATED (7 COLONNES SANS CHECKBOX ET SANS ÉDITION)
// =========================================================
void on_treeview_2_row_activated(GtkTreeView *treeview,
                                 GtkTreePath *path,
                                 GtkTreeViewColumn *column,
                                 gpointer user_data) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    
    if (!model) return;

    char course_id[50], course_name[50], course_type[50];
    char course_center[50], course_time[50], equipment[50], capacity[50];

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter,
                           0, course_id,
                           1, course_name,
                           2, course_type,
                           3, course_center,
                           4, course_time,
                           5, equipment,
                           6, capacity,
                           -1);

        printf("DEBUG: Course selected: %s - %s at %s (Equipment: %s, Capacity: %s)\n", 
               course_id, course_name, course_time, equipment, capacity);
    }
}

// =========================================================
// refresh_2 CLICKED (Load Courses) - AVEC POP-UP
// =========================================================
void on_refresh_2_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_refresh_2_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    GtkWidget *treeview_2 = lookup_widget(win, "treeview_2");
    if (!treeview_2) {
        printf("Error: treeview_2 not found!\n");
        show_popup_message(win, "Error", "Treeview widget not found!", GTK_MESSAGE_ERROR);
        return;
    }
    
    loadCourses(GTK_TREE_VIEW(treeview_2), "courses.txt");
    
    show_popup_message(win, "Success", "Courses loaded successfully!", GTK_MESSAGE_INFO);
    
    printf("=== DEBUG on_refresh_2_clicked END ===\n");
}

// =========================================================
// btchoose CLICKED (Choose/Assign Coach to Course) - VERSION SIMPLIFIÉE
// =========================================================
void on_btchoose_clicked(GtkButton *button, gpointer user_data) {
    printf("\n=== DEBUG on_btchoose_clicked START ===\n");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryid = lookup_widget(win, "entryid");
    GtkWidget *treeview_2 = lookup_widget(win, "treeview_2");
    GtkWidget *label201 = lookup_widget(win, "label201");

    if (!entryid || !treeview_2 || !label201) {
        printf("Error: Some widgets not found!\n");
        show_popup_message(win, "Error", "Some widgets are missing!", GTK_MESSAGE_ERROR);
        return;
    }

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entryid));

    if (!id_text || strlen(id_text) == 0 || !is_valid_id(id_text)) {
        show_popup_message(win, "Validation Error", "Invalid Coach ID! Must be numbers only (max 8 digits).", GTK_MESSAGE_WARNING);
        return;
    }

    int coach_id = atoi(id_text);
    
    printf("DEBUG: Checking if coach ID %d exists in coach.txt\n", coach_id);
    
    int coach_exists = 0;
    Coach existing_coach;
    FILE *coach_file = fopen("coach.txt", "r");
    if (coach_file) {
        Coach temp;
        char gender_str[10];
        char specialty[30];
        
        while (fscanf(coach_file, "%d %29s %29s %d %d %d %29s %19s %9s %29s",
                      &temp.id, temp.lastName, temp.firstName,
                      &temp.dateOfBirth.day, &temp.dateOfBirth.month, &temp.dateOfBirth.year,
                      temp.center, temp.phoneNumber, gender_str, specialty) == 10) {
            if (temp.id == coach_id) {
                existing_coach = temp;
                existing_coach.gender = (strcmp(gender_str, "Male") == 0) ? 1 : 2;
                strncpy(existing_coach.specialty, specialty, sizeof(existing_coach.specialty)-1);
                existing_coach.specialty[sizeof(existing_coach.specialty)-1] = '\0';
                coach_exists = 1;
                printf("DEBUG: Coach found: %s %s (Specialty: %s)\n", 
                       existing_coach.firstName, existing_coach.lastName, existing_coach.specialty);
                break;
            }
        }
        fclose(coach_file);
    } else {
        printf("ERROR: Cannot open coach.txt for reading\n");
    }
    
    if (!coach_exists) {
        gchar *error_msg = g_strdup_printf("Coach ID %d does not exist!", coach_id);
        show_popup_message(win, "Error", error_msg, GTK_MESSAGE_ERROR);
        g_free(error_msg);
        return;
    }
    
    // Vérifier si un cours est sélectionné
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_2));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_popup_message(win, "Warning", "Please select a course first!", GTK_MESSAGE_WARNING);
        return;
    }

    // Récupérer les informations du cours sélectionné
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

    printf("DEBUG: Selected course - ID: '%s', Name: '%s', Capacity: '%s'\n", 
           course_id ? course_id : "NULL", course_name ? course_name : "NULL", capacity_text ? capacity_text : "NULL");

    if (course_id && strcmp(course_id, "ERROR") == 0) {
        show_popup_message(win, "Error", "Please load courses first!", GTK_MESSAGE_ERROR);
        
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }

    if (!course_id || strlen(course_id) == 0 || 
        !course_name || strlen(course_name) == 0 || 
        !course_center || strlen(course_center) == 0 || 
        !course_time || strlen(course_time) == 0 ||
        !capacity_text || strlen(capacity_text) == 0) {
        show_popup_message(win, "Error", "Invalid course data selected!", GTK_MESSAGE_ERROR);
        
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }

    int capacity = atoi(capacity_text);
    if (capacity <= 0) {
        show_popup_message(win, "Error", "Invalid capacity value!", GTK_MESSAGE_ERROR);
        
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }
    
    // Vérifier si le cours est déjà plein
    int assigned_coaches = count_assigned_coaches(course_id);
    
    printf("DEBUG: Course %s - Capacity: %d, Currently assigned: %d\n", 
           course_id, capacity, assigned_coaches);
    
    if (assigned_coaches >= capacity) {
        gchar *message = g_strdup_printf("Course is FULL! Capacity: %d, Assigned: %d", capacity, assigned_coaches);
        show_popup_message(win, "Error", message, GTK_MESSAGE_ERROR);
        g_free(message);
        
        g_free(course_id);
        g_free(course_name);
        g_free(course_type);
        g_free(course_center);
        g_free(course_time);
        g_free(equipment);
        g_free(capacity_text);
        return;
    }
    
    int day_val = existing_coach.dateOfBirth.day;
    int month_val = existing_coach.dateOfBirth.month;
    int year_val = existing_coach.dateOfBirth.year;
    
    // Nettoyer les chaînes
    gchar *clean_course_id = g_strdup(course_id ? course_id : "");
    gchar *clean_course_name = g_strdup(course_name ? course_name : "");
    gchar *clean_course_center = g_strdup(course_center ? course_center : "");
    gchar *clean_course_time = g_strdup(course_time ? course_time : "");
    
    if (clean_course_id) g_strstrip(clean_course_id);
    if (clean_course_name) g_strstrip(clean_course_name);
    if (clean_course_center) g_strstrip(clean_course_center);
    if (clean_course_time) g_strstrip(clean_course_time);
    
    char full_name[100];
    snprintf(full_name, sizeof(full_name), "%s %s", existing_coach.firstName, existing_coach.lastName);
    
    printf("DEBUG: Creating assignment - Coach: %s, Course: %s, Specialty: %s\n", 
           full_name, clean_course_name, existing_coach.specialty);
    
    // 1. SUPPRIMER LE COACH DE coach.txt
    printf("DEBUG: Removing coach ID %d from coach.txt...\n", coach_id);
    
    FILE *original_file = fopen("coach.txt", "r");
    FILE *temp_file = fopen("coach_temp.txt", "w");
    
    int coach_removed = 0;
    
    if (original_file && temp_file) {
        char line[256];
        while (fgets(line, sizeof(line), original_file)) {
            int current_id;
            if (sscanf(line, "%d", &current_id) == 1) {
                if (current_id != coach_id) {
                    fputs(line, temp_file);
                } else {
                    coach_removed = 1;
                    printf("DEBUG: Coach ID %d removed from coach.txt\n", coach_id);
                }
            } else {
                fputs(line, temp_file);
            }
        }
        
        fclose(original_file);
        fclose(temp_file);
        
        if (coach_removed) {
            remove("coach.txt");
            rename("coach_temp.txt", "coach.txt");
            printf("DEBUG: Successfully updated coach.txt\n");
        } else {
            remove("coach_temp.txt");
            printf("DEBUG: Coach not found in coach.txt\n");
        }
    } else {
        printf("ERROR: Cannot open coach.txt or create temp file\n");
        if (original_file) fclose(original_file);
        if (temp_file) fclose(temp_file);
    }
    
    // 2. AJOUTER L'ASSIGNATION
    FILE *f = fopen("coach_assignment.txt", "a");
    if (!f) {
        show_popup_message(win, "Error", "Error creating assignment file!", GTK_MESSAGE_ERROR);
        
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

    fprintf(f, "%d|%s|%s|%s|%d|%d|%d|%s|%s|%s|%s\n",
            coach_id,
            full_name,
            existing_coach.phoneNumber,
            existing_coach.specialty,
            day_val,
            month_val,
            year_val,
            clean_course_id,
            clean_course_name,
            clean_course_center,
            clean_course_time);
    
    fclose(f);
    
    printf("DEBUG: Assignment saved to coach_assignment.txt\n");
    debug_assignment_file();
    
    // 3. METTRE À JOUR LA CAPACITÉ
    int update_result = update_course_capacity(clean_course_id, -1);
    
    if (update_result) {
        // Recharger les TreeViews
        refresh_course_tree(treeview_2);
        
        GtkWidget *treeview_coach = lookup_widget(win, "treeviewmanage");
        if (treeview_coach) {
            refresh_tree(treeview_coach);
        }
        
        int new_capacity = capacity - 1;
        int new_assigned = assigned_coaches + 1;
        int remaining = new_capacity - new_assigned;
        
        if (remaining < 0) remaining = 0;
        
        gchar *success_message = g_strdup_printf(
            "✅ Coach assigned successfully!\n\n"
            "Coach: %s\n"
            "Specialty: %s\n"
            "Course: %s\n"
            "Remaining slots: %d/%d",
            full_name, existing_coach.specialty,
            clean_course_name, remaining, new_capacity);
        
        show_popup_message(win, "Success", success_message, GTK_MESSAGE_INFO);
        g_free(success_message);
        
        // Email
        char email_body[768];
        snprintf(email_body, sizeof(email_body),
                 "ASSIGNMENT DETAILS:\n\n"
                 "COACH INFORMATION:\n"
                 "• Coach ID: %d\n"
                 "• Coach Name: %s\n"
                 "• Coach Phone: %s\n"
                 "• Coach Specialty: %s\n"
                 "• Center: %s\n"
                 "• Assignment Date: %d/%d/%d\n\n"
                 "COURSE INFORMATION:\n"
                 "• Course ID: %s\n"
                 "• Course Name: %s\n"
                 "• Course Type: %s\n"
                 "• Center: %s\n"
                 "• Time: %s\n"
                 "• Equipment: %s\n\n"
                 "CAPACITY INFORMATION:\n"
                 "• Course Capacity: %d\n"
                 "• Assigned Coaches: %d\n"
                 "• Remaining Slots: %d/%d",
                 coach_id, full_name, existing_coach.phoneNumber, existing_coach.specialty, existing_coach.center,
                 day_val, month_val, year_val,
                 clean_course_id, clean_course_name, course_type ? course_type : "N/A",
                 clean_course_center, clean_course_time, equipment ? equipment : "N/A",
                 new_capacity, new_assigned, remaining, new_capacity);
        
        char coach_email[100];
        snprintf(coach_email, sizeof(coach_email), "%s.coach@topgym.com", existing_coach.firstName);
        
        send_email_to_coach(coach_email, "Course Assignment Confirmation", email_body);
        
        printf("DEBUG: Course capacity updated: %d -> %d\n", capacity, new_capacity);
    } else {
        show_popup_message(win, "Warning", "Coach assigned but failed to update capacity!", GTK_MESSAGE_WARNING);
    }
    
    // Effacer le champ ID
    gtk_entry_set_text(GTK_ENTRY(entryid), "");
    
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
    
    printf("DEBUG: Assignment complete for coach %d to course %s\n", coach_id, clean_course_id);
    printf("=== DEBUG on_btchoose_clicked END ===\n\n");
}

// =========================================================
// CHECKBUTTON CENTER TOGGLED
// =========================================================
void on_checkbutton_center_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    printf("DEBUG: Center filter %s\n", active ? "ACTIVATED" : "DEACTIVATED");
    
    GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *combo_center = lookup_widget(win, "entrycentre1");
    
    if (combo_center) {
        gtk_widget_set_sensitive(combo_center, active);
    }
}

// =====================================================
//                   FONCTIONS POUR LES COURS
// =====================================================

// Fonction pour obtenir le texte d'un combo (compatibilité GTK2)
const char* get_combo_text(GtkWidget *combo) {
    if (!GTK_IS_COMBO(combo)) return "";

    GtkCombo *c = GTK_COMBO(combo);
    GtkWidget *entry = c->entry;

    if (!GTK_IS_ENTRY(entry)) return "";

    return gtk_entry_get_text(GTK_ENTRY(entry));
}

// Fonction pour envoyer un email pour les cours
int send_course_email_to_admin(const char *subject, const char *body, const char *action_type, course c) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "topgym3b2@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "geutzwptyybqiffa");
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<topgym3b2@gmail.com>");

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        recipients = curl_slist_append(recipients, "<yomnaferjani@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        char message[2048];
        const char *action_emoji = "📋";
        const char *action_title = "COURSE SYSTEM NOTIFICATION";

        char time_str[20];
        switch(c.time) {
            case 1: strcpy(time_str, "Morning"); break;
            case 2: strcpy(time_str, "Afternoon"); break;
            case 3: strcpy(time_str, "Night"); break;
            default: strcpy(time_str, "Unknown");
        }

        char equipment_str[30];
        switch(c.equipment) {
            case 1: strcpy(equipment_str, "Treadmill"); break;
            case 2: strcpy(equipment_str, "Bunch press"); break;
            case 3: strcpy(equipment_str, "Squat machine"); break;
            case 4: strcpy(equipment_str, "No equipment"); break;
            default: strcpy(equipment_str, "Unknown");
        }

        if (strcmp(action_type, "add") == 0) {
            action_emoji = "➕";
            action_title = "NEW COURSE ADDED";
        } else if (strcmp(action_type, "modify") == 0) {
            action_emoji = "✏️";
            action_title = "COURSE MODIFIED";
        } else if (strcmp(action_type, "delete") == 0) {
            action_emoji = "🗑️";
            action_title = "COURSE DELETED";
        }

        snprintf(message, sizeof(message),
                 "To: Administrator <yomnaferjani@gmail.com>\r\n"
                 "From: TopGym Course System <topgym3b2@gmail.com>\r\n"
                 "Subject: %s\r\n"
                 "Content-Type: text/plain; charset=UTF-8\r\n"
                 "\r\n"
                 "%s %s %s\r\n"
                 "══════════════════════════════════════════\r\n"
                 "\r\n"
                 "📋 COURSE DETAILS:\r\n"
                 "══════════════════════════════════════════\r\n"
                 "• Course ID: %s\r\n"
                 "• Course Name: %s\r\n"
                 "• Course Type: %s\r\n"
                 "• Center: %s\r\n"
                 "• Time Slot: %s\r\n"
                 "• Equipment: %s\r\n"
                 "• Capacity: %d members\r\n"
                 "\r\n"
                 "%s\r\n"
                 "\r\n"
                 "📍 This is an automated notification from the TopGym course management system.\r\n"
                 "📍 Course database has been updated accordingly.\r\n"
                 "\r\n"
                 "📊 System Status: Action completed successfully\r\n"
                 "✅ Action Type: %s\r\n"
                 "🕒 Timestamp: %s\r\n"
                 "\r\n"
                 "No further action required. This email is for your information only.\r\n"
                 "\r\n"
                 "Best regards,\r\n"
                 "🏋️ TopGym Course Management System 🏋️\r\n"
                 "\r\n"
                 ".\r\n",
                 subject, action_emoji, action_title, action_emoji,
                 c.id, c.name, c.type, c.center, time_str, equipment_str, c.capacity,
                 body, action_type, __DATE__);

        const char *email_data = message;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "COURSE EMAIL ERROR: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return (res == CURLE_OK);
}

int send_email_to_member(const char *member_email, const char *member_name,
                         const char *course_id, const char *course_name,
                         const char *course_center, const char *course_time,
                         const char *reservation_date) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "topgym3b2@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "geutzwptyybqiffa");
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<topgym3b2@gmail.com>");

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        char member_email_full[256];
        if (member_email && strlen(member_email) > 0) {
            snprintf(member_email_full, sizeof(member_email_full), "<%s>", member_email);
        } else {
            snprintf(member_email_full, sizeof(member_email_full), "<member@topgym.com>");
        }

        recipients = curl_slist_append(recipients, member_email_full);
        recipients = curl_slist_append(recipients, "<yomnaferjani@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        char message[2048];
        char subject[256];
        snprintf(subject, sizeof(subject), "✅ Course Reservation Confirmation - %s", course_name);

        int conf_id = rand() % 900000 + 100000;

        snprintf(message, sizeof(message),
                 "To: %s <%s>\r\n"
                 "Cc: TopGym Admin <yomnaferjani@gmail.com>\r\n"
                 "From: TopGym Reservations <topgym3b2@gmail.com>\r\n"
                 "Subject: %s\r\n"
                 "Content-Type: text/plain; charset=UTF-8\r\n"
                 "\r\n"
                 "🎉 CONGRATULATIONS %s! 🎉\r\n"
                 "══════════════════════════════════════════\r\n"
                 "\r\n"
                 "Your course reservation has been successfully confirmed!\r\n"
                 "\r\n"
                 "📋 RESERVATION DETAILS:\r\n"
                 "══════════════════════════════════════════\r\n"
                 "• Member Name: %s\r\n"
                 "• Course: %s (ID: %s)\r\n"
                 "• Center: %s\r\n"
                 "• Time Slot: %s\r\n"
                 "• Reservation Date: %s\r\n"
                 "• Confirmation ID: R%d\r\n"
                 "\r\n"
                 "📍 IMPORTANT INFORMATION:\r\n"
                 "══════════════════════════════════════════\r\n"
                 "• Please arrive 10-15 minutes before the class\r\n"
                 "• Bring comfortable sports clothing\r\n"
                 "• Water bottle is recommended\r\n"
                 "• Please inform us if you cannot attend\r\n"
                 "\r\n"
                 "📞 CONTACT INFORMATION:\r\n"
                 "══════════════════════════════════════════\r\n"
                 "• TopGym Support: +216 12 345 678\r\n"
                 "• Email: support@topgym.com\r\n"
                 "• Website: www.topgym-tunisia.com\r\n"
                 "\r\n"
                 "📍 CENTER LOCATION:\r\n"
                 "══════════════════════════════════════════\r\n"
                 "TopGym %s Center\r\n"
                 "Tunis, Tunisia\r\n"
                 "\r\n"
                 "Thank you for choosing TopGym!\r\n"
                 "We look forward to helping you achieve your fitness goals! 💪\r\n"
                 "\r\n"
                 "Best regards,\r\n"
                 "The TopGym Team\r\n"
                 "🏆 Excellence in Fitness! 🏆\r\n"
                 "\r\n"
                 "📢 Follow us on social media:\r\n"
                 "Facebook: @TopGymTunisia\r\n"
                 "Instagram: @TopGym_TN\r\n"
                 "\r\n"
                 ".\r\n",
                 member_name, member_email,
                 subject, member_name, member_name,
                 course_name, course_id, course_center,
                 course_time, reservation_date, conf_id,
                 course_center);

        const char *email_data = message;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "RESERVATION EMAIL ERROR: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return (res == CURLE_OK);
}

// =====================================================
//                   TREEVIEW FUNCTIONS
// =====================================================

void on_cell_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer data) {
    GtkWidget *treeview = GTK_WIDGET(data);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    GtkTreeIter iter;
    gboolean active;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &active, -1);
        active = !active;
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, active, -1);
    }

    gtk_tree_path_free(path);
}

void on_cell_edited(GtkCellRendererText *cell, gchar *path_str, gchar *new_text, gpointer data) {
    GtkWidget *treeview = GTK_WIDGET(data);
    GtkWidget *window = gtk_widget_get_toplevel(treeview);
    
    if (!show_confirmation_dialog(window, "Confirm Modification", "Do you want to save the changes to this course?")) {
        return;
    }
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    GtkTreeIter iter;
    
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_path_free(path);
        return;
    }
    
    // Get the course ID from the treeview
    gchar *course_id = NULL;
    gtk_tree_model_get(model, &iter, 1, &course_id, -1);
    
    if (!course_id) {
        gtk_tree_path_free(path);
        return;
    }
    
    // Get current values from all columns
    gchar *id, *name, *type, *center, *time_str, *equipment_str, *capacity_str;
    gtk_tree_model_get(model, &iter, 
                       1, &id,
                       2, &name,
                       3, &type,
                       4, &center,
                       5, &time_str,
                       6, &equipment_str,
                       7, &capacity_str,
                       -1);
    
    // Find which column was edited
    GtkTreeViewColumn *column = NULL;
    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
    GList *col_iter = columns;
    int col_index = 0;
    int edited_column = -1;
    
    // Get the column that contains this cell renderer
    while (col_iter) {
        GtkTreeViewColumn *col = GTK_TREE_VIEW_COLUMN(col_iter->data);
        GList *renderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(col));
        GList *rend_iter = renderers;
        
        while (rend_iter) {
            if (rend_iter->data == cell) {
                edited_column = col_index;
                column = col;
                break;
            }
            rend_iter = rend_iter->next;
        }
        
        g_list_free(renderers);
        
        if (edited_column != -1)
            break;
            
        col_index++;
        col_iter = col_iter->next;
    }
    g_list_free(columns);
    
    if (edited_column == -1 || edited_column == 0 || edited_column == 1) {
        // Can't edit checkbox or ID column
        g_free(course_id);
        g_free(id); g_free(name); g_free(type); g_free(center);
        g_free(time_str); g_free(equipment_str); g_free(capacity_str);
        gtk_tree_path_free(path);
        return;
    }
    
    // Update the value in the tree store
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, edited_column, new_text, -1);
    
    // Update the file
    FILE *f = fopen("courses.txt", "r");
    FILE *f2 = fopen("temp.txt", "w");
    
    if (f && f2) {
        course c;
        int updated = 0;
        
        while (fscanf(f, "%s %s %s %s %d %d %d",
                     c.id, c.name, c.type, c.center,
                     &c.time, &c.equipment, &c.capacity) != EOF) {
            if (strcmp(c.id, course_id) == 0) {
                // Update the specific field that was edited
                switch(edited_column) {
                    case 2: // Name column
                        strncpy(c.name, new_text, sizeof(c.name)-1);
                        c.name[sizeof(c.name)-1] = '\0';
                        break;
                    case 3: // Type column
                        strncpy(c.type, new_text, sizeof(c.type)-1);
                        c.type[sizeof(c.type)-1] = '\0';
                        break;
                    case 4: // Center column
                        strncpy(c.center, new_text, sizeof(c.center)-1);
                        c.center[sizeof(c.center)-1] = '\0';
                        break;
                    case 5: // Time column
                        if (strcasecmp(new_text, "Morning") == 0) c.time = 1;
                        else if (strcasecmp(new_text, "Afternoon") == 0) c.time = 2;
                        else if (strcasecmp(new_text, "Night") == 0) c.time = 3;
                        else c.time = atoi(new_text);
                        break;
                    case 6: // Equipment column
                        if (strcasecmp(new_text, "Treadmill") == 0) c.equipment = 1;
                        else if (strcasecmp(new_text, "Bunch press") == 0) c.equipment = 2;
                        else if (strcasecmp(new_text, "Squat machine") == 0) c.equipment = 3;
                        else if (strcasecmp(new_text, "No equipment") == 0) c.equipment = 4;
                        else c.equipment = atoi(new_text);
                        break;
                    case 7: // Capacity column
                        c.capacity = atoi(new_text);
                        if (c.capacity <= 0) c.capacity = 1; // Minimum capacity
                        break;
                }
                updated = 1;
            }
            fprintf(f2, "%s %s %s %s %d %d %d\n",
                   c.id, c.name, c.type, c.center,
                   c.time, c.equipment, c.capacity);
        }
        fclose(f);
        fclose(f2);
        
        if (updated) {
            remove("courses.txt");
            rename("temp.txt", "courses.txt");
            
            show_popup_message(window, "Course Modified", "Course modified successfully!", GTK_MESSAGE_INFO);
            
            // Send email notification
            char email_body[256];
            snprintf(email_body, sizeof(email_body),
                    "A course has been modified in the TopGym system.\n"
                    "Course ID: %s\nField modified: %s\n"
                    "New value: %s\n"
                    "Action performed by: System Administrator",
                    course_id, 
                    edited_column == 2 ? "Name" : 
                    edited_column == 3 ? "Type" : 
                    edited_column == 4 ? "Center" : 
                    edited_column == 5 ? "Time" : 
                    edited_column == 6 ? "Equipment" : "Capacity",
                    new_text);
            
            char email_subject[256];
            snprintf(email_subject, sizeof(email_subject),
                    "✏️ Course Modified via Treeview: %s", c.name);
            
            send_course_email_to_admin(email_subject, email_body, "modify", c);
        } else {
            remove("temp.txt");
        }
    } else {
        if (f) fclose(f);
        if (f2) fclose(f2);
    }
    
    // Free memory
    g_free(course_id);
    g_free(id);
    g_free(name);
    g_free(type);
    g_free(center);
    g_free(time_str);
    g_free(equipment_str);
    g_free(capacity_str);
    gtk_tree_path_free(path);
}

void initialize_treeview_management_columns(GtkWidget *treeview) {
    // Remove all existing columns
    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
    if (columns) {
        // Keep removing first column until none left
        while (gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0) != NULL) {
            GtkTreeViewColumn *col = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0);
            if (col) {
                gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), col);
            }
        }
        g_list_free(columns);
    }

    // Create checkbox column
    GtkCellRenderer *check_renderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn *check_column = gtk_tree_view_column_new_with_attributes("Select", 
                                                                               check_renderer,
                                                                               "active", 0,
                                                                               NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), check_column);
    g_signal_connect(check_renderer, "toggled", G_CALLBACK(on_cell_toggled), treeview);

    // ID column (NOT editable - ID is immutable)
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", FALSE, NULL); // ID is not editable
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Name column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Type column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Center column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Center", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Time column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Time", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Equipment column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Equipment", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Capacity column (editable)
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), treeview);
    column = gtk_tree_view_column_new_with_attributes("Capacity", renderer, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

void initialize_treeview_selection_columns(GtkWidget *treeview) {
    // Remove all existing columns
    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
    if (columns) {
        // Keep removing first column until none left
        while (gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0) != NULL) {
            GtkTreeViewColumn *col = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0);
            if (col) {
                gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), col);
            }
        }
        g_list_free(columns);
    }

    // Create checkbox column
    GtkCellRenderer *check_renderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn *check_column = gtk_tree_view_column_new_with_attributes("Select", 
                                                                               check_renderer,
                                                                               "active", 0,
                                                                               NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), check_column);
    g_signal_connect(check_renderer, "toggled", G_CALLBACK(on_cell_toggled), treeview);

    // ID column
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Course ID", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Name column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Course Name", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Type column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Center column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Center", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Time column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Time", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // Available spots column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Available Spots", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

GList* get_selected_course_ids_management(GtkWidget *treeview) {
    GList *selected_ids = NULL;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return NULL;
    
    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gboolean selected;
        gchar *id;
        
        gtk_tree_model_get(model, &iter, 0, &selected, 1, &id, -1);
        
        if (selected && id) {
            selected_ids = g_list_append(selected_ids, g_strdup(id));
        }
        
        if (id) g_free(id);
        valid = gtk_tree_model_iter_next(model, &iter);
    }
    
    return selected_ids;
}

GList* get_selected_course_ids_selection(GtkWidget *treeview) {
    GList *selected_ids = NULL;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return NULL;
    
    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gboolean selected;
        gchar *id;
        
        gtk_tree_model_get(model, &iter, 0, &selected, 1, &id, -1);
        
        if (selected && id) {
            selected_ids = g_list_append(selected_ids, g_strdup(id));
        }
        
        if (id) g_free(id);
        valid = gtk_tree_model_iter_next(model, &iter);
    }
    
    return selected_ids;
}

void load_courses_to_treeview_management(GtkWidget *treeview) {
    FILE *f = fopen("courses.txt", "r");
    if (!f) {
        show_popup_message(gtk_widget_get_toplevel(treeview), "Error", "Cannot open courses.txt for reading", GTK_MESSAGE_ERROR);
        return;
    }

    course c;
    GtkTreeIter iter;

    if (store_management == NULL) {
        // 8 columns: checkbox + 7 course fields
        store_management = gtk_tree_store_new(8,
            G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
            G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_management));
        initialize_treeview_management_columns(treeview);
    } else {
        gtk_tree_store_clear(store_management);
    }

    int count = 0;
    while (fscanf(f, "%s %s %s %s %d %d %d",
                  c.id, c.name, c.type, c.center,
                  &c.time, &c.equipment, &c.capacity) != EOF) {
        char time_str[20];
        switch(c.time) {
            case 1: strcpy(time_str, "Morning"); break;
            case 2: strcpy(time_str, "Afternoon"); break;
            case 3: strcpy(time_str, "Night"); break;
            default: snprintf(time_str, sizeof(time_str), "%d", c.time);
        }

        char equipment_str[30];
        switch(c.equipment) {
            case 1: strcpy(equipment_str, "Treadmill"); break;
            case 2: strcpy(equipment_str, "Bunch press"); break;
            case 3: strcpy(equipment_str, "Squat machine"); break;
            case 4: strcpy(equipment_str, "No equipment"); break;
            default: snprintf(equipment_str, sizeof(equipment_str), "%d", c.equipment);
        }

        char capacity_str[10];
        snprintf(capacity_str, sizeof(capacity_str), "%d", c.capacity);

        gtk_tree_store_append(store_management, &iter, NULL);
        gtk_tree_store_set(store_management, &iter,
            0, FALSE,
            1, c.id,
            2, c.name,
            3, c.type,
            4, c.center,
            5, time_str,
            6, equipment_str,
            7, capacity_str,
            -1);
        count++;
    }

    fclose(f);
}

void load_courses_to_treeview_selection(GtkWidget *treeview) {
    if (!treeview || !GTK_IS_TREE_VIEW(treeview)) {
        return;
    }

    FILE *f = fopen("courses.txt", "r");
    if (!f) {
        show_popup_message(gtk_widget_get_toplevel(treeview), "Error", "Cannot open courses.txt file", GTK_MESSAGE_ERROR);
        return;
    }

    course c;
    GtkTreeIter iter;

    if (store_selection == NULL) {
        // 7 columns: checkbox + 6 course fields
        store_selection = gtk_tree_store_new(7,
            G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
            G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_selection));
        
        // Initialize columns for GTK2
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        // Checkbox column
        renderer = gtk_cell_renderer_toggle_new();
        column = gtk_tree_view_column_new_with_attributes("Select", renderer, "active", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        g_signal_connect(renderer, "toggled", G_CALLBACK(on_cell_toggled), treeview);
        
        // ID column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Course ID", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Name column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Course Name", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Type column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Center column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Center", renderer, "text", 4, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Time column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Time", renderer, "text", 5, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Available spots column
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Available Spots", renderer, "text", 6, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
    } else {
        gtk_tree_store_clear(store_selection);
    }

    int displayed_courses = 0;

    while (fscanf(f, "%s %s %s %s %d %d %d",
                  c.id, c.name, c.type, c.center,
                  &c.time, &c.equipment, &c.capacity) != EOF) {

        int available_spots = check_course_capacity(c.id);
        
        // Formater l'affichage des places
        char spots_str[50];
        if (available_spots > 0) {
            snprintf(spots_str, sizeof(spots_str), "%d/%d", available_spots, c.capacity);
        } else {
            snprintf(spots_str, sizeof(spots_str), "FULL (%d/%d)", c.capacity, c.capacity);
        }

        char time_str[20];
        switch(c.time) {
            case 1: strcpy(time_str, "Morning"); break;
            case 2: strcpy(time_str, "Afternoon"); break;
            case 3: strcpy(time_str, "Night"); break;
            default: snprintf(time_str, sizeof(time_str), "%d", c.time);
        }

        gtk_tree_store_append(store_selection, &iter, NULL);
        gtk_tree_store_set(store_selection, &iter,
            0, FALSE,  // Checkbox désactivée
            1, c.id,
            2, c.name,
            3, c.type,
            4, c.center,
            5, time_str,
            6, spots_str,  // Format "X/Y" ou "FULL"
            -1);
        displayed_courses++;
    }

    fclose(f);
}

// NOUVELLE FONCTION : Vérifier la capacité d'un cours
int check_course_capacity(const char *course_id) {
    // Obtenir la capacité du cours
    FILE *f_course = fopen("courses.txt", "r");
    int capacity = 0;
    
    if (!f_course) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir courses.txt\n");
        return -1;
    }
    
    course c;
    while (fscanf(f_course, "%s %s %s %s %d %d %d",
                 c.id, c.name, c.type, c.center,
                 &c.time, &c.equipment, &c.capacity) != EOF) {
        if (strcmp(c.id, course_id) == 0) {
            capacity = c.capacity;
            break;
        }
    }
    fclose(f_course);
    
    if (capacity == 0) return 0;
    
    // Compter les réservations actuelles
    int reservations = 0;
    FILE *f_res = fopen("reservations.txt", "r");
    
    if (f_res) {
        char line[256];
        while (fgets(line, sizeof(line), f_res)) {
            char id_cours[30];
            // Essayer de lire l'ID du cours (dernier champ)
            char *last_space = strrchr(line, ' ');
            if (last_space) {
                // Extraire l'ID du cours (après le dernier espace)
                strcpy(id_cours, last_space + 1);
                // Supprimer le saut de ligne
                id_cours[strcspn(id_cours, "\n")] = '\0';
                
                if (strcmp(id_cours, course_id) == 0) {
                    reservations++;
                }
            }
        }
        fclose(f_res);
    }
    
    return capacity - reservations;
}

// =====================================================
//                   UPDATE BOTH TREEVIEWS FUNCTION
// =====================================================
void update_both_treeviews(GtkWidget *current_window) {
    // Update current treeview (management)
    GtkWidget *treeview_management = lookup_widget(current_window, "treeviewcourse_y");
    if (treeview_management && GTK_IS_TREE_VIEW(treeview_management)) {
        load_courses_to_treeview_management(treeview_management);
    }
    
    // Find and update reservation treeview
    GtkWidget *treeview_selection = lookup_widget(current_window, "treeviewchoose_y");
    if (treeview_selection && GTK_IS_TREE_VIEW(treeview_selection)) {
        load_courses_to_treeview_selection(treeview_selection);
    }
}

// =====================================================
//                   CAPACITY FUNCTION
// =====================================================
int get_available_spots(const char *course_id) {
    // Get course capacity
    FILE *f_course = fopen("courses.txt", "r");
    int capacity = 0;
    int found = 0;
    
    if (f_course) {
        course c;
        while (fscanf(f_course, "%s %s %s %s %d %d %d",
                     c.id, c.name, c.type, c.center,
                     &c.time, &c.equipment, &c.capacity) != EOF) {
            if (strcmp(c.id, course_id) == 0) {
                capacity = c.capacity;
                found = 1;
                break;
            }
        }
        fclose(f_course);
    }
    
    if (!found || capacity == 0) {
        return 0;
    }
    
    // Count current reservations
    FILE *f_res = fopen("reservations.txt", "r");
    int reservations_count = 0;
    
    if (f_res) {
        reservation r;
        char line[256];
        
        // Read line by line
        while (fgets(line, sizeof(line), f_res)) {
            if (sscanf(line, "%s %s %s %d %d %d %s",
                      r.id_member, r.name_member, r.phone_number,
                      &r.date.jour, &r.date.mois, &r.date.annee,
                      r.id_cours) == 7) {
                if (strcmp(r.id_cours, course_id) == 0) {
                    reservations_count++;
                }
            }
        }
        fclose(f_res);
    }
    
    int available = capacity - reservations_count;
    return (available > 0) ? available : 0;
}

// =====================================================
//                   COURSES CALLBACKS
// =====================================================

void on_btadd_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if (!show_confirmation_dialog(window, "Confirm Addition", "Do you want to add this course?")) {
        return;
    }

    GtkWidget *entry_id = lookup_widget(window, "entryid_y");
    GtkWidget *entry_name = lookup_widget(window, "entryname2_y");
    GtkWidget *combo_type = lookup_widget(window, "combo1_y");
    GtkWidget *combo_center = lookup_widget(window, "entrycenter2_y");
    GtkWidget *spin_cap = lookup_widget(window, "capacity_y");
    GtkWidget *treeview = lookup_widget(window, "treeviewcourse_y");

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entry_name));

    if (strlen(id_text) == 0) {
        show_popup_message(window, "Error", "ID cannot be empty!", GTK_MESSAGE_WARNING);
        return;
    }

    if (!is_number(id_text)) {
        show_popup_message(window, "Error", "ID must contain only numbers!", GTK_MESSAGE_WARNING);
        return;
    }

    if (strlen(name_text) == 0) {
        show_popup_message(window, "Error", "Name cannot be empty!", GTK_MESSAGE_WARNING);
        return;
    }

    const char *type_text = get_combo_text(combo_type);

    const char *center_text = "";
    if (GTK_IS_COMBO_BOX_ENTRY(combo_center)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_center));
        if (GTK_IS_ENTRY(entry)) {
            center_text = gtk_entry_get_text(GTK_ENTRY(entry));
        }
    } else {
        center_text = get_combo_text(combo_center);
    }

    if (strlen(type_text) == 0) {
        show_popup_message(window, "Error", "Please select a type!", GTK_MESSAGE_WARNING);
        return;
    }

    if (strlen(center_text) == 0) {
        show_popup_message(window, "Error", "Please select a center!", GTK_MESSAGE_WARNING);
        return;
    }

    if (time_c == 0) {
        show_popup_message(window, "Error", "Please select time!", GTK_MESSAGE_WARNING);
        return;
    }

    if (equipment_c == 0) {
        show_popup_message(window, "Error", "Please select equipment!", GTK_MESSAGE_WARNING);
        return;
    }

    int capacity = 0;
    if (spin_cap && GTK_IS_SPIN_BUTTON(spin_cap)) {
        capacity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_cap));
        if (capacity <= 0) capacity = 10;
    } else {
        capacity = 10;
    }

    course c;
    strncpy(c.id, id_text, sizeof(c.id) - 1);
    strncpy(c.name, name_text, sizeof(c.name) - 1);
    strncpy(c.type, type_text, sizeof(c.type) - 1);
    strncpy(c.center, center_text, sizeof(c.center) - 1);
    c.time = time_c;
    c.equipment = equipment_c;
    c.capacity = capacity;

    if (add("courses.txt", c)) {
        show_popup_message(window, "Success", "Course added successfully!", GTK_MESSAGE_INFO);

        char email_body[256];
        snprintf(email_body, sizeof(email_body),
                "A new course has been added to the TopGym system.\n"
                "Course ID: %s\nCourse Name: %s\n"
                "Center: %s\nCapacity: %d\n"
                "Action performed by: System Administrator",
                id_text, name_text, center_text, capacity);

        char email_subject[256];
        snprintf(email_subject, sizeof(email_subject), "➕ New Course Added: %s", name_text);

        send_course_email_to_admin(email_subject, email_body, "add", c);

        if (treeview && GTK_IS_TREE_VIEW(treeview)) {
            load_courses_to_treeview_management(treeview);
        }
    } else {
        show_popup_message(window, "Error", "Error: Course ID already exists!", GTK_MESSAGE_ERROR);
    }

    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    gtk_entry_set_text(GTK_ENTRY(entry_name), "");

    if (GTK_IS_COMBO(combo_type)) {
        GtkCombo *type_combo = GTK_COMBO(combo_type);
        gtk_entry_set_text(GTK_ENTRY(type_combo->entry), "");
    }

    if (GTK_IS_COMBO_BOX_ENTRY(combo_center)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_center));
        if (GTK_IS_ENTRY(entry))
            gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
    update_both_treeviews(window);
}

void on_btsearch_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *entry_search_name = lookup_widget(window, "entryname_y");
    GtkWidget *combo_search_center = lookup_widget(window, "entrycentre1_y");
    GtkWidget *check_name = lookup_widget(window, "checkbuttonname_y");
    GtkWidget *check_center = lookup_widget(window, "checkbuttoncentre_y");
    GtkWidget *treeview = lookup_widget(window, "treeviewcourse_y");

    char name[30] = "";
    char center[30] = "";

    gboolean search_by_name = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_name));
    gboolean search_by_center = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_center));

    if (search_by_name) {
        const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entry_search_name));
        if (name_text != NULL && strlen(name_text) > 0) {
            strncpy(name, name_text, sizeof(name) - 1);
        } else {
            show_popup_message(window, "Warning", "Name filter is active but name field is empty!", GTK_MESSAGE_WARNING);
            return;
        }
    }

    if (search_by_center) {
        const char *center_text = "";
        if (GTK_IS_COMBO_BOX_ENTRY(combo_search_center)) {
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_search_center));
            if (GTK_IS_ENTRY(entry)) {
                center_text = gtk_entry_get_text(GTK_ENTRY(entry));
            }
        } else {
            center_text = get_combo_text(combo_search_center);
        }

        if (center_text != NULL && strlen(center_text) > 0) {
            strncpy(center, center_text, sizeof(center) - 1);
        } else {
            show_popup_message(window, "Warning", "Center filter is active but center field is empty!", GTK_MESSAGE_WARNING);
            return;
        }
    }

    if (!search_by_name && !search_by_center) {
        show_popup_message(window, "Warning", "Please activate at least one filter!\nCheck 'Name' and/or 'Center' checkbox", 
                   GTK_MESSAGE_WARNING);
        return;
    }

    const char *search_name = (search_by_name && strlen(name) > 0) ? name : "";
    const char *search_center = (search_by_center && strlen(center) > 0) ? center : "";

    course found_courses[100];
    int num_found = search_all_courses("courses.txt",
                                      (char*)search_name,
                                      (char*)search_center,
                                      found_courses,
                                      100);

    if (num_found > 0) {
        char result_msg[256];
        if (search_by_name && search_by_center) {
            snprintf(result_msg, sizeof(result_msg),
                    "Found %d course(s)\nName: '%s'\nCenter: '%s'",
                    num_found, search_name, search_center);
        } else if (search_by_name) {
            snprintf(result_msg, sizeof(result_msg),
                    "Found %d course(s)\nName: '%s'",
                    num_found, search_name);
        } else if (search_by_center) {
            snprintf(result_msg, sizeof(result_msg),
                    "Found %d course(s)\nCenter: '%s'",
                    num_found, search_center);
        }

        show_popup_message(window, "Info", result_msg, GTK_MESSAGE_INFO);

        if (store_search == NULL) {
            store_search = gtk_tree_store_new(8,
                G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
            gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_search));
            initialize_treeview_management_columns(treeview);
        } else {
            gtk_tree_store_clear(store_search);
        }

        for (int i = 0; i < num_found; i++) {
            GtkTreeIter iter;
            gtk_tree_store_append(store_search, &iter, NULL);
            
            char time_str[20];
            switch(found_courses[i].time) {
                case 1: strcpy(time_str, "Morning"); break;
                case 2: strcpy(time_str, "Afternoon"); break;
                case 3: strcpy(time_str, "Night"); break;
                default: snprintf(time_str, sizeof(time_str), "%d", found_courses[i].time);
            }

            char equipment_str[30];
            switch(found_courses[i].equipment) {
                case 1: strcpy(equipment_str, "Treadmill"); break;
                case 2: strcpy(equipment_str, "Bunch press"); break;
                case 3: strcpy(equipment_str, "Squat machine"); break;
                case 4: strcpy(equipment_str, "No equipment"); break;
                default: snprintf(equipment_str, sizeof(equipment_str), "%d", found_courses[i].equipment);
            }

            char capacity_str[10];
            snprintf(capacity_str, sizeof(capacity_str), "%d", found_courses[i].capacity);

            gtk_tree_store_set(store_search, &iter,
                0, FALSE,
                1, found_courses[i].id,
                2, found_courses[i].name,
                3, found_courses[i].type,
                4, found_courses[i].center,
                5, time_str,
                6, equipment_str,
                7, capacity_str,
                -1);
        }
    } else {
        char error_msg[256];
        if (search_by_name && search_by_center) {
            snprintf(error_msg, sizeof(error_msg),
                    "No courses found!\nSearch criteria:\n- Name: '%s'\n- Center: '%s'\n\nSuggestions:\n1. Check spelling\n2. Try using only one filter",
                    search_name, search_center);
        } else if (search_by_name) {
            snprintf(error_msg, sizeof(error_msg),
                    "No courses found!\nSearch criteria:\n- Name: '%s'\n\nSuggestions:\n1. Check spelling\n2. Try partial name",
                    search_name);
        } else if (search_by_center) {
            snprintf(error_msg, sizeof(error_msg),
                    "No courses found!\nSearch criteria:\n- Center: '%s'\n\nSuggestions:\n1. Check center name\n2. Center might be empty in database",
                    search_center);
        }

        show_popup_message(window, "Warning", error_msg, GTK_MESSAGE_WARNING);

        if (store_search != NULL) {
            gtk_tree_store_clear(store_search);
        }
    }
}

void on_btdelete_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "treeviewcourse_y");
    GtkWidget *entry_id = lookup_widget(window, "entryid_y");

    GList *selected_ids = get_selected_course_ids_management(treeview);
    
    if (selected_ids == NULL) {
        const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
        if (strlen(id_text) > 0) {
            selected_ids = g_list_append(NULL, g_strdup(id_text));
        }
    }

    if (selected_ids == NULL) {
        show_popup_message(window, "Warning", "Please select courses to delete!\nYou can either:\n1. Check courses in the list\n2. Enter course ID in the field", 
                   GTK_MESSAGE_WARNING);
        return;
    }

    char confirmation_msg[256];
    int count = g_list_length(selected_ids);
    if (count == 1) {
        snprintf(confirmation_msg, sizeof(confirmation_msg), 
                "Do you want to delete course ID: %s?", 
                (char*)selected_ids->data);
    } else {
        snprintf(confirmation_msg, sizeof(confirmation_msg), 
                "Do you want to delete %d selected courses?", count);
    }
    
    if (!show_confirmation_dialog(window, "Confirm Deletion", confirmation_msg)) {
        g_list_free_full(selected_ids, g_free);
        return;
    }

    int deleted_count = 0;
    GList *iter = selected_ids;
    
    while (iter != NULL) {
        char *id_to_delete = (char*)iter->data;
        
        FILE *f = fopen("courses.txt", "r");
        course course_to_delete;
        int course_found = 0;

        if (f != NULL) {
            while (fscanf(f, "%s %s %s %s %d %d %d",
                          course_to_delete.id, course_to_delete.name, course_to_delete.type, course_to_delete.center,
                          &course_to_delete.time, &course_to_delete.equipment, &course_to_delete.capacity) != EOF) {
                if (strcmp(course_to_delete.id, id_to_delete) == 0) {
                    course_found = 1;
                    break;
                }
            }
            fclose(f);
        }

        if (course_found) {
            if (Delete("courses.txt", id_to_delete)) {
                deleted_count++;
                
                char email_body[256];
                snprintf(email_body, sizeof(email_body),
                        "A course has been deleted from the TopGym system.\n"
                        "Deleted Course ID: %s\nDeleted Course Name: %s\n"
                        "Center: %s\nCapacity: %d\n"
                        "Action performed by: System Administrator",
                        course_to_delete.id, course_to_delete.name,
                        course_to_delete.center, course_to_delete.capacity);

                char email_subject[256];
                snprintf(email_subject, sizeof(email_subject),
                        "🗑️ Course Deleted: %s", course_to_delete.name);

                send_course_email_to_admin(email_subject, email_body, "delete", course_to_delete);
            }
        }
        
        iter = iter->next;
    }

    char result_msg[256];
    if (deleted_count > 0) {
        snprintf(result_msg, sizeof(result_msg), 
                "Successfully deleted %d out of %d courses!", 
                deleted_count, count);
        show_popup_message(window, "Success", result_msg, GTK_MESSAGE_INFO);
        
        load_courses_to_treeview_management(treeview);
    } else {
        show_popup_message(window, "Warning", "No courses were deleted. They might not exist.", 
                   GTK_MESSAGE_WARNING);
    }

    g_list_free_full(selected_ids, g_free);
    gtk_entry_set_text(GTK_ENTRY(entry_id), "");

    update_both_treeviews(window);
}

void on_btmodify_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if (!show_confirmation_dialog(window, "Confirm Modification", "Do you want to modify this course?")) {
        return;
    }

    GtkWidget *entry_id = lookup_widget(window, "entryid_y");
    GtkWidget *entry_name = lookup_widget(window, "entryname2_y");
    GtkWidget *combo_type = lookup_widget(window, "combo1_y");
    GtkWidget *combo_center = lookup_widget(window, "entrycenter2_y");
    GtkWidget *spin_cap = lookup_widget(window, "capacity_y");
    GtkWidget *treeview = lookup_widget(window, "treeviewcourse_y");

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));

    if (strlen(id_text) == 0) {
        show_popup_message(window, "Warning", "Please enter ID!", GTK_MESSAGE_WARNING);
        return;
    }

    const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const char *type_text = get_combo_text(combo_type);

    const char *center_text = "";
    if (GTK_IS_COMBO_BOX_ENTRY(combo_center)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_center));
        if (GTK_IS_ENTRY(entry)) {
            center_text = gtk_entry_get_text(GTK_ENTRY(entry));
        }
    }

    if (strlen(name_text) == 0) {
        show_popup_message(window, "Warning", "Name cannot be empty!", GTK_MESSAGE_WARNING);
        return;
    }

    if (strlen(type_text) == 0) {
        show_popup_message(window, "Warning", "Please select type!", GTK_MESSAGE_WARNING);
        return;
    }

    if (strlen(center_text) == 0) {
        show_popup_message(window, "Warning", "Please select center!", GTK_MESSAGE_WARNING);
        return;
    }

    if (time_c == 0) {
        show_popup_message(window, "Warning", "Please select time!", GTK_MESSAGE_WARNING);
        return;
    }

    if (equipment_c == 0) {
        show_popup_message(window, "Warning", "Please select equipment!", GTK_MESSAGE_WARNING);
        return;
    }

    int capacity = 0;
    if (spin_cap && GTK_IS_SPIN_BUTTON(spin_cap)) {
        capacity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_cap));
        if (capacity <= 0) capacity = 10;
    } else {
        capacity = 10;
    }

    course nouv;
    strncpy(nouv.id, id_text, sizeof(nouv.id) - 1);
    strncpy(nouv.name, name_text, sizeof(nouv.name) - 1);
    strncpy(nouv.type, type_text, sizeof(nouv.type) - 1);
    strncpy(nouv.center, center_text, sizeof(nouv.center) - 1);
    nouv.time = time_c;
    nouv.equipment = equipment_c;
    nouv.capacity = capacity;

    if (modify("courses.txt", (char*)id_text, nouv)) {
        show_popup_message(window, "Success", "Course modified successfully!", GTK_MESSAGE_INFO);

        char email_body[256];
        snprintf(email_body, sizeof(email_body),
                "Course information has been updated in the TopGym system.\n"
                "Course ID: %s\nUpdated Name: %s\n"
                "Updated Center: %s\nUpdated Capacity: %d\n"
                "Action performed by: System Administrator",
                id_text, name_text, center_text, capacity);

        char email_subject[256];
        snprintf(email_subject, sizeof(email_subject), "✏️ Course Modified: %s", name_text);

        send_course_email_to_admin(email_subject, email_body, "modify", nouv);

        if (treeview && GTK_IS_TREE_VIEW(treeview)) {
            load_courses_to_treeview_management(treeview);
        }
    } else {
        show_popup_message(window, "Error", "Course not found!", GTK_MESSAGE_ERROR);
    }

    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    gtk_entry_set_text(GTK_ENTRY(entry_name), "");

    if (GTK_IS_COMBO(combo_type)) {
        GtkCombo *type_combo = GTK_COMBO(combo_type);
        gtk_entry_set_text(GTK_ENTRY(type_combo->entry), "");
    }

    if (GTK_IS_COMBO_BOX_ENTRY(combo_center)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_center));
        if (GTK_IS_ENTRY(entry))
            gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
    update_both_treeviews(window);
}

void on_treeviewcourse_y_row_activated(GtkTreeView *treeview,
                                    GtkTreePath *path,
                                    GtkTreeViewColumn *column,
                                    gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
    GtkWidget *entry_id = lookup_widget(window, "entryid_y");

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gchar *selected_id = NULL;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 1, &selected_id, -1);
        if (selected_id != NULL) {
            gtk_entry_set_text(GTK_ENTRY(entry_id), selected_id);
            g_free(selected_id);
        }
    }
}

void on_radiobuttonMorning_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        time_c = 1;
}

void on_radiobuttonafternoon_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        time_c = 2;
}

void on_radiobuttonnight_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        time_c = 3;
}

void on_radiobuttonTreadmill_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        equipment_c = 1;
}

void on_radiobuttonbunchpress_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        equipment_c = 2;
}

void on_radiobuttonsquatmachine_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        equipment_c = 3;
}

void on_radiobuttonNo_y_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton))
        equipment_c = 4;
}

void on_load_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "treeviewcourse_y");

    if (!treeview || !GTK_IS_TREE_VIEW(treeview)) {
        show_popup_message(window, "Error", "Error: Treeview not initialized!", GTK_MESSAGE_ERROR);
        return;
    }

    load_courses_to_treeview_management(treeview);

    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model) {
        gint row_count = gtk_tree_model_iter_n_children(model, NULL);
        if (row_count > 0) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Courses loaded successfully! %d course(s) found.", row_count);
            show_popup_message(window, "Success", msg, GTK_MESSAGE_INFO);
        } else {
            show_popup_message(window, "Warning", "No courses found in the database!", GTK_MESSAGE_WARNING);
        }
    } else {
        show_popup_message(window, "Error", "Failed to load courses!", GTK_MESSAGE_ERROR);
    }
}

void on_loadcourses_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview_choose = lookup_widget(window, "treeviewchoose_y");

    if (!treeview_choose || !GTK_IS_TREE_VIEW(treeview_choose)) {
        show_popup_message(window, "Error", "Error: Treeview not initialized!", GTK_MESSAGE_ERROR);
        return;
    }

    load_courses_to_treeview_selection(treeview_choose);

    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_choose));
    if (model) {
        gint row_count = gtk_tree_model_iter_n_children(model, NULL);
        if (row_count > 0) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Courses loaded successfully! %d course(s) available.", row_count);
            show_popup_message(window, "Success", msg, GTK_MESSAGE_INFO);
        } else {
            show_popup_message(window, "Warning", "No courses available or all courses are full!", GTK_MESSAGE_WARNING);
        }
    } else {
        show_popup_message(window, "Error", "Failed to load courses!", GTK_MESSAGE_ERROR);
    }
}

void on_treeviewchoose_y_row_activated(GtkTreeView *treeview,
                                    GtkTreePath *path,
                                    GtkTreeViewColumn *column,
                                    gpointer user_data) {
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gboolean active;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &active, -1);
        active = !active;
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, active, -1);
    }
}

void on_btchoose_y_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview_choose = lookup_widget(window, "treeviewchoose_y");
    GtkWidget *entry_member_id = lookup_widget(window, "entryid");

    // Validation de base
    const gchar *member_id = gtk_entry_get_text(GTK_ENTRY(entry_member_id));
    if (strlen(member_id) == 0) {
        show_popup_message(window, "Warning", "Member ID cannot be empty!", GTK_MESSAGE_WARNING);
        return;
    }
    
    // Valider le membre
    char member_name[100];
    if (!validate_member_id(member_id, member_name, sizeof(member_name))) {
        show_popup_message(window, "Warning", "Member ID not found!", GTK_MESSAGE_WARNING);
        return;
    }
    
    // Obtenir les cours sélectionnés
    GList *selected_ids = get_selected_course_ids_selection(treeview_choose);
    if (!selected_ids) {
        show_popup_message(window, "Warning", "Please select at least one course!", GTK_MESSAGE_WARNING);
        return;
    }
    
    // Date actuelle
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    int day = tm_info->tm_mday;
    int month = tm_info->tm_mon + 1;
    int year = tm_info->tm_year + 1900;
    
    GList *iter = selected_ids;
    int success_count = 0;
    int full_courses = 0;
    int already_reserved = 0;
    
    // FICHIER TEMPORAIRE pour les réservations
    char temp_filename[] = "reservations_temp.txt";
    
    while (iter != NULL) {
        char *course_id = (char*)iter->data;
        
        // VÉRIFIER LA CAPACITÉ EN TEMPS RÉEL
        int available_spots = check_course_capacity(course_id);
        
        if (available_spots <= 0) {
            // Obtenir le nom du cours pour le message d'erreur
            FILE *f_course = fopen("courses.txt", "r");
            char course_name[30] = "Course";
            if (f_course) {
                course c;
                while (fscanf(f_course, "%s %s %s %s %d %d %d",
                             c.id, c.name, c.type, c.center,
                             &c.time, &c.equipment, &c.capacity) != EOF) {
                    if (strcmp(c.id, course_id) == 0) {
                        strcpy(course_name, c.name);
                        break;
                    }
                }
                fclose(f_course);
            }
            
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "❌ Sorry, course '%s' (ID: %s) is already full!\n\n"
                    "No available spots for this course.",
                    course_name, course_id);
            
            // Afficher un message d'erreur pour ce cours spécifique
            show_popup_message(window, "Warning", error_msg, GTK_MESSAGE_WARNING);
            full_courses++;
            
            iter = iter->next;
            continue;
        }
        
        // Obtenir le téléphone du membre
        char phone[15] = "00000000";
        FILE *f_member = fopen("members.txt", "r");
        if (f_member) {
            char id[30], prenom[30], nom[30], center[30], phone_tmp[30];
            int d, m, y, status;
            while (fscanf(f_member, "%s %s %s %d %d %d %s %s %d", 
                         id, prenom, nom, &d, &m, &y, center, phone_tmp, &status) != EOF) {
                if (strcmp(id, member_id) == 0) {
                    strncpy(phone, phone_tmp, sizeof(phone)-1);
                    break;
                }
            }
            fclose(f_member);
        }
        
        // Créer la réservation
        reservation r;
        strncpy(r.id_member, member_id, sizeof(r.id_member)-1);
        strncpy(r.name_member, member_name, sizeof(r.name_member)-1);
        strncpy(r.phone_number, phone, sizeof(r.phone_number)-1);
        r.date.jour = day;
        r.date.mois = month;
        r.date.annee = year;
        strncpy(r.id_cours, course_id, sizeof(r.id_cours)-1);
        
        // TENTER la réservation
        int result = make_reservation("reservations.txt", r);
        
        if (result == 1) {
            success_count++;
            
            // Envoyer email si réussi
            char member_email[100];
            snprintf(member_email, sizeof(member_email), "%s@topgym.com", member_id);
            
            // Obtenir les détails du cours pour l'email
            FILE *f_course = fopen("courses.txt", "r");
            char course_name[30] = "Course";
            char course_center[30] = "Center";
            char time_str[20] = "Unknown";
            if (f_course) {
                course c;
                while (fscanf(f_course, "%s %s %s %s %d %d %d",
                             c.id, c.name, c.type, c.center,
                             &c.time, &c.equipment, &c.capacity) != EOF) {
                    if (strcmp(c.id, course_id) == 0) {
                        strcpy(course_name, c.name);
                        strcpy(course_center, c.center);
                        switch(c.time) {
                            case 1: strcpy(time_str, "Morning"); break;
                            case 2: strcpy(time_str, "Afternoon"); break;
                            case 3: strcpy(time_str, "Night"); break;
                            default: snprintf(time_str, sizeof(time_str), "%d", c.time);
                        }
                        break;
                    }
                }
                fclose(f_course);
            }
            
            char reservation_date[50];
            snprintf(reservation_date, sizeof(reservation_date), "%02d/%02d/%04d", day, month, year);
            
            send_email_to_member(member_email, member_name,
                                course_id, course_name,
                                course_center, time_str, reservation_date);
            
            // MISE À JOUR IMMÉDIATE : Recharger la treeview après chaque succès
            load_courses_to_treeview_selection(treeview_choose);
            
        } else if (result == 0) {
            // L'erreur est déjà affichée par make_reservation
            // Mais on peut vérifier le type d'erreur
            FILE *f_res = fopen("reservations.txt", "r");
            if (f_res) {
                char line[256];
                int member_has_reservation = 0;
                while (fgets(line, sizeof(line), f_res)) {
                    char temp_member[30], temp_course[30];
                    if (sscanf(line, "%s %*s %*s %*d %*d %*d %s", 
                               temp_member, temp_course) == 2) {
                        if (strcmp(temp_member, member_id) == 0 && 
                            strcmp(temp_course, course_id) == 0) {
                            member_has_reservation = 1;
                            break;
                        }
                    }
                }
                fclose(f_res);
                
                if (member_has_reservation) {
                    already_reserved++;
                }
            }
        }
        
        iter = iter->next;
    }
    
    // Afficher le résumé final
    char summary_msg[512];
    if (success_count > 0) {
        snprintf(summary_msg, sizeof(summary_msg),
                "✅ %d reservation(s) successful!\n\n"
                "Member: %s (ID: %s)\n"
                "Date: %02d/%02d/%04d\n\n"
                "%d course(s) were full\n"
                "%d course(s) already reserved by this member\n\n"
                "Confirmation emails sent for successful reservations.",
                success_count, member_name, member_id, day, month, year,
                full_courses, already_reserved);
        
        show_popup_message(window, "Success", summary_msg, GTK_MESSAGE_INFO);
        
        // Effacer le champ
        gtk_entry_set_text(GTK_ENTRY(entry_member_id), "");
        
    } else {
        snprintf(summary_msg, sizeof(summary_msg),
                "❌ No reservations were made.\n\n"
                "Reasons:\n"
                "- %d course(s) were full\n"
                "- %d course(s) already reserved by this member\n"
                "- %d course(s) failed for other reasons",
                full_courses, already_reserved,
                g_list_length(selected_ids) - full_courses - already_reserved);
        
        show_popup_message(window, "Warning", summary_msg, GTK_MESSAGE_WARNING);
    }
    
    // Libérer la mémoire
    g_list_free_full(selected_ids, g_free);
}

// =========================================================
// FONCTIONS POUR ÉQUIPEMENTS
// =========================================================

// Fonction pour afficher les équipements dans le treeview
void display_equipments_in_treeview(GtkWidget *treeview, Equipment *equipments, int count) {
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Supprimer les anciennes colonnes
    while (gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0) != NULL) {
        column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0);
        gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), column);
    }
    
    // Créer le modèle
    store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
    
    // Ajouter les données
    for (int i = 0; i < count; i++) {
        char date[20];
        sprintf(date, "%02d/%02d/%04d", equipments[i].day, equipments[i].month, equipments[i].year);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, equipments[i].id,
                          1, equipments[i].name,
                          2, equipments[i].centre,
                          3, equipments[i].type,
                          4, equipments[i].quantity,
                          5, date,
                          6, equipments[i].state,
                          -1);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Créer les colonnes
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Centre", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Quantity", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("State", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

// Fonction pour vider les champs
void clear_entries_admin(GtkWidget *window) {
    GtkWidget *entry;
    
    entry = lookup_widget(window, "entry_id");
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    
    entry = lookup_widget(window, "entry_name");
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    
    entry = lookup_widget(window, "entry_centre");
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    
    entry = lookup_widget(window, "entry_type");
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    
    strcpy(current_id, "");
    strcpy(current_name, "");
    strcpy(current_centre, "");
    strcpy(current_type, "");
}

// =========================================================
// COMPLETE LOGIN FUNCTION FOR ALL USERS
// =========================================================
void on_buttonlogin_clicked(GtkButton *button, gpointer user_data)
{
    printf("\n=== LOGIN ATTEMPT ===\n");
    
    // Get the login window
    GtkWidget *login_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    // Get the username and password entries
    GtkWidget *entry_username = lookup_widget(login_window, "entryiduser");
    GtkWidget *entry_password = lookup_widget(login_window, "entrypassworduser");
    
    if (!entry_username || !entry_password) {
        printf("ERROR: Login fields not found!\n");
        return;
    }
    
    // Get the text from entries
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const char *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    
    printf("Username: '%s', Password: '%s'\n", username, password);
    
    // Check against all users
    int authenticated = 0;
    UserCredentials *matched_user = NULL;
    
    for (int i = 0; users[i].username != NULL; i++) {
        if (strcmp(username, users[i].username) == 0 && 
            strcmp(password, users[i].password) == 0) {
            authenticated = 1;
            matched_user = &users[i];
            printf("Matched: %s - %s\n", username, users[i].description);
            break;
        }
    }
    
    if (authenticated && matched_user) {
        printf("SUCCESS: Opening %s window\n", matched_user->description);
        
        // Create the appropriate window
        GtkWidget *target_window = matched_user->create_window_func();
        
        if (target_window) {
            // Show the new window
            gtk_widget_show_all(target_window);
            
            // Hide the login window
            gtk_widget_hide(login_window);
            
            // Optional: Show welcome message
            char welcome_msg[256];
            snprintf(welcome_msg, sizeof(welcome_msg), 
                    "Welcome %s!\n%s system opened successfully.", 
                    username, matched_user->description);
            
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(target_window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "%s", welcome_msg);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            
            printf("Window opened successfully\n");
        } else {
            printf("ERROR: Failed to create window\n");
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Error: Failed to open %s window!", matched_user->description);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    } else {
        // Login failed
        printf("FAILED: Invalid credentials\n");
        
        // Build helpful error message
        char error_msg[512];
        if (strcmp(username, "raedcoach") == 0) {
            snprintf(error_msg, sizeof(error_msg),
                "Wrong password for raedcoach!\n\n"
                "Available passwords for raedcoach:\n"
                "• raed123 - Manage Coach\n"
                "• raed2025 - Reserve Material");
        } else if (strcmp(username, "admin") == 0) {
            snprintf(error_msg, sizeof(error_msg),
                "Wrong password for admin!\n\n"
                "Password: admin123");
        } else if (strcmp(username, "trainer") == 0) {
            snprintf(error_msg, sizeof(error_msg),
                "Wrong password for trainer!\n\n"
                "Password: trainer123");
        } else {
            // Unknown username
            snprintf(error_msg, sizeof(error_msg),
                "Invalid username!\n\n"
                "Available users:\n"
                "• raedcoach (raed123 / raed2025)\n"
                "• admin (admin123)\n"
                "• trainer (trainer123)");
        }
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "%s", error_msg);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    printf("=== LOGIN PROCESS END ===\n\n");
}

// =========================================================
// FONCTIONS POUR LE MENU DE SÉLECTION
// =========================================================

// Callbacks pour le menu de sélection
void on_btn_select_admin_clicked(GtkButton *button, gpointer user_data) {
    strcpy(selected_role, "admin");
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_hide(window);
    
    GtkWidget *login = create_login();
    gtk_widget_show(login);
    current_window = login;
}

void on_btn_select_trainer_clicked(GtkButton *button, gpointer user_data) {
    strcpy(selected_role, "trainer");
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_hide(window);
    
    GtkWidget *login = create_login();
    gtk_widget_show(login);
    current_window = login;
}

// =========================================================
// FONCTIONS POUR "REMEMBER ME" CHECKBOX
// =========================================================

void on_checkbtnrememberme_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    remember_me = gtk_toggle_button_get_active(togglebutton);
}

// =========================================================
// GENERIC LOGOUT FUNCTION (used by all logout buttons)
// =========================================================

static void logout_to_login(GtkWidget *current_window)
{
    if (!current_window) return;
    
    // Clear authentication state
    remember_me = 0;
    memset(selected_role, 0, sizeof(selected_role));
    memset(current_user_id, 0, sizeof(current_user_id));
    
    // Destroy current window
    gtk_widget_destroy(current_window);
    
    // Show login window again
    GtkWidget *login_window = create_login();
    if (login_window) {
        gtk_widget_show_all(login_window);
    }
}

// =========================================================
// FONCTIONS POUR LOGOUT
// =========================================================

void on_buttonlogoutadmin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    logout_to_login(window);
}

void on_buttonlogouttrtainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    logout_to_login(window);
}

void on_logoutraed_manage_coach_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    logout_to_login(window);
}

void on_logoutraed_reserve_material_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    logout_to_login(window);
}

// =========================================================
// FONCTIONS POUR L'ADMIN - ÉQUIPEMENTS
// =========================================================

void on_entry_id_changed(GtkEditable *editable, gpointer user_data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    strcpy(current_id, text);
}

void on_entry_name_changed(GtkEditable *editable, gpointer user_data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    strcpy(current_name, text);
}

void on_entry_centre_changed(GtkEditable *editable, gpointer user_data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    strcpy(current_centre, text);
}

void on_entry_type_changed(GtkEditable *editable, gpointer user_data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    strcpy(current_type, text);
}

void on_entrysearch_changed(GtkEditable *editable, gpointer user_data) {
    // La recherche se fait lors du clic sur le bouton search
}

void on_checkbuttonname_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    search_by_name_eq = gtk_toggle_button_get_active(togglebutton);
}

void on_checkbuttoncentre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    search_by_centre_eq = gtk_toggle_button_get_active(togglebutton);
}

void on_radiobtngood_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton)) {
        strcpy(current_state, "Good");
    }
}

void on_radiobuttonbroken_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton)) {
        strcpy(current_state, "Broken");
    }
}

void on_comboboxquantite_changed(GtkComboBox *combobox, gpointer user_data) {
    char *text = gtk_combo_box_get_active_text(combobox);
    if (text != NULL) {
        current_quantity = atoi(text);
        g_free(text);
    }
}

void on_spinbutton1_change_value(GtkSpinButton *spinbutton, gpointer user_data) {
    current_day = gtk_spin_button_get_value_as_int(spinbutton);
}

void on_spinbutton2_value_changed(GtkSpinButton *spinbutton, gpointer user_data) {
    current_month = gtk_spin_button_get_value_as_int(spinbutton);
}

void on_spinbutton3_change_value(GtkSpinButton *spinbutton, gpointer user_data) {
    current_year = gtk_spin_button_get_value_as_int(spinbutton);
}

void on_btn_add_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    if (strlen(current_id) == 0 || strlen(current_name) == 0 || 
        strlen(current_centre) == 0 || strlen(current_type) == 0) {
        show_popup_message(window, "Error", "Please fill all fields!", GTK_MESSAGE_ERROR);
        return;
    }
    
    Equipment eq;
    strcpy(eq.id, current_id);
    strcpy(eq.name, current_name);
    strcpy(eq.centre, current_centre);
    strcpy(eq.type, current_type);
    eq.quantity = current_quantity;
    eq.day = current_day;
    eq.month = current_month;
    eq.year = current_year;
    strcpy(eq.state, current_state);
    
    if (add_equipment(eq)) {
        show_popup_message(window, "Success", "Equipment added successfully!", GTK_MESSAGE_INFO);
        clear_entries_admin(window);
        
        // Rafraîchir le treeview
        GtkWidget *treeview = lookup_widget(window, "treeviewajout");
        int count;
        Equipment *all_eq = get_all_equipments(&count);
        if (all_eq != NULL) {
            display_equipments_in_treeview(treeview, all_eq, count);
            free(all_eq);
        }
    } else {
        show_popup_message(window, "Error", "Could not add equipment!", GTK_MESSAGE_ERROR);
    }
}



void on_btn_modifie_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    if (strlen(current_id) == 0) {
        show_popup_message(window, "Error", "Please select or enter an equipment ID!", GTK_MESSAGE_ERROR);
        return;
    }
    
    Equipment eq;
    strcpy(eq.id, current_id);
    strcpy(eq.name, current_name);
    strcpy(eq.centre, current_centre);
    strcpy(eq.type, current_type);
    eq.quantity = current_quantity;
    eq.day = current_day;
    eq.month = current_month;
    eq.year = current_year;
    strcpy(eq.state, current_state);
    
    if (modify_equipment(current_id, eq)) {
        show_popup_message(window, "Success", "Equipment modified successfully!", GTK_MESSAGE_INFO);
        
        // Rafraîchir le treeview
        GtkWidget *treeview = lookup_widget(window, "treeviewajout");
        int count;
        Equipment *all_eq = get_all_equipments(&count);
        if (all_eq != NULL) {
            display_equipments_in_treeview(treeview, all_eq, count);
            free(all_eq);
        }
    } else {
        show_popup_message(window, "Error", "Equipment not found!", GTK_MESSAGE_ERROR);
    }
}

void on_btn_delete_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    if (strlen(current_id) == 0) {
        show_popup_message(window, "Error", "Please select or enter an equipment ID!", GTK_MESSAGE_ERROR);
        return;
    }
    
    if (delete_equipment(current_id)) {
        show_popup_message(window, "Success", "Equipment deleted successfully!", GTK_MESSAGE_INFO);
        clear_entries_admin(window);
        
        // Rafraîchir le treeview
        GtkWidget *treeview = lookup_widget(window, "treeviewajout");
        int count;
        Equipment *all_eq = get_all_equipments(&count);
        if (all_eq != NULL) {
            display_equipments_in_treeview(treeview, all_eq, count);
            free(all_eq);
        }
    } else {
        show_popup_message(window, "Error", "Equipment not found!", GTK_MESSAGE_ERROR);
    }
}

void on_btn_search_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry_search = lookup_widget(window, "entrysearch");
    GtkWidget *treeview = lookup_widget(window, "treeviewajout");
    
    const char *search_text = gtk_entry_get_text(GTK_ENTRY(entry_search));
    
    if (strlen(search_text) == 0) {
        show_popup_message(window, "Error", "Please enter a search term!", GTK_MESSAGE_ERROR);
        return;
    }
    
    Equipment *results = NULL;
    int count = 0;
    int total_count = 0;
    
    if (search_by_name_eq && search_by_centre_eq) {
        // Recherche par nom ET centre
        FILE *f = fopen("equipments.txt", "r");
        if (f != NULL) {
            Equipment eq;
            while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                          eq.id, eq.name, eq.centre, eq.type,
                          &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
                if (strstr(eq.name, search_text) != NULL && strstr(eq.centre, search_text) != NULL) {
                    count++;
                    results = (Equipment*)realloc(results, count * sizeof(Equipment));
                    results[count-1] = eq;
                }
            }
            fclose(f);
        }
    } else if (search_by_name_eq) {
        // Recherche par nom uniquement
        FILE *f = fopen("equipments.txt", "r");
        if (f != NULL) {
            Equipment eq;
            while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                          eq.id, eq.name, eq.centre, eq.type,
                          &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
                if (strstr(eq.name, search_text) != NULL) {
                    count++;
                    results = (Equipment*)realloc(results, count * sizeof(Equipment));
                    results[count-1] = eq;
                }
            }
            fclose(f);
        }
    } else if (search_by_centre_eq) {
        // Recherche par centre uniquement
        FILE *f = fopen("equipments.txt", "r");
        if (f != NULL) {
            Equipment eq;
            while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                          eq.id, eq.name, eq.centre, eq.type,
                          &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
                if (strstr(eq.centre, search_text) != NULL) {
                    count++;
                    results = (Equipment*)realloc(results, count * sizeof(Equipment));
                    results[count-1] = eq;
                }
            }
            fclose(f);
        }
    } else {
        show_popup_message(window, "Error", "Please select Name or Centre checkbox!", GTK_MESSAGE_ERROR);
        return;
    }
    
    if (results != NULL && count > 0) {
        display_equipments_in_treeview(treeview, results, count);
        free(results);
        
        char msg[100];
        sprintf(msg, "%d equipment(s) found!", count);
        show_popup_message(window, "Info", msg, GTK_MESSAGE_INFO);
    } else {
        show_popup_message(window, "Info", "No equipment found!", GTK_MESSAGE_INFO);
    }
}

void on_btn_Display_All_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "treeviewajout");
    
    int count;
    Equipment *all_eq = get_all_equipments(&count);
    
    if (all_eq != NULL && count > 0) {
        display_equipments_in_treeview(treeview, all_eq, count);
        free(all_eq);
        
        char msg[100];
        sprintf(msg, "%d equipment(s) displayed!", count);
        show_popup_message(window, "Info", msg, GTK_MESSAGE_INFO);
    } else {
        show_popup_message(window, "Info", "No equipments found!", GTK_MESSAGE_INFO);
    }
}

// =========================================================
// FONCTIONS POUR TREEVIEW ÉQUIPEMENTS
// =========================================================

void on_treeviewajout_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                    GtkTreeViewColumn *column, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *id, *name, *centre, *type, *state, *date;
    int quantity;
    
    model = gtk_tree_view_get_model(treeview);
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter,
                          0, &id,
                          1, &name,
                          2, &centre,
                          3, &type,
                          4, &quantity,
                          5, &date,
                          6, &state,
                          -1);
        
        GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
        
        // Remplir les champs avec les données sélectionnées
        GtkWidget *entry = lookup_widget(window, "entry_id");
        gtk_entry_set_text(GTK_ENTRY(entry), id);
        strcpy(current_id, id);
        
        entry = lookup_widget(window, "entry_name");
        gtk_entry_set_text(GTK_ENTRY(entry), name);
        strcpy(current_name, name);
        
        entry = lookup_widget(window, "entry_centre");
        gtk_entry_set_text(GTK_ENTRY(entry), centre);
        strcpy(current_centre, centre);
        
        entry = lookup_widget(window, "entry_type");
        gtk_entry_set_text(GTK_ENTRY(entry), type);
        strcpy(current_type, type);
        
        // Parse date
        int d, m, y;
        sscanf(date, "%d/%d/%d", &d, &m, &y);
        
        GtkWidget *spin = lookup_widget(window, "spinbuttondatejours");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), d);
        current_day = d;
        
        spin = lookup_widget(window, "spinbuttondatemoins");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), m);
        current_month = m;
        
        spin = lookup_widget(window, "spinbuttondateanne");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), y);
        current_year = y;
        
        // Set quantity
        GtkWidget *combo = lookup_widget(window, "comboboxquantite");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), quantity - 1);
        current_quantity = quantity;
        
        // Set state radio button
        if (strcmp(state, "Good") == 0) {
            GtkWidget *radio = lookup_widget(window, "radiobtngood");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            strcpy(current_state, "Good");
        } else {
            GtkWidget *radio = lookup_widget(window, "radiobuttonbroken");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            strcpy(current_state, "Broken");
        }
        
        g_free(id);
        g_free(name);
        g_free(centre);
        g_free(type);
        g_free(state);
        g_free(date);
    }
}

// =========================================================
// FONCTIONS POUR LE TRAINER
// =========================================================

void on_affichereqpdispotrainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "treeviewafficherdispo");
    
    int count;
    Equipment *available = get_available_equipments(&count);
    
    if (available != NULL && count > 0) {
        display_equipments_in_treeview(treeview, available, count);
        free(available);
        
        char msg[100];
        sprintf(msg, "%d available equipment(s) loaded!", count);
        show_popup_message(window, "Info", msg, GTK_MESSAGE_INFO);
    } else {
        show_popup_message(window, "Info", "No available equipments!", GTK_MESSAGE_INFO);
    }
}

void on_togglebuttonreservertrainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry = lookup_widget(window, "entryidequipmenttrainer");
    
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(id) == 0) {
        show_popup_message(window, "Error", "Please enter an equipment ID!", GTK_MESSAGE_ERROR);
        return;
    }
    
    Equipment *eq = search_equipment_by_id((char*)id);
    
    if (eq != NULL) {
        if (strcmp(eq->state, "Good") == 0) {
            // Get date and time from spinbuttons
            GtkWidget *spin_day = lookup_widget(window, "spinbuttonjrs");
            GtkWidget *spin_month = lookup_widget(window, "spinbuttonmounth");
            GtkWidget *spin_hour = lookup_widget(window, "spinbuttonheurs");
            GtkWidget *spin_min = lookup_widget(window, "spinbuttonmint");
            
            int day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_day));
            int month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_month));
            int hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_hour));
            int min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_min));
            
            char msg[200];
            sprintf(msg, "Equipment '%s' reserved successfully!\nDate: %02d/%02d\nTime: %02d:%02d", 
                    eq->name, day, month, hour, min);
            show_popup_message(window, "Success", msg, GTK_MESSAGE_INFO);
            free(eq);
        } else {
            show_popup_message(window, "Error", "Equipment is broken and cannot be reserved!", GTK_MESSAGE_ERROR);
            free(eq);
        }
    } else {
        show_popup_message(window, "Error", "Equipment not found!", GTK_MESSAGE_ERROR);
    }
}

void on_treeviewafficherdispo_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                           GtkTreeViewColumn *column, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *id;
    
    model = gtk_tree_view_get_model(treeview);
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        
        GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
        GtkWidget *entry = lookup_widget(window, "entryidequipmenttrainer");
        gtk_entry_set_text(GTK_ENTRY(entry), id);
        
        g_free(id);
    }
}

// =========================================================
// FONCTION POUR CANCEL BUTTON
// =========================================================

void on_buttoncancel_clicked(GtkButton *button, gpointer user_data) {
    gtk_main_quit();
}
