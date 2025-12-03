#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "coach.h"

// Fonction pour supprimer les espaces/tabs à la fin d'une chaîne
void trim_trailing_whitespace(char *str) {
    if (str == NULL) return;
    
    int len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\t' || str[len-1] == '\n' || str[len-1] == '\r')) {
        str[len-1] = '\0';
        len--;
    }
}

// Fonction pour supprimer les espaces/tabs au début et à la fin
void trim_whitespace(char *str) {
    if (str == NULL) return;
    
    // Trim leading
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }
    
    // Déplacer si nécessaire
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    // Trim trailing
    trim_trailing_whitespace(str);
}

// ----------------------------------------------------
//               ADD COACH
// ----------------------------------------------------
int addCoach(const char *filename, Coach c)
{
    FILE *f = fopen(filename, "a");
    if (!f)
        return 0;

    char gender_str[10];
    strcpy(gender_str, (c.gender == 1) ? "Male" : "Female");

    fprintf(f, "%d %s %s %d %d %d %s %s %s\n",
            c.id, c.lastName, c.firstName,
            c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
            c.center, c.phoneNumber, gender_str);

    fclose(f);
    return 1;
}

// ----------------------------------------------------
//               MODIFY COACH (by ID)
// ----------------------------------------------------
int modifyCoach(const char *filename, int id, Coach updated)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");
    if (!f || !f2)
        return 0;

    Coach c;
    int found = 0;
    char gender_str[10];
    char updated_gender_str[10];
    
    strcpy(updated_gender_str, (updated.gender == 1) ? "Male" : "Female");

    while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, gender_str) == 9)
    {
        // Nettoyer le centre lu du fichier
        trim_trailing_whitespace(c.center);
        
        if (c.id == id)
        {
            fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                    updated.id, updated.lastName, updated.firstName,
                    updated.dateOfBirth.day, updated.dateOfBirth.month, updated.dateOfBirth.year,
                    updated.center, updated.phoneNumber, updated_gender_str);
            found = 1;
        }
        else
        {
            fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                    c.id, c.lastName, c.firstName,
                    c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                    c.center, c.phoneNumber, gender_str);
        }
    }

    fclose(f);
    fclose(f2);

    remove(filename);
    rename("temp.txt", filename);

    return found;
}

// ----------------------------------------------------
//               DELETE COACH (by ID)
// ----------------------------------------------------
int deleteCoach(const char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");
    if (!f || !f2)
        return 0;

    Coach c;
    int found = 0;
    char gender_str[10];

    while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, gender_str) == 9)
    {
        if (c.id == id)
            found = 1; // skip to delete
        else
            fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                    c.id, c.lastName, c.firstName,
                    c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                    c.center, c.phoneNumber, gender_str);
    }

    fclose(f);
    fclose(f2);

    remove(filename);
    rename("temp.txt", filename);

    return found;
}

// ----------------------------------------------------
//               SEARCH COACH (by lastName + center) - CORRIGÉ
// ----------------------------------------------------
Coach searchCoach(const char *filename, const char *lastName, const char *center)
{
    FILE *f = fopen(filename, "r");
    Coach c;
    c.id = -1;
    char gender_str[10];
    if (!f)
        return c;

    while (fscanf(f, "%d %29s %29s %d %d %d %29s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, gender_str) == 9)
    {
        // Convert gender string to int
        c.gender = (strcmp(gender_str, "Male") == 0) ? 1 : 2;
        
        // Nettoyer le centre lu du fichier
        trim_trailing_whitespace(c.center);
        
        // Préparer les chaînes de recherche (nettoyées)
        char search_lastName[30] = "";
        char search_center[30] = "";
        
        if (lastName && strlen(lastName) > 0) {
            strncpy(search_lastName, lastName, sizeof(search_lastName)-1);
            search_lastName[sizeof(search_lastName)-1] = '\0';
            trim_whitespace(search_lastName);
        }
        
        if (center && strlen(center) > 0) {
            strncpy(search_center, center, sizeof(search_center)-1);
            search_center[sizeof(search_center)-1] = '\0';
            trim_whitespace(search_center);
        }
        
        // Comparaison insensible à la casse
        int matchLastName = 1;
        int matchCenter = 1;
        
        if (strlen(search_lastName) > 0) {
            // Comparaison insensible à la casse
            matchLastName = (strcasecmp(c.lastName, search_lastName) == 0);
        }
        
        if (strlen(search_center) > 0) {
            // Comparaison insensible à la casse
            matchCenter = (strcasecmp(c.center, search_center) == 0);
        }

        if (matchLastName && matchCenter)
        {
            fclose(f);
            return c;
        }
    }

    fclose(f);
    c.id = -1; // Not found
    return c;
}

// ----------------------------------------------------
//               ASSIGN COACH TO COURSE
// ----------------------------------------------------
int assignCoachToCourse(const char *filename, CoachAssignment assignment)
{
    FILE *f = fopen(filename, "a");
    if (!f)
        return 0;

    fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%d|%d|%d\n",
            assignment.coach_id,
            assignment.coach_name,
            assignment.coach_phone,
            assignment.course_id,
            assignment.course_name,
            assignment.course_center,
            assignment.course_time,
            assignment.day,
            assignment.month,
            assignment.year);

    fclose(f);
    return 1;
}

// ----------------------------------------------------
//               CHECK IF COACH IS ASSIGNED
// ----------------------------------------------------
int isCoachAssigned(const char *filename, int coach_id, const char *course_id)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return 0;

    char line[256];
    int found = 0;
    
    while (fgets(line, sizeof(line), f))
    {
        int stored_coach_id;
        char stored_course_id[10];
        
        // Parser la ligne avec le format: id|nom|phone|cours_id|...
        sscanf(line, "%d|%*[^|]|%*[^|]|%[^|]", &stored_coach_id, stored_course_id);
        
        if (stored_coach_id == coach_id && strcmp(stored_course_id, course_id) == 0)
        {
            found = 1;
            break;
        }
    }
    
    fclose(f);
    return found;
}

// ----------------------------------------------------
//               GET COACH COUNT FOR COURSE
// ----------------------------------------------------
int getCoachCountForCourse(const char *filename, const char *course_id, int capacity)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        printf("DEBUG: File %s not found, returning 0\n", filename);
        return 0; // Si le fichier n'existe pas, aucun coach assigné
    }

    char line[256];
    int count = 0;
    
    printf("DEBUG: Checking assignments for course %s\n", course_id);
    
    while (fgets(line, sizeof(line), f))
    {
        char stored_course_id[10];
        // Extraire seulement l'ID du cours
        if (sscanf(line, "%*d|%*[^|]|%*[^|]|%[^|]", stored_course_id) == 1)
        {
            if (strcmp(stored_course_id, course_id) == 0)
            {
                count++;
                printf("DEBUG: Found assignment #%d for course %s\n", count, course_id);
            }
        }
    }
    
    fclose(f);
    printf("DEBUG: Total assignments for course %s: %d\n", course_id, count);
    return count;
}

// ----------------------------------------------------
//               LOAD COURSE ASSIGNMENTS (pour TreeView)
// ----------------------------------------------------
void loadCourseAssignments(GtkTreeView *treeview, const char *filename)
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
        // Créer les colonnes si elles n'existent pas
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        const char *titles[10] = {"Coach ID", "Coach Name", "Phone", "Course ID", 
                                  "Course Name", "Center", "Time", "Day", "Month", "Year"};
        
        for (int i = 0; i < 10; i++)
        {
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_append_column(treeview, column);
        }
    }

    GtkListStore *store;
    GtkTreeIter iter;

    store = gtk_list_store_new(10,
                               G_TYPE_INT,    // Coach ID
                               G_TYPE_STRING, // Coach Name
                               G_TYPE_STRING, // Phone
                               G_TYPE_STRING, // Course ID
                               G_TYPE_STRING, // Course Name
                               G_TYPE_STRING, // Center
                               G_TYPE_STRING, // Time
                               G_TYPE_INT,    // Day
                               G_TYPE_INT,    // Month
                               G_TYPE_INT     // Year
    );

    FILE *f = fopen(filename, "r");
    if (f)
    {
        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            CoachAssignment ca;
            char *token;
            int field = 0;
            
            // Parser la ligne délimitée par |
            token = strtok(line, "|");
            while (token != NULL && field < 10)
            {
                switch(field)
                {
                    case 0: ca.coach_id = atoi(token); break;
                    case 1: strncpy(ca.coach_name, token, sizeof(ca.coach_name)); break;
                    case 2: strncpy(ca.coach_phone, token, sizeof(ca.coach_phone)); break;
                    case 3: strncpy(ca.course_id, token, sizeof(ca.course_id)); break;
                    case 4: strncpy(ca.course_name, token, sizeof(ca.course_name)); break;
                    case 5: strncpy(ca.course_center, token, sizeof(ca.course_center)); break;
                    case 6: strncpy(ca.course_time, token, sizeof(ca.course_time)); break;
                    case 7: ca.day = atoi(token); break;
                    case 8: ca.month = atoi(token); break;
                    case 9: ca.year = atoi(token); break;
                }
                token = strtok(NULL, "|");
                field++;
            }
            
            if (field == 10)
            {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                   0, ca.coach_id,
                                   1, ca.coach_name,
                                   2, ca.coach_phone,
                                   3, ca.course_id,
                                   4, ca.course_name,
                                   5, ca.course_center,
                                   6, ca.course_time,
                                   7, ca.day,
                                   8, ca.month,
                                   9, ca.year,
                                   -1);
            }
        }
        fclose(f);
    }

    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
}
