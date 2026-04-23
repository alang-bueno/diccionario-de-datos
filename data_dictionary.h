#ifndef DATA_DICTIONARY_H
#define DATA_DICTIONARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 50
#define NULL_POINTER                -1L   
#define MAIN_DATA_DICTIONARY_HEADER  0L   

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

#endif 
