#include "data_dictionary.h"
#include <ctype.h>

static void strToLower(char *dst, const char *src, int maxLen);

int createDataDictionary(const char *fileName) {
    if (fileName == NULL || fileName[0] == '\0') {
        printf("Error: El nombre del archivo no puede estar vacío.\n");
        return 0;
    }

    FILE *check = fopen(fileName, "rb");
    if (check != NULL) {
        fclose(check);

        printf("Advertencia: El diccionario '%s' ya existe.\n", fileName);
        printf("¿Deseas sobrescribirlo? (s/n): ");

        char respuesta;
        scanf(" %c", &respuesta);

        if (respuesta != 's' && respuesta != 'S') {
            printf("Operación cancelada. El diccionario existente no fue modificado.\n");
            return 0;
        }
    }

    FILE *file = fopen(fileName, "wb+");
    if (file == NULL) {
        printf("Error: No se pudo crear el archivo '%s'.\n", fileName);
        return 0;
    }

    long header = -1;
    if (fwrite(&header, sizeof(long), 1, file) != 1) {
        printf("Error: No se pudo escribir la cabecera en '%s'.\n", fileName);
        fclose(file);
        return 0;
    }

    printf("Diccionario de datos '%s' creado exitosamente.\n", fileName);
    fclose(file);
    return 1;
}

FILE *openDataDictionary(const char *fileName) {
    if (fileName == NULL || fileName[0] == '\0') {
        printf("Error: El nombre del archivo no puede estar vacío.\n");
        return NULL;
    }

    FILE *file = fopen(fileName, "rb+");
    if (file == NULL) {
        printf("Error: No se encontró el diccionario '%s'.\n", fileName);
        printf("Sugerencia: Crea uno nuevo antes de intentar abrirlo.\n");
        return NULL;
    }
 
    long header = 0;
    if (fread(&header, sizeof(long), 1, file) != 1) {
        printf("Advertencia: No se pudo leer la cabecera de '%s'.\n", fileName);
        printf("El archivo podría estar dañado o vacío.\n");
        fclose(file);
        return NULL;
    }
 
    if (header < -1) {
        printf("Advertencia: La cabecera del archivo '%s' contiene un valor\n", fileName);
        printf("fuera de rango (%ld). El archivo podría estar dañado.\n", header);
        fclose(file);
        return NULL;
    }
 
    printf("Diccionario '%s' abierto correctamente.\n", fileName);
 
    if (header == -1) {
        printf("Estado: El diccionario está vacío (sin entidades registradas).\n");
    } else {
        printf("Estado: El diccionario contiene entidades. ");
        printf("Primera entidad en offset: %ld bytes.\n", header);
    }
 
    return file;
}

static long appendEntity(Entity table, FILE *dataDictionary) {
    long entityDirection;
 
    if (fseek(dataDictionary, 0, SEEK_END) != 0) {
        printf("Error: No se pudo mover al final del archivo.\n");
        return -1;
    }
 
    entityDirection = ftell(dataDictionary);
    if (entityDirection == -1) {
        printf("Error: No se pudo obtener la posición en el archivo.\n");
        return -1;
    }
 
    if (fwrite(&table, sizeof(Entity), 1, dataDictionary) != 1) {
        printf("Error: No se pudo escribir la entidad en el archivo.\n");
        return -1;
    }
 
    return entityDirection;
}
 
static int entityExists(FILE *dataDictionary, const char *entityName) {
    long currentDir;
    fseek(dataDictionary, 0, SEEK_SET);
    if (fread(&currentDir, sizeof(long), 1, dataDictionary) != 1)
        return 0;

    while (currentDir != NULL_POINTER) {
        Entity current;
        fseek(dataDictionary, currentDir, SEEK_SET);
        if (fread(&current, sizeof(Entity), 1, dataDictionary) != 1)
            break;

        char existingLower[MAX_CHARS];
        char searchLower[MAX_CHARS];
        strToLower(existingLower, current.name, MAX_CHARS);
        strToLower(searchLower, entityName, MAX_CHARS);

        if (strcmp(searchLower, existingLower) == 0)
            return 1;

        currentDir = current.nextEntity;
    }
    return 0;
}

int createEntity(FILE *dataDictionary, const char *entityName) {
    if (entityName == NULL || entityName[0] == '\0') {
        printf("Error: El nombre de la entidad no puede estar vacío.\n");
        return 0;
    }
    if (entityExists(dataDictionary, entityName)) {
        printf("Error: La entidad '%s' ya existe en el diccionario.\n", entityName);
        return 0;
    }
    Entity newEntity;
    memset(&newEntity, 0, sizeof(Entity));
    strcpy(newEntity.name, entityName);
    newEntity.dataPointer = NULL_POINTER;
    newEntity.attributesPointer = NULL_POINTER;
    newEntity.nextEntity = NULL_POINTER;
 
    long currentEntityPtr = MAIN_DATA_DICTIONARY_HEADER; 
    long currentEntityDir;
 
    fseek(dataDictionary, currentEntityPtr, SEEK_SET);
    if (fread(&currentEntityDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera del diccionario.\n");
        return 0;
    }
 
    int sortingCriteriaMet = 0;
 
    while (currentEntityDir != NULL_POINTER && !sortingCriteriaMet) {
        Entity currentEntity;
 
        fseek(dataDictionary, currentEntityDir, SEEK_SET);
        if (fread(&currentEntity, sizeof(Entity), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer una entidad existente.\n");
            return 0;
        }

        char newLower[MAX_CHARS], existingLower[MAX_CHARS];
        strToLower(newLower, entityName, MAX_CHARS);
        strToLower(existingLower, currentEntity.name, MAX_CHARS);
        if (strcmp(newLower, existingLower) < 0) {
            sortingCriteriaMet = 1;
        }
        else
        {
            currentEntityPtr = currentEntityDir + (long)sizeof(Entity) - (long)sizeof(long);
            currentEntityDir = currentEntity.nextEntity;
        }
    }
 
    long entityDir = appendEntity(newEntity, dataDictionary);
    if (entityDir == -1)
        return 0;
 
    fseek(dataDictionary, currentEntityPtr, SEEK_SET);
    if (fwrite(&entityDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace al nodo anterior.\n");
        return 0;
    }
 
    long nextPtr = entityDir + (long)sizeof(Entity) - (long)sizeof(long);
    fseek(dataDictionary, nextPtr, SEEK_SET);
    if (fwrite(&currentEntityDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace al nodo siguiente.\n");
        return 0;
    }
 
    printf("Entidad '%s' insertada correctamente.\n", entityName);
    return 1;
}
 
void printEntities(FILE *dataDictionary) {
    long currentDir;
    fseek(dataDictionary, 0, SEEK_SET);
    if (fread(&currentDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera.\n");
        return;
    }
 
    if (currentDir == NULL_POINTER) {
        printf("El diccionario está vacío. No hay entidades registradas.\n");
        return;
    }
 
    printf("\n%-5s %-20s %-10s %-10s %-10s\n",
           "No.", "Nombre", "Offset", "Atributos", "Siguiente");
    printf("%-5s %-20s %-10s %-10s %-10s\n",
           "───", "──────────────────", "────────", "─────────", "─────────");
 
    int count = 1;
    while (currentDir != NULL_POINTER) {
        Entity current;
 
        fseek(dataDictionary, currentDir, SEEK_SET);
        if (fread(&current, sizeof(Entity), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer la entidad en offset %ld.\n", currentDir);
            return;
        }
 
        printf("%-5d %-20s %-10ld %-10ld %-10ld\n", count, current.name, currentDir, current.attributesPointer, current.nextEntity);
        currentDir = current.nextEntity;
        count++;
    }
    printf("\nTotal de entidades: %d\n", count - 1);
}

int removeEntity(FILE *dataDictionary, const char *entityName) {
    if (entityName == NULL || entityName[0] == '\0') {
        printf("Error: El nombre de la entidad no puede estar vacío.\n");
        return 0;
    }
 
    long currentEntityDir = NULL_POINTER;
    long currentEntityPtr = MAIN_DATA_DICTIONARY_HEADER;
    int  entityFound      = 0;

    fseek(dataDictionary, currentEntityPtr, SEEK_SET);
    if (fread(&currentEntityDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera del diccionario.\n");
        return 0;
    }
 
    while (currentEntityDir != NULL_POINTER && entityFound == 0) {
        Entity currentEntity;
        fseek(dataDictionary, currentEntityDir, SEEK_SET);
        if (fread(&currentEntity, sizeof(Entity), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer entidad en offset %ld.\n",
                   currentEntityDir);
            return 0;
        }

        char searchLower[MAX_CHARS], existingLower[MAX_CHARS];
        strToLower(searchLower, entityName, MAX_CHARS);
        strToLower(existingLower, currentEntity.name, MAX_CHARS);
        if (strcmp(searchLower, existingLower) == 0) {
            printf("¿Seguro que deseas eliminar la entidad '%s'? (s/n): ",
                   entityName);
            char respuesta;
            scanf(" %c", &respuesta);
 
            if (respuesta != 's' && respuesta != 'S') {
                printf("Operación cancelada. No se eliminó ninguna entidad.\n");
                return 0;
            }
            printf("\nEstado ANTES de eliminar:\n");
            printEntities(dataDictionary);
            fseek(dataDictionary, currentEntityPtr, SEEK_SET);
            if (fwrite(&currentEntity.nextEntity, sizeof(long), 1,
                       dataDictionary) != 1) {
                printf("Error: No se pudo actualizar el puntero anterior.\n");
                return 0;
            }
            entityFound = 1;
        }
        else
        {
            currentEntityPtr = currentEntityDir + (long)sizeof(Entity) - (long)sizeof(long);
            currentEntityDir = currentEntity.nextEntity;
        }
    }

    if (entityFound) {
        printf("\nEntidad '%s' eliminada correctamente.\n", entityName);
        long firstEntity;
        fseek(dataDictionary, MAIN_DATA_DICTIONARY_HEADER, SEEK_SET);
        fread(&firstEntity, sizeof(long), 1, dataDictionary);
        if (firstEntity == NULL_POINTER) {
            printf("El diccionario ha quedado vacío.\n");
        } else {
            printf("\nEstado DESPUÉS de eliminar:\n");
            printEntities(dataDictionary);
        }
    } else {
        printf("Error: No se encontró la entidad '%s' en el diccionario.\n",
               entityName);
    }
 
    return entityFound;
}

static void strToLower(char *dst, const char *src, int maxLen) {
    int i;
    for (i = 0; i < maxLen - 1 && src[i] != '\0'; i++)
        dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

int defaultLengthForType(AttributeType type) {
    switch (type) {
        case Integer:   return 4;
        case Decimal:   return 8;
        case Character: return 1;
        case String:    return MAX_CHARS;
        default:        return 0;
    }
}

static int validateAttributeLength(AttributeType type, int length) {
    switch (type) {
        case Integer:   return (length == 4);
        case Decimal:   return (length == 8);
        case Character: return (length == 1);
        case String:    return (length > 0 && length <= MAX_CHARS);
        default:        return 0;
    }
}

long findEntity(FILE *dataDictionary, Entity *entity) {
    long currentEntityDir = NULL_POINTER;
    int  entityFound      = 0;
 
    fseek(dataDictionary, MAIN_DATA_DICTIONARY_HEADER, SEEK_SET);
    if (fread(&currentEntityDir, sizeof(long), 1, dataDictionary) != 1)
        return NULL_POINTER;
 
    while (currentEntityDir != NULL_POINTER && !entityFound) {
        Entity currentEntity;
 
        fseek(dataDictionary, currentEntityDir, SEEK_SET);
        if (fread(&currentEntity, sizeof(Entity), 1, dataDictionary) != 1)
            return NULL_POINTER;

        char searchLower[MAX_CHARS], existingLower[MAX_CHARS];
        strToLower(searchLower, entity->name, MAX_CHARS);
        strToLower(existingLower, currentEntity.name, MAX_CHARS);
        if (strcmp(searchLower, existingLower) == 0) {
            entity->dataPointer       = currentEntity.dataPointer;
            entity->attributesPointer = currentEntity.attributesPointer;
            entity->nextEntity        = currentEntity.nextEntity;
            entityFound = 1;
        }
        else
        {
            currentEntityDir = currentEntity.nextEntity;
        }
    }
 
    return entityFound ? currentEntityDir : NULL_POINTER;
}

long appendAttribute(Attribute attribute, FILE *dataDictionary) {
    if (fseek(dataDictionary, 0, SEEK_END) != 0) {
        printf("Error: No se pudo posicionar al final del archivo.\n");
        return NULL_POINTER;
    }
 
    long offset = ftell(dataDictionary);
    if (offset == -1) {
        printf("Error: No se pudo obtener la posición final del archivo.\n");
        return NULL_POINTER;
    }
 
    if (fwrite(&attribute, sizeof(Attribute), 1, dataDictionary) != 1) {
        printf("Error: No se pudo escribir el atributo en el archivo.\n");
        return NULL_POINTER;
    }
 
    return offset;
}

int createAttribute(FILE *dataDictionary, long attributesHeader,
                    Attribute attribute) {
 
    if (attribute.name[0] == '\0') {
        printf("Error: El nombre del atributo no puede estar vacío.\n");
        return 0;
    }
 
    if (!validateAttributeLength(attribute.type, attribute.length)) {
        printf("Error: Longitud %d no válida para el tipo elegido.\n",
               attribute.length);
        return 0;
    }
 
    long currentAttributeDir = NULL_POINTER;
    long currentAttributePtr = attributesHeader;
    int  sortingCriteriaMet  = 0;
 
    char newNameLower[MAX_CHARS];
    strToLower(newNameLower, attribute.name, MAX_CHARS);

    fseek(dataDictionary, currentAttributePtr, SEEK_SET);
    if (fread(&currentAttributeDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera de atributos.\n");
        return 0;
    }
 
    while (currentAttributeDir != NULL_POINTER && !sortingCriteriaMet) {
        Attribute currentAttribute;
 
        fseek(dataDictionary, currentAttributeDir, SEEK_SET);
        if (fread(&currentAttribute, sizeof(Attribute), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer atributo en offset %ld.\n",
                   currentAttributeDir);
            return 0;
        }

        char existingLower[MAX_CHARS];
        strToLower(existingLower, currentAttribute.name, MAX_CHARS);
        if (strcmp(newNameLower, existingLower) == 0) {
            printf("Error: Ya existe un atributo llamado '%s' en esta entidad.\n",
                   currentAttribute.name);
            return 0;
        }
 
        if (attribute.isPrimaryKey == 'Y' && currentAttribute.isPrimaryKey == 'Y') {
            printf("Error: Ya existe una llave primaria ('%s') en esta entidad.\n",
                   currentAttribute.name);
            printf("Solo se permite una Primary Key por entidad.\n");
            return 0;
        }
 
        if (strcmp(newNameLower, existingLower) < 0) {
            sortingCriteriaMet = 1;
        } else {
            currentAttributePtr = currentAttributeDir + (long)sizeof(Attribute) - (long)sizeof(long);
            currentAttributeDir = currentAttribute.nextAttribute;
        }
    }
 
    long attributeDir = appendAttribute(attribute, dataDictionary);
    if (attributeDir == NULL_POINTER)
        return 0;

    fseek(dataDictionary, currentAttributePtr, SEEK_SET);
    if (fwrite(&attributeDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace al nodo anterior.\n");
        return 0;
    }
 
    long nextPtr = attributeDir + (long)sizeof(Attribute) - (long)sizeof(long);
    fseek(dataDictionary, nextPtr, SEEK_SET);
    if (fwrite(&currentAttributeDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace al nodo siguiente.\n");
        return 0;
    }
 
    printf("Atributo '%s' insertado correctamente.\n", attribute.name);
    return 1;
}

void printAttributes(FILE *dataDictionary, long attributesHeader) {
    long currentDir;
 
    fseek(dataDictionary, attributesHeader, SEEK_SET);
    if (fread(&currentDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera de atributos.\n");
        return;
    }
 
    if (currentDir == NULL_POINTER) {
        printf("Esta entidad no tiene atributos registrados.\n");
        return;
    }

    const char *tipos[] = {"Integer", "Decimal", "Character", "String"};
    printf("\n%-20s %-12s %-8s %-5s\n",
           "Atributo", "Tipo", "Length", "PK");
    printf("%-20s %-12s %-8s %-5s\n",
           "────────────────────", "────────────", "────────", "─────");
 
    int count = 0;
    while (currentDir != NULL_POINTER) {
        Attribute current;
 
        fseek(dataDictionary, currentDir, SEEK_SET);
        if (fread(&current, sizeof(Attribute), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer atributo en offset %ld.\n", currentDir);
            return;
        }
 
        printf("%-20s %-12s %-8d %-5c\n",
               current.name,
               tipos[current.type],
               current.length,
               current.isPrimaryKey);
 
        currentDir = current.nextAttribute;
        count++;
    }
    printf("\nTotal de atributos: %d\n", count);
}

int removeAttribute(FILE *dataDictionary, long attributesHeader, const char *attributeName) {
    if (attributeName == NULL || attributeName[0] == '\0') {
        printf("Error: El nombre del atributo no puede estar vacío.\n");
        return 0;
    }
 
    long currentAttributeDir = NULL_POINTER;
    long currentAttributePtr = attributesHeader;
    int  attributeFound      = 0;
 
    fseek(dataDictionary, currentAttributePtr, SEEK_SET);
    if (fread(&currentAttributeDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera de atributos.\n");
        return 0;
    }
 
    char searchLower[MAX_CHARS];
    strToLower(searchLower, attributeName, MAX_CHARS);
 
    while (currentAttributeDir != NULL_POINTER && !attributeFound) {
        Attribute currentAttribute;
 
        fseek(dataDictionary, currentAttributeDir, SEEK_SET);
        if (fread(&currentAttribute, sizeof(Attribute), 1, dataDictionary) != 1) {
            printf("Error: Fallo al leer atributo en offset %ld.\n",
                   currentAttributeDir);
            return 0;
        }
 
        char existingLower[MAX_CHARS];
        strToLower(existingLower, currentAttribute.name, MAX_CHARS);
 
        if (strcmp(searchLower, existingLower) == 0) {
            printf("¿Seguro que deseas eliminar el atributo '%s'? (s/n): ",
                   currentAttribute.name);
            char respuesta;
            scanf(" %c", &respuesta);
 
            if (respuesta != 's' && respuesta != 'S') {
                printf("Operación cancelada.\n");
                return 0;
            }
 
            printf("\nEstado ANTES de eliminar:\n");
            printAttributes(dataDictionary, attributesHeader);
 
            fseek(dataDictionary, currentAttributePtr, SEEK_SET);
            if (fwrite(&currentAttribute.nextAttribute, sizeof(long), 1, dataDictionary) != 1) {
                printf("Error: No se pudo actualizar el puntero anterior.\n");
                return 0;
            }
 
            attributeFound = 1;
 
        } else {
            currentAttributePtr = currentAttributeDir + (long)sizeof(Attribute) - (long)sizeof(long);
            currentAttributeDir = currentAttribute.nextAttribute;
        }
    }
 
    if (attributeFound) {
        printf("\nAtributo '%s' eliminado correctamente.\n", attributeName);
        long firstAttr;
        fseek(dataDictionary, attributesHeader, SEEK_SET);
        fread(&firstAttr, sizeof(long), 1, dataDictionary);
 
        if (firstAttr == NULL_POINTER) {
            printf("La entidad ha quedado sin atributos.\n");
        } else {
            printf("\nEstado DESPUÉS de eliminar:\n");
            printAttributes(dataDictionary, attributesHeader);
        }
    } else {
        printf("Error: No se encontró el atributo '%s' en esta entidad.\n",
               attributeName);
    }
 
    return attributeFound;
}

static int pkExists(FILE *dataDictionary, long dataRecordsHeader, DataRecord *newRecord) {
    if (newRecord->primaryKeyOffset == -1 || newRecord->primaryKeyLength == 0)
        return 0;
 
    long currentDir;
    fseek(dataDictionary, dataRecordsHeader, SEEK_SET);
    if (fread(&currentDir, sizeof(long), 1, dataDictionary) != 1)
        return 0;
 
    while (currentDir != NULL_POINTER) {
        void *currentBlock = malloc(newRecord->dataLength);
        if (!currentBlock) return 0;
 
        fseek(dataDictionary, currentDir, SEEK_SET);
        fread(currentBlock, newRecord->dataLength, 1, dataDictionary);

        char pkA[MAX_CHARS], pkB[MAX_CHARS];
        memset(pkA, 0, MAX_CHARS);
        memset(pkB, 0, MAX_CHARS);
        memcpy(pkA, (char *)newRecord->data + newRecord->primaryKeyOffset, newRecord->primaryKeyLength);
        memcpy(pkB, (char *)currentBlock + newRecord->primaryKeyOffset, newRecord->primaryKeyLength);

        char pkALower[MAX_CHARS], pkBLower[MAX_CHARS];
        strToLower(pkALower, pkA, MAX_CHARS);
        strToLower(pkBLower, pkB, MAX_CHARS);

        int equal = (strcmp(pkALower, pkBLower) == 0);

        long nextDir;
        memcpy(&nextDir, (char *)currentBlock + newRecord->dataLength - sizeof(long), sizeof(long));
        free(currentBlock);
 
        if (equal) return 1;
        currentDir = nextDir;
    }
 
    return 0;
}

static int comparePK(DataRecord *newRec, void *existingBlock) {
    const char *newPK = (char *)newRec->data + newRec->primaryKeyOffset;
    const char *exPK  = (char *)existingBlock + newRec->primaryKeyOffset;
 
    if (newRec->primaryKeyLength == sizeof(int)) {
        int a, b;
        memcpy(&a, newPK, sizeof(int));
        memcpy(&b, exPK,  sizeof(int));
        return a - b;
    }
    if (newRec->primaryKeyLength == sizeof(float)) {
        float a, b;
        memcpy(&a, newPK, sizeof(float));
        memcpy(&b, exPK,  sizeof(float));
        return (a > b) - (a < b);
    }
    return memcmp(newPK, exPK, (size_t)newRec->primaryKeyLength);
}

DataRecord generateDataRecord(FILE *dataDictionary, long attributesHeader) {
    long currentAttributeDir;
    DataRecord newDataRecord;
 
    newDataRecord.data = malloc(1);
    newDataRecord.primaryKeyOffset = -1;
    newDataRecord.primaryKeyLength = 0;
    newDataRecord.dataLength = 0;
 
    fseek(dataDictionary, attributesHeader, SEEK_SET);
    fread(&currentAttributeDir, sizeof(long), 1, dataDictionary);
 
    while (currentAttributeDir != NULL_POINTER) {
        Attribute currentAttribute;
        fseek(dataDictionary, currentAttributeDir, SEEK_SET);
        fread(&currentAttribute, sizeof(Attribute), 1, dataDictionary);
 
        switch (currentAttribute.type) {
 
            case Integer: {
                int *intData = (int *)malloc(sizeof(int));
                printf("  Valor para '%s' (Integer, %d bytes): ", currentAttribute.name, currentAttribute.length);
                scanf("%d", intData);
                while (getchar() != '\n');
 
                newDataRecord.data = realloc(newDataRecord.data, newDataRecord.dataLength + sizeof(int));
                memcpy((char *)newDataRecord.data + newDataRecord.dataLength, intData, sizeof(int));
 
                if (currentAttribute.isPrimaryKey == 'Y') {
                    newDataRecord.primaryKeyOffset = newDataRecord.dataLength;
                    newDataRecord.primaryKeyLength = sizeof(int);
                }
                free(intData);
                newDataRecord.dataLength += currentAttribute.length;
                break;
            }
 
            case Decimal: {
                float *floatData = (float *)malloc(sizeof(float));
                printf("  Valor para '%s' (Decimal, %d bytes): ", currentAttribute.name, currentAttribute.length);
                scanf("%f", floatData);
                while (getchar() != '\n');
 
                newDataRecord.data = realloc(newDataRecord.data, newDataRecord.dataLength + sizeof(float));
                memcpy((char *)newDataRecord.data + newDataRecord.dataLength, floatData, sizeof(float));
 
                if (currentAttribute.isPrimaryKey == 'Y') {
                    newDataRecord.primaryKeyOffset = newDataRecord.dataLength;
                    newDataRecord.primaryKeyLength = sizeof(float);
                }
                free(floatData);
                newDataRecord.dataLength += currentAttribute.length;
                break;
            }
 
            case Character: {
                char *charData = (char *)malloc(sizeof(char));
                printf("  Valor para '%s' (Character, 1 byte): ", currentAttribute.name);
                scanf(" %c", charData);
                while (getchar() != '\n');
 
                newDataRecord.data = realloc(newDataRecord.data, newDataRecord.dataLength + sizeof(char));
                memcpy((char *)newDataRecord.data + newDataRecord.dataLength, charData, sizeof(char));
 
                if (currentAttribute.isPrimaryKey == 'Y') {
                    newDataRecord.primaryKeyOffset = newDataRecord.dataLength;
                    newDataRecord.primaryKeyLength = sizeof(char);
                }
                free(charData);
                newDataRecord.dataLength += currentAttribute.length;
                break;
            }

            case String:
            {
                char *stringData = (char *)malloc(currentAttribute.length + 1);
                memset(stringData, 0, currentAttribute.length + 1);
                printf("  Valor para '%s' (String, %d bytes): ", currentAttribute.name, currentAttribute.length);
                scanf(" %[^\n]", stringData);
                while (getchar() != '\n' && !feof(stdin));

                newDataRecord.data = realloc(newDataRecord.data, newDataRecord.dataLength + currentAttribute.length);
                memcpy((char *)newDataRecord.data + newDataRecord.dataLength, stringData, currentAttribute.length);

                if (currentAttribute.isPrimaryKey == 'Y'){
                    newDataRecord.primaryKeyOffset = newDataRecord.dataLength;
                    newDataRecord.primaryKeyLength = currentAttribute.length;
                }
                free(stringData);
                newDataRecord.dataLength += currentAttribute.length;
                break;
            }

            default:
                printf("Tipo de atributo desconocido.\n");
                break;
        }
 
        currentAttributeDir = currentAttribute.nextAttribute;
    }
    long nextPtr = NULL_POINTER;
    newDataRecord.data = realloc(newDataRecord.data, newDataRecord.dataLength + sizeof(long));
    memcpy((char *)newDataRecord.data + newDataRecord.dataLength, &nextPtr, sizeof(long));
    newDataRecord.dataLength += sizeof(long);
 
    return newDataRecord;
}


long appendDataRecord(FILE *dataDictionary, DataRecord dataRecord) {
    fseek(dataDictionary, 0, SEEK_END);
    long offset = ftell(dataDictionary);
 
    if (fwrite(dataRecord.data, dataRecord.dataLength, 1, dataDictionary) != 1) {
        printf("Error: No se pudo escribir el registro en el archivo.\n");
        return NULL_POINTER;
    }
 
    return offset;
}

int createDataRecord(FILE *dataDictionary, long attributesHeader, long dataRecordsHeader) {
    DataRecord newDataRecord = generateDataRecord(dataDictionary, attributesHeader);
 
    if (newDataRecord.primaryKeyOffset != -1) {
        if (pkExists(dataDictionary, dataRecordsHeader, &newDataRecord)) {
            printf("Error: Ya existe un registro con esa clave primaria.\n");
            free(newDataRecord.data);
            return 0;
        }
    }

    long currentDataRecordDir;
    long currentDataRecordPtr = dataRecordsHeader;
    int  sortingCriteriaMet = 0;
 
    fseek(dataDictionary, currentDataRecordPtr, SEEK_SET);
    if (fread(&currentDataRecordDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo leer la cabecera de registros.\n");
        free(newDataRecord.data);
        return 0;
    }
 
    while (currentDataRecordDir != NULL_POINTER && !sortingCriteriaMet) {
        void *currentBlock = malloc(newDataRecord.dataLength);
        if (!currentBlock) {
            printf("Error: Memoria insuficiente.\n");
            free(newDataRecord.data);
            return 0;
        }
 
        fseek(dataDictionary, currentDataRecordDir, SEEK_SET);
        fread(currentBlock, newDataRecord.dataLength, 1, dataDictionary);
 
        if (newDataRecord.primaryKeyOffset != -1 && comparePK(&newDataRecord, currentBlock) < 0) {
            sortingCriteriaMet = 1;
        } else {
            currentDataRecordPtr = currentDataRecordDir + newDataRecord.dataLength - sizeof(long);
            long nextDir;
            memcpy(&nextDir, (char *)currentBlock + newDataRecord.dataLength - sizeof(long), sizeof(long));
            currentDataRecordDir = nextDir;
        }
        free(currentBlock);
    }
 
    long dataRecordDir = appendDataRecord(dataDictionary, newDataRecord);
    if (dataRecordDir == NULL_POINTER) {
        free(newDataRecord.data);
        return 0;
    }
 
    fseek(dataDictionary, currentDataRecordPtr, SEEK_SET);
    if (fwrite(&dataRecordDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace anterior.\n");
        free(newDataRecord.data);
        return 0;
    }
 
    long nextPtr = dataRecordDir + newDataRecord.dataLength - sizeof(long);
    fseek(dataDictionary, nextPtr, SEEK_SET);
    if (fwrite(&currentDataRecordDir, sizeof(long), 1, dataDictionary) != 1) {
        printf("Error: No se pudo actualizar el enlace siguiente.\n");
        free(newDataRecord.data);
        return 0;
    }
 
    free(newDataRecord.data);
    printf("Registro insertado correctamente.\n");
    return 1;
}

void printDataRecords(FILE *dataDictionary, long attributesHeader, long dataRecordsHeader) {
    long attrDir;
    fseek(dataDictionary, attributesHeader, SEEK_SET);
    fread(&attrDir, sizeof(long), 1, dataDictionary);
 
    if (attrDir == NULL_POINTER) {
        printf("Esta entidad no tiene atributos definidos.\n");
        return;
    }

    Attribute attrs[64];
    int attrCount = 0;
    while (attrDir != NULL_POINTER && attrCount < 64) {
        fseek(dataDictionary, attrDir, SEEK_SET);
        fread(&attrs[attrCount], sizeof(Attribute), 1, dataDictionary);
        attrDir = attrs[attrCount].nextAttribute;
        attrCount++;
    }
 
    long currentDir;
    fseek(dataDictionary, dataRecordsHeader, SEEK_SET);
    fread(&currentDir, sizeof(long), 1, dataDictionary);
 
    if (currentDir == NULL_POINTER) {
        printf("No hay registros en esta entidad.\n");
        return;
    }

    int dataLength = 0;
    for (int i = 0; i < attrCount; i++)
        dataLength += attrs[i].length;
    dataLength += sizeof(long);
    for (int i = 0; i < attrCount; i++)
        printf("%-20s ", attrs[i].name);
    printf("\n");
    for (int i = 0; i < attrCount; i++)
        printf("%-20s ", "────────────────────");
    printf("\n");
 
    int count = 0;
    while (currentDir != NULL_POINTER) {
        void *block = malloc(dataLength);
        if (!block) break;
 
        fseek(dataDictionary, currentDir, SEEK_SET);
        fread(block, dataLength, 1, dataDictionary);
 
        int offset = 0;
        for (int i = 0; i < attrCount; i++) {
            switch (attrs[i].type) {
                case Integer: {
                    int val;
                    memcpy(&val, (char *)block + offset, sizeof(int));
                    printf("%-20d ", val);
                    break;
                }
                case Decimal: {
                    float val;
                    memcpy(&val, (char *)block + offset, sizeof(float));
                    printf("%-20.2f ", val);
                    break;
                }
                case Character: {
                    char val;
                    memcpy(&val, (char *)block + offset, sizeof(char));
                    printf("%-20c ", val);
                    break;
                }
                case String: {
                    char val[MAX_CHARS + 1];
                    memset(val, 0, sizeof(val));
                    memcpy(val, (char *)block + offset, attrs[i].length);
                    printf("%-20s ", val);
                    break;
                }
                default:
                    printf("%-20s ", "?");
                    break;
            }
            offset += attrs[i].length;
        }
        printf("\n");
 
        long nextDir;
        memcpy(&nextDir, (char *)block + dataLength - sizeof(long), sizeof(long));
        free(block);
        currentDir = nextDir;
        count++;
    }
    printf("\nTotal de registros: %d\n", count);
}