#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include "coach.h"

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


#endif


void
on_treeview13_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_button39_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_button40_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_checkbutton_center_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_checkbutton_name_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_treeviewajout_row_activated         (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_checkbuttonname_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_checkbuttoncentre_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_comboboxquantite_changed            (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_buttonlogoutadmin_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobtngood_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_treeviewafficherdispo_row_activated (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_buttonlogouttrtainer_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_btdelete_b_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_btsearch_b_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_btshowall_b_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_btadd_b_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmodify_b_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeview1_b_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_treeview2_b_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_btshow_b_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_btshoose_b_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeviewchoose_y_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_loadcourses_y_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_btchoose_y_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeviewcourse_y_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_btsearch_y_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_btdelete_y_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_load_y_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_btadd_y_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmodify_y_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobuttonMorning_y_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonafternoon_y_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonnight_y_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTreadmill_y_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonsquatmachine_y_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonNo_y_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_entrysearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_treeviewajout_row_activated         (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_checkbuttonname_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_bttaddcoach_a_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_btassigncoach_a_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmanageequipement_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmanagecourses_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmanagecoachs_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_btmanagememebers_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_btjoincourse_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_bttreserveequipement_a_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_btparticipatecoach_a_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_btreserveprivatecoach_a_clicked     (GtkButton       *button,
                                        gpointer         user_data);
