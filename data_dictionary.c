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
