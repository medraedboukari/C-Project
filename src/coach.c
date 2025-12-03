
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

