#include "data_dictionary.h"

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

