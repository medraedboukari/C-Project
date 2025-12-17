#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include "equipment.h"
#include "coach.h"

// =========================================================
// VARIABLES GLOBALES
// =========================================================
extern GtkWidget *current_window;
extern char selected_role[20];
extern char current_user_id[20];
extern int gender; // 1 = Male, 2 = Female

// Variables pour équipements
extern char current_id[20];
extern char current_name[50];
extern char current_centre[50];
extern char current_type[50];
extern int current_quantity;
extern int current_day;
extern int current_month;
extern int current_year;
extern char current_state[20];
extern int search_by_name;
extern int search_by_centre;
extern int remember_me;

// =========================================================
// FONCTIONS UTILITAIRES
// =========================================================
void show_popup_message(GtkWidget *parent, const char *title, const char *message, GtkMessageType type);
void show_message(GtkWidget *parent, const char *message, GtkMessageType type);
void show_error(const char *message);
gboolean show_confirmation_dialog(GtkWidget *parent, const char *title, const char *message);
int is_number(const char *str);
int is_valid_id(const char *str);
int is_valid_phone(const char *str);
int is_letters(const char *str);
gboolean clear_label(gpointer data);
void debug_coach_file();
void debug_assignment_file();
int send_email_to_admin(const char *subject, const char *body, const char *action_type);
int send_email_to_coach(const char *coach_email, const char *subject, const char *body);
int send_email(const char *subject, const char *body);
void update_coach_in_file(int coach_id, int column_index, const char *new_value);
void display_equipments_in_treeview(GtkWidget *treeview, Equipment *equipments, int count);
void clear_entries_admin(GtkWidget *window);

// =========================================================
// FONCTIONS POUR TREEVIEW COACH
// =========================================================
void on_cell_toggled_coach(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void on_cell_edited_coach(GtkCellRendererText *cell, gchar *path_str, gchar *new_text, gpointer data);
void create_columns_with_checkbox_coach(GtkTreeView *treeview, const char **titles, int num_columns);
GList* get_selected_coach_ids(GtkWidget *treeview);
void loadCoaches(GtkTreeView *treeview, const char *filename);
void loadCourses(GtkTreeView *treeview, const char *filename);
void create_columns(GtkTreeView *treeview);
void refresh_tree(GtkWidget *treeview);
void refresh_course_tree(GtkWidget *treeview);
int update_course_capacity(const char *course_id, int change_amount);
int count_assigned_coaches(const char *course_id);

// =========================================================
// CALLBACKS POUR LE MENU DE SÉLECTION
// =========================================================
void on_btn_select_admin_clicked(GtkButton *button, gpointer user_data);
void on_btn_select_trainer_clicked(GtkButton *button, gpointer user_data);

// =========================================================
// CALLBACKS POUR LE LOGIN
// =========================================================
void on_buttonlogin_clicked(GtkButton *button, gpointer user_data);
void on_buttoncancel_clicked(GtkButton *button, gpointer user_data);
void on_checkbtnrememberme_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// =========================================================
// CALLBACKS POUR L'ADMIN - ÉQUIPEMENTS
// =========================================================
void on_btn_add_clicked(GtkButton *button, gpointer user_data);
void on_btn_modifie_clicked(GtkButton *button, gpointer user_data);
void on_btn_delete_clicked(GtkButton *button, gpointer user_data);
void on_btn_search_clicked(GtkButton *button, gpointer user_data);
void on_btn_Display_All_clicked(GtkButton *button, gpointer user_data);
void on_buttonlogoutadmin_clicked(GtkButton *button, gpointer user_data);

void on_entrysearch_changed(GtkEditable *editable, gpointer user_data);
void on_entry_id_changed(GtkEditable *editable, gpointer user_data);
void on_entry_name_changed(GtkEditable *editable, gpointer user_data);
void on_entry_centre_changed(GtkEditable *editable, gpointer user_data);
void on_entry_type_changed(GtkEditable *editable, gpointer user_data);

void on_checkbuttonname_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_checkbuttoncentre_toggled(GtkToggleButton *togglebutton, gpointer user_data);

void on_comboboxquantite_changed(GtkComboBox *combobox, gpointer user_data);

void on_radiobuttonbroken_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobtngood_toggled(GtkToggleButton *togglebutton, gpointer user_data);

void on_spinbutton1_change_value(GtkSpinButton *spinbutton, gpointer user_data);
void on_spinbutton2_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_spinbutton3_change_value(GtkSpinButton *spinbutton, gpointer user_data);

void on_treeviewajout_row_activated(GtkTreeView *treeview, GtkTreePath *path, 
                                    GtkTreeViewColumn *column, gpointer user_data);

// =========================================================
// CALLBACKS POUR LE TRAINER
// =========================================================
void on_affichereqpdispotrainer_clicked(GtkButton *button, gpointer user_data);
void on_togglebuttonreservertrainer_clicked(GtkButton *button, gpointer user_data);
void on_buttonlogouttrtainer_clicked(GtkButton *button, gpointer user_data);
void on_treeviewafficherdispo_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                           GtkTreeViewColumn *column, gpointer user_data);

// =========================================================
// CALLBACKS POUR LES COACHS
// =========================================================
// ADD
void on_btadd_clicked(GtkButton *button, gpointer user_data);

// MODIFY
void on_btmodify_clicked(GtkButton *button, gpointer user_data);

// DELETE
void on_btdelete_clicked(GtkButton *button, gpointer user_data);

// SEARCH
void on_btsearch_clicked(GtkButton *button, gpointer user_data);

// RADIO BUTTONS
void on_radiobuttonman_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobuttonwomen_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// TREEVIEW EVENTS
void on_treeviewmanage_row_activated(GtkTreeView *treeview,
                                     GtkTreePath *path,
                                     GtkTreeViewColumn *column,
                                     gpointer user_data);

// REFRESH BUTTON
void on_refresh_clicked(GtkButton *button, gpointer user_data);

// AUTRES FONCTIONS
void on_checkbutton_center_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_checkbutton_name_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_treeview_2_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                 GtkTreeViewColumn *column, gpointer user_data);
void on_refresh_2_clicked(GtkButton *button, gpointer user_data);
void on_btchoose_clicked(GtkButton *button, gpointer user_data);

#endif // CALLBACKS_H
