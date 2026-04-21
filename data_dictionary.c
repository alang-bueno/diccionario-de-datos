#include "data_dictionary.h"

/* ────────────────────────────────────────────────────────────────
 *  createDataDictionary
 *  Crea un archivo binario nuevo e inicializa la cabecera con -1
 *  (lista de entidades vacía).
 *
 *  Mejoras sobre la versión base del profesor:
 *    1. Detecta si el archivo ya existe y pide confirmación antes
 *       de sobrescribirlo.
 *    2. Valida que el nombre no sea NULL ni cadena vacía.
 *
 *  Retorna: 1 = éxito, 0 = error o cancelado por el usuario.
 * ──────────────────────────────────────────────────────────────── */
int createDataDictionary(const char *fileName) {

    /* ── Validación básica del nombre ── */
    if (fileName == NULL || fileName[0] == '\0') {
        printf("Error: El nombre del archivo no puede estar vacío.\n");
        return 0;
    }

    /* ── ¿Ya existe el archivo? ── */
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

    /* ── Crear / sobrescribir el archivo ── */
    FILE *file = fopen(fileName, "wb+");
    if (file == NULL) {
        printf("Error: No se pudo crear el archivo '%s'.\n", fileName);
        return 0;
    }

    /* Cabecera: puntero a primera entidad = -1 (lista vacía) */
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
