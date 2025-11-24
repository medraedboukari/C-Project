#include <stdio.h>
#include <string.h>
#include "coach.h"

int addCoach(char *filename, Coach c)
{
    FILE *f = fopen(filename, "a");
    if (f != NULL)
    {
        fprintf(f, "%d %s %s %d %d %d %s %s %s\n",
                c.id, c.lastName, c.firstName,
                c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                c.center, c.phoneNumber, c.gender);
        fclose(f);
        return 1;
    }
    else
        return 0;
}

int modifyCoach(char *filename, int id, Coach updated)
{
    int found = 0;
    Coach c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");

    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%d %s %s %d %d %d %s %s %s\n",
                      &c.id, c.lastName, c.firstName,
                      &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                      c.center, c.phoneNumber, c.gender) != EOF)
        {
            if (c.id == id)
            {
                fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                        updated.id, updated.lastName, updated.firstName,
                        updated.dateOfBirth.day, updated.dateOfBirth.month, updated.dateOfBirth.year,
                        updated.center, updated.phoneNumber, updated.gender);
                found = 1;
            }
            else
            {
                fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                        c.id, c.lastName, c.firstName,
                        c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                        c.center, c.phoneNumber, c.gender);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("temp.txt", filename);
    }
    return found;
}

int deleteCoach(char *filename, int id)
{
    int found = 0;
    Coach c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");

    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%d %s %s %d %d %d %s %s %s\n",
                      &c.id, c.lastName, c.firstName,
                      &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                      c.center, c.phoneNumber, c.gender) != EOF)
        {
            if (c.id == id)
                found = 1;
            else
                fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                        c.id, c.lastName, c.firstName,
                        c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
                        c.center, c.phoneNumber, c.gender);
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("temp.txt", filename);
    }
    return found;
}

Coach searchCoach(char *filename, int id)
{
    Coach c;
    int found = 0;
    FILE *f = fopen(filename, "r");

    if (f != NULL)
    {
        while (fscanf(f, "%d %s %s %d %d %d %s %s %s\n",
                      &c.id, c.lastName, c.firstName,
                      &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                      c.center, c.phoneNumber, c.gender) != EOF)
        {
            if (c.id == id)
            {
                found = 1;
                break;
            }
        }
        fclose(f);
    }

    if (!found)
        c.id = -1;

    return c;
}
