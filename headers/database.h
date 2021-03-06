#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include "textformat.h"
#include "note.h"
#include "console.h"

typedef struct{
    FILE *  file;
    char    name[1000];
    size_t  size;
    int     count;
}DB;

DB * db_new(char * name, size_t size)
{
    DB * database = (DB*) malloc(sizeof(DB));
    if((database) == NULL || (database->file = fopen(name, "wb")) == NULL)
        return NULL;
    
    database->size = size;
    strcpy(database->name, name);
    database->count = 0;
    fwrite(database, sizeof(DB), 1, database->file);
    fclose(database->file);
    return database;
}

DB * db_open(char * name)
{
    DB * database = (DB*) malloc(sizeof(DB));
    FILE * file = fopen(name, "rb");
    
    if(file == NULL)
        return NULL;

    fseek(file, 0, SEEK_SET);
    fread(database, sizeof(DB), 1, file);
    database->file = file;
    fclose(file);
    return database;
}

int db_add(void * obj, DB * database){
    if((database->file = fopen(database->name, "ab")) == NULL){
        printf("%s \"%s\"\n", "can't open file", database->name);
        return EXIT_FAILURE;
    }
    fwrite(obj, database->size, 1 , database->file);
    fclose(database->file);

    database->count++;
    if((database->file = fopen(database->name, "r+b")) == NULL){
        printf("%s \"%s\"\n", "can't open file", database->name);
        return EXIT_FAILURE;
    }
    fseek(database->file, 0, SEEK_SET);
    fwrite(database, sizeof(DB), 1, database->file);
    fclose(database->file);
    return 0;
}

int db_showAll(DB * database)
{
    if(database->count == 0)
    {
        printf("this book is empty (use '%snote%s' to add new notes).\n\n", TEXT_COLOR_FG_LMAGENTA, TEXT_DEFAULT);
        return 0;
    }
    
    Note * note = (Note*) malloc(sizeof(Note));
    database->file = fopen(database->name, "rb");

    fseek(database->file, sizeof(DB), SEEK_SET);
    printf("\n%s%-3s %-23s %-30s %-11s %-8s%s\n",TEXT_BOLD, "ID", "TITLE", "INFO", "DATE", "TIME", TEXT_DEFAULT);
    
    for(int i = 0; i < database->count; i++)
    {
        fread(note, database->size, 1, database->file);
        printf("%-3d %-23s %-30s %-11s %-8s\n", note->id,
                console_stringCuted(note->title, 22),
                console_stringCuted(note->info, 29),
                date_string(&note->date),
                time_toString(&note->time));
    }
    
    fclose(database->file);
    printf("\n\n");
}

int db_remove(DB * database, int * ids, int idsCount)
{
    if(database->count == 0)
    {
        printf("this book is empty (use '%snote%s' to add new notes).\n\n", TEXT_COLOR_FG_LMAGENTA, TEXT_DEFAULT);
        return 0;
    }

    //save notes in dinamic memory
    Note * notes = (Note*) malloc(sizeof(Note) * database->count);
    int count = database->count;

    database->file = fopen(database->name, "r+b");
    fseek(database->file, sizeof(DB), SEEK_SET);
    
    for(int i = 0; i < database->count; i++)
        fread(&notes[i], database->size, 1, database->file);
    
    fclose(database->file);

    //clear database
    database->file = fopen(database->name, "wb");
    database->count = 0;
    fseek(database->file, 0, SEEK_SET);
    fwrite(database, sizeof(DB), 1, database->file);
    fclose(database->file);
    
    //write notes in file
    for(int i = 0, newid = 1; i < count; i++)
    {
        if(i == *ids - 1)
        {
            ids++;
            continue;
        }

        notes[i].id = newid;
        newid++;
        db_add(&notes[i], database);
    }

    return 0;
}

#endif //DATABASE_H
