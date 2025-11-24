#ifndef COACH_H_INCLUDED
#define COACH_H_INCLUDED

#include <stdio.h>

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
    char gender[10];
} Coach;


int addCoach(char *filename, Coach c);
int modifyCoach(char *filename, int id, Coach updated);
int deleteCoach(char *filename, int id);
Coach searchCoach(char *filename, int id);

#endif 
