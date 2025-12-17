#include "equipment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <time.h>

#define EQUIPMENT_FILE "equipments.txt"
#define USER_FILE "connect.txt"
#define RESERVATION_FILE "reservations.txt"



// Ajouter un équipement
int add_equipment(Equipment eq) {
    FILE *f = fopen(EQUIPMENT_FILE, "a");
    if (f == NULL) {
        return 0;
    }
    
    fprintf(f, "%s|%s|%s|%s|%d|%d|%d|%d|%s\n", 
            eq.id, eq.name, eq.centre, eq.type, 
            eq.quantity, eq.day, eq.month, eq.year, eq.state);
    
    fclose(f);
    return 1;
}

// Modifier un équipement
int modify_equipment(char *id, Equipment new_eq) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    
    if (f == NULL || temp == NULL) {
        if (f) fclose(f);
        if (temp) fclose(temp);
        return 0;
    }
    
    Equipment eq;
    int found = 0;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strcmp(eq.id, id) == 0) {
            fprintf(temp, "%s|%s|%s|%s|%d|%d|%d|%d|%s\n",
                    new_eq.id, new_eq.name, new_eq.centre, new_eq.type,
                    new_eq.quantity, new_eq.day, new_eq.month, new_eq.year, new_eq.state);
            found = 1;
        } else {
            fprintf(temp, "%s|%s|%s|%s|%d|%d|%d|%d|%s\n",
                    eq.id, eq.name, eq.centre, eq.type,
                    eq.quantity, eq.day, eq.month, eq.year, eq.state);
        }
    }
    
    fclose(f);
    fclose(temp);
    
    remove(EQUIPMENT_FILE);
    rename("temp.txt", EQUIPMENT_FILE);
    
    return found;
}

// Supprimer un équipement
int delete_equipment(char *id) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    
    if (f == NULL || temp == NULL) {
        if (f) fclose(f);
        if (temp) fclose(temp);
        return 0;
    }
    
    Equipment eq;
    int found = 0;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strcmp(eq.id, id) != 0) {
            fprintf(temp, "%s|%s|%s|%s|%d|%d|%d|%d|%s\n",
                    eq.id, eq.name, eq.centre, eq.type,
                    eq.quantity, eq.day, eq.month, eq.year, eq.state);
        } else {
            found = 1;
        }
    }
    
    fclose(f);
    fclose(temp);
    
    remove(EQUIPMENT_FILE);
    rename("temp.txt", EQUIPMENT_FILE);
    
    return found;
}

// Rechercher un équipement par ID
Equipment* search_equipment_by_id(char *id) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) return NULL;
    
    Equipment *eq = (Equipment*)malloc(sizeof(Equipment));
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq->id, eq->name, eq->centre, eq->type,
                  &eq->quantity, &eq->day, &eq->month, &eq->year, eq->state) == 9) {
        
        if (strcmp(eq->id, id) == 0) {
            fclose(f);
            return eq;
        }
    }
    
    fclose(f);
    free(eq);
    return NULL;
}

// Rechercher par nom
Equipment* search_equipment_by_name(char *name) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) return NULL;
    
    Equipment *results = NULL;
    int count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strstr(eq.name, name) != NULL) {
            count++;
            results = (Equipment*)realloc(results, count * sizeof(Equipment));
            results[count-1] = eq;
        }
    }
    
    fclose(f);
    return results;
}

// Rechercher par centre
Equipment* search_equipment_by_centre(char *centre) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) return NULL;
    
    Equipment *results = NULL;
    int count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strstr(eq.centre, centre) != NULL) {
            count++;
            results = (Equipment*)realloc(results, count * sizeof(Equipment));
            results[count-1] = eq;
        }
    }
    
    fclose(f);
    return results;
}

// Rechercher par nom et centre
Equipment* search_equipment_by_name_and_centre(char *name, char *centre) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) return NULL;
    
    Equipment *results = NULL;
    int count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strstr(eq.name, name) != NULL && strstr(eq.centre, centre) != NULL) {
            count++;
            results = (Equipment*)realloc(results, count * sizeof(Equipment));
            results[count-1] = eq;
        }
    }
    
    fclose(f);
    return results;
}

// Compter les équipements
int count_equipments() {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) return 0;
    
    int count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        count++;
    }
    
    fclose(f);
    return count;
}

// Obtenir tous les équipements
Equipment* get_all_equipments(int *count) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) {
        *count = 0;
        return NULL;
    }
    
    Equipment *results = NULL;
    *count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        (*count)++;
        results = (Equipment*)realloc(results, (*count) * sizeof(Equipment));
        results[(*count)-1] = eq;
    }
    
    fclose(f);
    return results;
}

// Obtenir les équipements disponibles (Good)
Equipment* get_available_equipments(int *count) {
    FILE *f = fopen(EQUIPMENT_FILE, "r");
    if (f == NULL) {
        *count = 0;
        return NULL;
    }
    
    Equipment *results = NULL;
    *count = 0;
    Equipment eq;
    
    while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^\n]\n",
                  eq.id, eq.name, eq.centre, eq.type,
                  &eq.quantity, &eq.day, &eq.month, &eq.year, eq.state) == 9) {
        
        if (strcmp(eq.state, "Good") == 0) {
            (*count)++;
            results = (Equipment*)realloc(results, (*count) * sizeof(Equipment));
            results[(*count)-1] = eq;
        }
    }
    
    fclose(f);
    return results;
}

// Sauvegarder les credentials de l'utilisateur
int save_user_credentials(User user) {
    FILE *f = fopen(USER_FILE, "w");
    if (f == NULL) return 0;
    
    fprintf(f, "%s|%s|%s\n", user.id, user.password, user.role);
    
    fclose(f);
    return 1;
}

// Charger les credentials de l'utilisateur
User* load_user_credentials() {
    FILE *f = fopen(USER_FILE, "r");
    if (f == NULL) return NULL;
    
    User *user = (User*)malloc(sizeof(User));
    
    if (fscanf(f, "%[^|]|%[^|]|%[^\n]\n", user->id, user->password, user->role) == 3) {
        fclose(f);
        return user;
    }
    
    fclose(f);
    free(user);
    return NULL;
}

// Vérifier un utilisateur
int verify_user(char *id, char *password, char *role) {
    // Vérifier le format de l'ID
    if (strlen(id) < 3) return 0;
    
    char prefix[4];
    strncpy(prefix, id, 3);
    prefix[3] = '\0';
    
    // Admin: 3 premiers caractères = ADM, password 6 caractères
    if (strcmp(prefix, "ADM") == 0) {
        if (strlen(password) == 6) {
            strcpy(role, "admin");
            return 1;
        }
    }
    
    // Trainer: 3 premiers caractères = TRN, password 6 caractères
    if (strcmp(prefix, "TRN") == 0) {
        if (strlen(password) == 6) {
            strcpy(role, "trainer");
            return 1;
        }
    }
    
    return 0;
}

int equipment_already_reserved(char id_eq[], char date[]) {
    FILE *f = fopen("reservations.txt", "r");
    if (!f) return 0;

    char fid[20], ftrainer[20], fdate[20], fstate[20];

    while (fscanf(f, "%[^;];%[^;];%[^;];%s\n",
                  fid, ftrainer, fdate, fstate) != EOF) {
        if (strcmp(fid, id_eq) == 0 &&
            strcmp(fdate, date) == 0 &&
            strcmp(fstate, "RESERVED") == 0) {
            fclose(f);
            return 1; // ❌ déjà réservé
        }
    }

    fclose(f);
    return 0; // ✅ disponible
}

void show_trainer_history(char trainer_id[]) {
    FILE *f = fopen("reservations.txt", "r");
    if (!f) return;

    char fid[20], ftrainer[20], fdate[20], fstate[20];
    char buffer[1024] = "";

    while (fscanf(f, "%[^;];%[^;];%[^;];%s\n",
                  fid, ftrainer, fdate, fstate) != EOF) {
        if (strcmp(ftrainer, trainer_id) == 0) {
            strcat(buffer, fid);
            strcat(buffer, " | ");
            strcat(buffer, fdate);
            strcat(buffer, " | ");
            strcat(buffer, fstate);
            strcat(buffer, "\n");
        }
    }

    fclose(f);

    if (strlen(buffer) == 0)
        strcpy(buffer, "No reservation history.");

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Reservation History:\n%s",
        buffer
    );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void save_reservation(char eq_id[], char trainer_id[], char date[]) {
    FILE *f = fopen("reservations.txt", "a");
    if (!f) return;

    fprintf(f, "%s;%s;%s;RESERVED\n",
            eq_id, trainer_id, date);

    fclose(f);
}

void save_login(char id[], char role[], char password[]) {
    FILE *f = fopen("connect.txt", "a");
    if (!f) return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(f, "%s;%s;%s;%04d-%02d-%02d %02d:%02d:%02d\n",
            id, role, password,
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);

    fclose(f);
}

int trainer_logged_before(char trainer_id[]) {
    FILE *f = fopen("connect.txt", "r");
    if (!f) return 0;

    char id[20], role[20], pass[20], datetime[40];

    while (fscanf(f, "%[^;];%[^;];%[^;];%[^\n]\n",
                  id, role, pass, datetime) != EOF) {
        if (strcmp(id, trainer_id) == 0 &&
            strcmp(role, "trainer") == 0) {
            fclose(f);
            return 1; // déjà connecté avant
        }
    }

    fclose(f);
    return 0;
}

// Ajoutez ces fonctions pour résoudre les erreurs restantes
int check_trainer_credentials(const char *id, const char *password) {
    char role[20];
    // Appeler verify_user avec des casts car les paramètres sont const char*
    return verify_user((char*)id, (char*)password, role) && strcmp(role, "trainer") == 0;
}

int check_admin_credentials(const char *id, const char *password) {
    char role[20];
    // Appeler verify_user avec des casts car les paramètres sont const char*
    return verify_user((char*)id, (char*)password, role) && strcmp(role, "admin") == 0;
}
