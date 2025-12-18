
#ifndef COURSE_H_INCLUDED
#define COURSE_H_INCLUDED

typedef struct {
    char id[30];        
    char name[30];      
    char type[30];      
    char center[30];    
    int time;          
    int equipment;      
    int capacity;       
} course;

typedef struct {
    int jour;
    int mois;
    int annee;
} Date_d;

typedef struct {
    char id_member[30];
    char name_member[30];
    char phone_number[15];
    Date_d date;
    char id_cours[30];
} reservation;

int add(char *filename, course c);
int modify(char *filename, char *id, course nouv);
int Delete(char *filename, char *id);
course search(char *filename, char *name, char *center);
int search_all_courses(char *filename, char *name, char *center, course results[], int max_results);
int make_reservation(char *filename_res, reservation r);

// MODIFICATION : Ajout de la déclaration de fonction validate_member_id
int validate_member_id(const char *member_id, char *member_name, size_t name_size);
void normalize_string(char *str);

int get_available_spots(const char *course_id); 

#endif // COURSE_H_INCLUDED

