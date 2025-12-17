#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <gtk/gtk.h>

// Structure pour représenter un équipement
typedef struct {
    char id[20];
    char name[50];
    char centre[50];
    char type[50];
    int quantity;
    int day;
    int month;
    int year;
    char state[20]; // "Good" ou "Broken"
} Equipment;

// Structure pour représenter un utilisateur
typedef struct {
    char id[20];
    char password[20];
    char role[20]; // "admin" ou "trainer"
} User;

// Fonctions pour la gestion des équipements
int add_equipment(Equipment eq);
int modify_equipment(char *id, Equipment new_eq);
int delete_equipment(char *id);
Equipment* search_equipment_by_id(char *id);
Equipment* search_equipment_by_name(char *name);
Equipment* search_equipment_by_centre(char *centre);
Equipment* search_equipment_by_name_and_centre(char *name, char *centre);
int count_equipments();
Equipment* get_all_equipments(int *count);
Equipment* get_available_equipments(int *count);

// Fonctions pour la gestion des utilisateurs
int save_user_credentials(User user);
User* load_user_credentials();
int verify_user(char *id, char *password, char *role);

// Fonctions utilitaires
void show_message(GtkWidget *parent, const char *message, GtkMessageType type);

#endif // EQUIPMENT_H
