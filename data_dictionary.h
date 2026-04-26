#ifndef DATA_DICTIONARY_H
#define DATA_DICTIONARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 50
#define NULL_POINTER                -1L   
#define MAIN_DATA_DICTIONARY_HEADER  0L   

typedef enum {
    EXIT                  = 0,
    CREATE_DATA_DICTIONARY = 1,
    OPEN_DATA_DICTIONARY   = 2
} MenuChoice;
 
typedef enum {
    ENTITY_MENU_EXIT    = 0,
    INSERT_ENTITY       = 1,
    LIST_ENTITIES       = 2,
    DELETE_ENTITY       = 3
} EntityMenuChoice;

typedef struct {
    char name[MAX_CHARS];
    long dataPointer;
    long attributesPointer;
    long nextEntity;
} Entity;

typedef struct {
    char name[MAX_CHARS];
    int  type;        
    int  size;
    char isPrimaryKey; 
    long nextAttribute;
} Attribute;

int  createDataDictionary(const char *fileName);
FILE *openDataDictionary(const char *fileName);
int   createEntity(FILE *dataDictionary, const char *entityName);
void  printEntities(FILE *dataDictionary);
int   removeEntity(FILE *dataDictionary, const char *entityName);

#endif 
