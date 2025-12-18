#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "course.h"



int add(char *filename, course c) {
    course temp;
    FILE *f = fopen(filename, "r");

    if (f != NULL) {
        while (fscanf(f, "%s %s %s %s %d %d %d",
                      temp.id, temp.name, temp.type, temp.center,
                      &temp.time, &temp.equipment, &temp.capacity) != EOF) {
            if (strcmp(temp.id, c.id) == 0) {
                fclose(f);
                printf("Erreur : ID du cours déjà existant (%s)\n", c.id);
                return 0; 
            }
        }
        fclose(f);
    }

    
    f = fopen(filename, "a");
    if (f != NULL) {
        fprintf(f, "%s %s %s %s %d %d %d\n",
                c.id, c.name, c.type, c.center,
                c.time, c.equipment, c.capacity);
        fclose(f);
        return 1;
    }
    return 0;
}


int modify(char *filename, char *id, course nouv) {
    int tr = 0;
    course c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");

    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%s %s %s %s %d %d %d",
                      c.id, c.name, c.type, c.center,
                      &c.time, &c.equipment, &c.capacity) != EOF) {
            if (strcmp(c.id, id) == 0) {
                fprintf(f2, "%s %s %s %s %d %d %d\n",
                        nouv.id, nouv.name, nouv.type, nouv.center,
                        nouv.time, nouv.equipment, nouv.capacity);
                tr = 1;
            } else {
                fprintf(f2, "%s %s %s %s %d %d %d\n",
                        c.id, c.name, c.type, c.center,
                        c.time, c.equipment, c.capacity);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv.txt", filename);
    }
    return tr;
}



int Delete(char *filename, char *id) {
    int tr = 0;
    course c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");

    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%s %s %s %s %d %d %d",
                      c.id, c.name, c.type, c.center,
                      &c.time, &c.equipment, &c.capacity) != EOF) {
            if (strcmp(c.id, id) != 0) {
                fprintf(f2, "%s %s %s %s %d %d %d\n",
                        c.id, c.name, c.type, c.center,
                        c.time, c.equipment, c.capacity);
            } else {
                tr = 1;
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv.txt", filename);
    }
    return tr;
}

course search(char *filename, char *name, char *center) {
    course c;
    int found = 0;
    FILE *f = fopen(filename, "r");

    if (f != NULL) {
        while (!found && fscanf(f, "%s %s %s %s %d %d %d",
                                 c.id, c.name, c.type, c.center,
                                 &c.time, &c.equipment, &c.capacity) != EOF) {
            int match_name = 1;
            int match_center = 1;
            
            // Recherche par nom
            if (name != NULL && strlen(name) > 0) {
                match_name = (strcmp(c.name, name) == 0);
            }
            
            // Recherche par centre
            if (center != NULL && strlen(center) > 0) {
                match_center = (strcmp(c.center, center) == 0);
            }
            
            if (match_name && match_center)
                found = 1;
        }
        fclose(f);
    }

    if (!found)
        strcpy(c.id, "-1");
    return c;
}


int search_all_courses(char *filename, char *name, char *center, course results[], int max_results) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f != NULL) {
        course c;
        while (count < max_results && fscanf(f, "%s %s %s %s %d %d %d",
                                             c.id, c.name, c.type, c.center,
                                             &c.time, &c.equipment, &c.capacity) != EOF) {
            
            // Normaliser les chaînes
            normalize_string(c.name);
            normalize_string(c.center);
            
            // Normaliser les paramètres de recherche
            char norm_name[30] = "";
            char norm_center[30] = "";
            
            if (name && strlen(name) > 0) {
                strncpy(norm_name, name, sizeof(norm_name) - 1);
                normalize_string(norm_name);
            }
            
            if (center && strlen(center) > 0) {
                strncpy(norm_center, center, sizeof(norm_center) - 1);
                normalize_string(norm_center);
            }
            
            int match_name = 1;
            int match_center = 1;
            
            // Comparer nom si spécifié
            if (name && strlen(name) > 0) {
                match_name = (strcmp(c.name, norm_name) == 0);
            }
            
            // Comparer centre si spécifié
            if (center && strlen(center) > 0) {
                match_center = (strcmp(c.center, norm_center) == 0);
            }
            
            if (match_name && match_center) {
                results[count] = c;
                count++;
            }
        }
        fclose(f);
    }
    
    return count;
}   

void normalize_string(char *str) {
    if (!str) return;
    
    // Supprimer les espaces/tabs au début
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t')) {
        start++;
    }
    
    // Si toute la chaîne est des espaces
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }
    
    // Supprimer les espaces/tabs à la fin
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    
    // Déplacer la chaîne normalisée au début
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int make_reservation(char *filename_res, reservation r) {
    // Vérifier d'abord si le cours existe et obtenir sa capacité
    FILE *f_course = fopen("courses.txt", "r");
    int capacity = -1;
    char course_name[30] = "Unknown";
    course c;
    
    if (f_course != NULL) {
        while (fscanf(f_course, "%s %s %s %s %d %d %d",
                      c.id, c.name, c.type, c.center,
                      &c.time, &c.equipment, &c.capacity) != EOF) {
            if (strcmp(c.id, r.id_cours) == 0) {
                capacity = c.capacity;
                strncpy(course_name, c.name, sizeof(course_name)-1);
                course_name[sizeof(course_name)-1] = '\0';
                break;
            }
        }
        fclose(f_course);
    }

    if (capacity == -1) {
        fprintf(stderr, "Erreur : Cours %s non trouvé.\n", r.id_cours);
        return 0;
    }

    // CRITIQUE : Ouvrir le fichier en mode lecture+écriture
    FILE *f_check = fopen(filename_res, "r+");
    if (f_check == NULL) {
        // Si le fichier n'existe pas, le créer
        f_check = fopen(filename_res, "w+");
        if (f_check == NULL) {
            fprintf(stderr, "Erreur : Impossible de créer/ouvrir le fichier des réservations.\n");
            return 0;
        }
    }
    
    int current_reservations = 0;
    int already_reserved = 0;
    char line[256];
    
    // COMPTER les réservations et vérifier les doublons
    while (fgets(line, sizeof(line), f_check)) {
        char temp_id_member[30], temp_id_cours[30];
        // Essayer d'extraire l'ID membre et l'ID cours
        if (sscanf(line, "%s %*s %*s %*d %*d %*d %s", 
                   temp_id_member, temp_id_cours) == 2) {
            
            // Compter les réservations pour ce cours
            if (strcmp(temp_id_cours, r.id_cours) == 0) {
                current_reservations++;
            }
            
            // Vérifier si ce membre a déjà réservé ce cours
            if (strcmp(temp_id_member, r.id_member) == 0 && 
                strcmp(temp_id_cours, r.id_cours) == 0) {
                already_reserved = 1;
            }
        }
    }
    
    // VÉRIFIER LA CAPACITÉ
    if (current_reservations >= capacity) {
        fclose(f_check);
        fprintf(stderr, "Erreur : Cours %s (%s) est déjà complet.\n"
               "Capacité: %d, Réservations actuelles: %d\n", 
               r.id_cours, course_name, capacity, current_reservations);
        return 0;
    }
    
    // VÉRIFIER LES DOUBLONS
    if (already_reserved) {
        fclose(f_check);
        fprintf(stderr, "Erreur : Membre %s a déjà réservé le cours %s (%s)\n", 
                r.id_member, r.id_cours, course_name);
        return 0;
    }
    
    // RÉSERVATION VALIDE - Ajouter à la fin du fichier
    fprintf(f_check, "%s %s %s %d %d %d %s\n",
            r.id_member, r.name_member, r.phone_number,
            r.date.jour, r.date.mois, r.date.annee, r.id_cours);
    
    fclose(f_check);

    // Calculer les places restantes
    int remaining = capacity - current_reservations - 1;
    printf("Réservation réussie : Membre %s -> Cours %s (%s)\n"
           "Capacité: %d, Réservations avant: %d, Réservations après: %d, Places restantes: %d\n", 
           r.id_member, r.id_cours, course_name, capacity, 
           current_reservations, current_reservations + 1, remaining);
    
    return 1;
}
// MODIFICATION : Nouvelle fonction pour valider l'ID membre
int validate_member_id(const char *member_id, char *member_name, size_t name_size) {
    FILE *f = fopen("members.txt", "r");
    if (!f) {
        return 0;
    }
    
    char id[30];
    char prenom[30];
    char nom[30];
    int day, month, year;
    char center[30];
    char phone[30];
    int status;
    
    while (fscanf(f, "%s %s %s %d %d %d %s %s %d", 
                  id, prenom, nom, &day, &month, &year, center, phone, &status) != EOF) {
        if (strcmp(id, member_id) == 0) {
            // Format name as "prenom nom"
            snprintf(member_name, name_size, "%s %s", prenom, nom);
            fclose(f);
            return 1;
        }
    }
    
    fclose(f);
    return 0;
}
