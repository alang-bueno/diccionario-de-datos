#include "data_dictionary.h"

void entityMenu(FILE *file) {
    int  opcion = 0;
    char entityName[MAX_CHARS];
 
    do {
        printf("        GESTIÓN DE ENTIDADES      \n");
        printf("  [1] Insertar entidad            \n");
        printf("  [2] Listar entidades            \n");
        printf("  [3] Volver al menú principal    \n");
        printf("Selecciona una opción: ");
        scanf("%d", &opcion);
 
        switch (opcion) {
 
            case 1:
                printf("\nNombre de la nueva entidad: ");
                scanf("%49s", entityName);
                createEntity(file, entityName);
                break;
 
            case 2:
                printEntities(file);
                break;
 
            case 3:
                printf("Volviendo al menú principal...\n");
                break;
 
            default:
                printf("Opción no válida. Intenta de nuevo.\n");
                break;
        }
 
    } while (opcion != 3);
}

int main(void) {
    int  opcion   = 0;
    char fileName[MAX_CHARS];
    FILE *file    = NULL;

    printf("║     DICCIONARIO DE DATOS     ║\n");

    do {
        printf("  [1] Crear diccionario                \n");
        printf("  [2] Abrir diccionario existente      \n");
        printf("  [3] Salir                            \n");
        printf("Selecciona una opción: ");
        scanf("%d", &opcion);
 
        switch (opcion) {
            case 1:
                printf("\nNombre del nuevo diccionario: ");
                scanf("%49s", fileName);
 
                if (createDataDictionary(fileName)) {
                    printf("Listo. Ya puedes abrirlo con la opción 2.\n");
                }
                printf("\n");
                break;

            case 2:
                printf("\nNombre del diccionario a abrir: ");
                scanf("%49s", fileName);
 
                file = openDataDictionary(fileName);
 
                if (file != NULL) {
                   entityMenu(file);
                   fclose(file);
                   file = NULL;
                }
                
                printf("\n");
                break;
 
            case 3:
                printf("Fin del programa\n");
                break;

            default:
                printf("Opción no válida. Intenta de nuevo.\n\n");
                break;
        }
 
    } while (opcion != 3);
 
    return 0;
}
