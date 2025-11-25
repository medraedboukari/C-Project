#include <stdio.h>
#include <string.h>
#include "coach.h"

// ----------------------------------------------------
//               ADD COACH
// ----------------------------------------------------
int addCoach(char *filename, Coach c)
{
    FILE *f = fopen(filename, "a");
    if (f == NULL)
        return 0;

    fprintf(f, "%d %s %s %d %d %d %s %s %s\n",
            c.id, c.lastName, c.firstName,
            c.dateOfBirth.day, c.dateOfBirth.month, c.dateOfBirth.year,
            c.center, c.phoneNumber, c.gender);

    fclose(f);
    return 1;
}


// ----------------------------------------------------
//               MODIFY COACH
// ----------------------------------------------------
int modifyCoach(char *filename, int id, Coach updated)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");

    if (f == NULL || f2 == NULL)
        return 0;

    Coach c;
    int found = 0;

    while (fscanf(f, "%d %49s %49s %d %d %d %49s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, c.gender) == 9)
    {
        if (c.id == id)
        {
            // Write updated coach
            fprintf(f2, "%d %s %s %d %d %d %s %s %s\n",
                    updated.id, updated.lastName, updated.firstName,
                    updated.dateOfBirth.day, updated.dateOfBirth.month, updated.dateOfBirth.year,
                    updated.center, updated.phoneNumber, updated.gender);
            found = 1;
        }
        else
        {
            // Write original
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

    return found;
}


// ----------------------------------------------------
//               DELETE COACH
// ----------------------------------------------------
int deleteCoach(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");

    if (f == NULL || f2 == NULL)
        return 0;

    Coach c;
    int found = 0;

    while (fscanf(f, "%d %49s %49s %d %d %d %49s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, c.gender) == 9)
    {
        if (c.id == id)
            found = 1;  // Skip writing => delete

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

    return found;
}


// ----------------------------------------------------
//               SEARCH COACH
// ----------------------------------------------------
Coach searchCoach(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    Coach c;
    int found = 0;

    if (f == NULL)
    {
        c.id = -1;
        return c;
    }

    while (fscanf(f, "%d %49s %49s %d %d %d %49s %19s %9s",
                  &c.id, c.lastName, c.firstName,
                  &c.dateOfBirth.day, &c.dateOfBirth.month, &c.dateOfBirth.year,
                  c.center, c.phoneNumber, c.gender) == 9)
    {
        if (c.id == id)
        {
            found = 1;
            break;
        }
    }

    fclose(f);

    if (!found)
        c.id = -1;

    return c;
}

