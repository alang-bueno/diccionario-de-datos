#include "data_dictionary.h"
#include <ctype.h>

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
 
        if (strcmp(entityName, current.name) == 0)
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
 
        if (strcmp(entityName, currentEntity.name) < 0) {
            sortingCriteriaMet = 1;
        } else {
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
 
        if (strcmp(entityName, currentEntity.name) == 0) {
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
        } else {
            currentEntityPtr = currentEntityDir
                               + (long)sizeof(Entity)
                               - (long)sizeof(long);
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
 
        if (strcmp(entity->name, currentEntity.name) == 0) {
            entity->dataPointer       = currentEntity.dataPointer;
            entity->attributesPointer = currentEntity.attributesPointer;
            entity->nextEntity        = currentEntity.nextEntity;
            entityFound = 1;
        } else {
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
            currentAttributePtr = currentAttributeDir
                                  + (long)sizeof(Attribute)
                                  - (long)sizeof(long);
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