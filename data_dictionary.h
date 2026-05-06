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
    DELETE_ENTITY       = 3,
    MANAGE_ATTRIBUTES = 4,
    MANAGE_RECORDS    = 5
} EntityMenuChoice;

typedef enum {
    ATTRIBUTE_MENU_EXIT  = 0,
    INSERT_ATTRIBUTE     = 1,
    LIST_ATTRIBUTES      = 2,
    DELETE_ATTRIBUTE     = 3
} AttributeMenuChoice;

typedef enum {
    RECORD_MENU_EXIT = 0,
    INSERT_RECORD    = 1,
    LIST_RECORDS     = 2
} RecordMenuChoice;

typedef enum {
    Integer   = 0,
    Decimal   = 1,
    Character = 2,
    String    = 3
} AttributeType;

typedef struct {
    char name[MAX_CHARS];
    long dataPointer;
    long attributesPointer;
    long nextEntity;
} Entity;

typedef struct {
    char name[MAX_CHARS];
    AttributeType type;
    int  length;       
    char isPrimaryKey;
    long nextAttribute;
} Attribute;

typedef struct {
    void *data;             
    long  primaryKeyOffset; 
    int   primaryKeyLength; 
    int   dataLength;       
} DataRecord;

int createDataDictionary (const char *fileName);
FILE *openDataDictionary (const char *fileName);
int createEntity (FILE *dataDictionary, const char *entityName);
void printEntities (FILE *dataDictionary);
int removeEntity (FILE *dataDictionary, const char *entityName);
long findEntity (FILE *dataDictionary, Entity *entity);
long appendAttribute (Attribute attribute,  FILE *dataDictionary);
int createAttribute (FILE *dataDictionary, long attributesHeader, Attribute attribute);
void printAttributes (FILE *dataDictionary, long attributesHeader);
int removeAttribute (FILE *dataDictionary, long attributesHeader, const char *attributeName);
DataRecord generateDataRecord (FILE *dataDictionary, long attributesHeader);
long appendDataRecord (FILE *dataDictionary, DataRecord dataRecord);
int createDataRecord (FILE *dataDictionary, long attributesHeader, long dataRecordsHeader);
void printDataRecords (FILE *dataDictionary, long attributesHeader, long dataRecordsHeader);

#endif 
