#ifndef DATA_DICTIONARY_H
#define DATA_DICTIONARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 50

// ─────────────────────────────────────────────
//  Estructuras
// ─────────────────────────────────────────────

typedef struct {
    char name[MAX_CHARS];
    long dataPointer;
    long attributesPointer;
    long nextEntity;
} Entity;

typedef struct {
    char name[MAX_CHARS];
    int  type;        // 0=INT, 1=FLOAT, 2=CHAR, 3=VARCHAR
    int  size;
    char isPrimaryKey; // 1=Sí, 0=No
    long nextAttribute;
} Attribute;

// ─────────────────────────────────────────────
//  Funciones
// ─────────────────────────────────────────────

int  createDataDictionary(const char *fileName);

#endif // DATA_DICTIONARY_H
