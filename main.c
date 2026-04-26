#include "data_dictionary.h"
 
// ─────────────────────────────────────────────
//  Submenú de entidades
// ─────────────────────────────────────────────
 
void entityMenu(FILE *file) {
    EntityMenuChoice opcion;
    char entityName[MAX_CHARS];
 
    do {
        printf("│        GESTIÓN DE ENTIDADES           \n");
        printf("  [%d] Insertar entidad                 \n", INSERT_ENTITY);
        printf("  [%d] Listar entidades                 \n", LIST_ENTITIES);
        printf("  [%d] Eliminar entidad                 \n", DELETE_ENTITY);
        printf("  [%d] Volver al menú principal         \n", ENTITY_MENU_EXIT);
        printf("Selecciona una opción: ");
 
        int raw;
        scanf("%d", &raw);
        opcion = (EntityMenuChoice)raw;
 
        switch (opcion) {
 
            case INSERT_ENTITY:
                printf("\nNombre de la nueva entidad: ");
                scanf("%49s", entityName);
                createEntity(file, entityName);
                break;
 
            case LIST_ENTITIES:
                printEntities(file);
                break;
 
            case DELETE_ENTITY:
                printf("\nNombre de la entidad a eliminar: ");
                scanf("%49s", entityName);
                removeEntity(file, entityName);
                break;
 
            case ENTITY_MENU_EXIT:
                printf("Volviendo al menú principal...\n");
                break;
 
            default:
                printf("Opción no válida. Intenta de nuevo.\n");
                break;
        }
 
    } while (opcion != ENTITY_MENU_EXIT);
}
 
int main() {
    MenuChoice opcion;
    char fileName[MAX_CHARS];
    FILE *file = NULL;
 
    printf("║     DICCIONARIO DE DATOS EN C        ║\n");
 
    do {
        printf("  [%d] Crear diccionario                \n", CREATE_DATA_DICTIONARY);
        printf("  [%d] Abrir diccionario existente      \n", OPEN_DATA_DICTIONARY);
        printf("  [%d] Salir                            \n", EXIT);
        printf("Selecciona una opción: ");
 
        int raw;
        scanf("%d", &raw);
        opcion = (MenuChoice)raw;
 
        switch (opcion) {
 
            case CREATE_DATA_DICTIONARY:
                printf("\nNombre del nuevo diccionario: ");
                scanf("%49s", fileName);
                if (createDataDictionary(fileName))
                    printf("Listo. Ya puedes abrirlo %d.\n",
                           OPEN_DATA_DICTIONARY);
                printf("\n");
                break;
 
            case OPEN_DATA_DICTIONARY:
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
 
            case EXIT:
                printf("Fin del programa\n");
                break;
 
            default:
                printf("Opción no válida. Intenta de nuevo.\n\n");
                break;
        }
 
    } while (opcion != EXIT);
 
    return 0;
}
