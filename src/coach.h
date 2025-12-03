
#ifndef COACH_H_INCLUDED
#define COACH_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

// ----------------------------
//          STRUCTURES
// ----------------------------
typedef struct
{
    int day;
    int month;
    int year;
} Date;

typedef struct
{
    int id;
    char lastName[30];
    char firstName[30];
    Date dateOfBirth;
    char center[30];
    char phoneNumber[20];
    int gender; // 1 = Male, 2 = Female
} Coach;

// ----------------------------
//         PROTOTYPES
// ----------------------------

// Ajout d'un coach
int addCoach(const char *filename, Coach c);

// Modification d'un coach par ID
int modifyCoach(const char *filename, int id, Coach updated);

// Suppression d'un coach par ID
int deleteCoach(const char *filename, int id);

// Recherche d'un coach (nom + centre)
Coach searchCoach(const char *filename, const char *lastName, const char *center);

#endif // COACH_H_INCLUDED

