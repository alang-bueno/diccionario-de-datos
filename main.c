#include "data_dictionary.h"
 
static AttributeType askAttributeType(void) {
    int raw;
    printf("  Tipos disponibles:\n");
    printf("    [%d] Integer   (4 bytes)\n",   Integer);
    printf("    [%d] Decimal   (8 bytes)\n",   Decimal);
    printf("    [%d] Character (1 byte)\n",    Character);
    printf("    [%d] String    (hasta %d bytes)\n", String, MAX_CHARS);
    printf("  Tipo: ");
    scanf("%d", &raw);
    return (AttributeType)raw;
}
 
static int askAttributeLength(AttributeType type) {
    int length;
    int defaultLen;
    switch (type) {
        case Integer:   defaultLen = 4;        break;
        case Decimal:   defaultLen = 8;        break;
        case Character: defaultLen = 1;        break;
        default:        defaultLen = MAX_CHARS; break;
    }
    printf("  Length (0 = usar valor por defecto %d): ", defaultLen);
    scanf("%d", &length);
    return (length == 0) ? defaultLen : length;
}
 
void attributeMenu(FILE *file, Entity *entity, long entityOffset) {
    AttributeMenuChoice opcion;                                  
    long attributesHeader = entityOffset + (long)sizeof(entity->name) + (long)sizeof(entity->dataPointer);
 
    do {
        printf("   ATRIBUTOS DE: %-21s│\n", entity->name);
        printf("──────────────────────────────────────\n");
        printf("  [%d] Insertar atributo               \n", INSERT_ATTRIBUTE);
        printf("  [%d] Listar atributos                \n", LIST_ATTRIBUTES);
        printf("  [%d] Volver                          \n", ATTRIBUTE_MENU_EXIT);
        printf("Selecciona una opción: ");
 
        int raw;
        scanf("%d", &raw);
        opcion = (AttributeMenuChoice)raw;
 
        switch (opcion) {
 
            case INSERT_ATTRIBUTE: {
                Attribute attr;
                memset(&attr, 0, sizeof(Attribute));
                attr.nextAttribute = NULL_POINTER;
 
                printf("\n  Nombre del atributo: ");
                scanf("%49s", attr.name);
 
                attr.type   = askAttributeType();
                attr.length = askAttributeLength(attr.type);
 
                char pk;
                printf("  ¿Es llave primaria? (Y/N): ");
                scanf(" %c", &pk);
                attr.isPrimaryKey = (pk == 'y' || pk == 'Y') ? 'Y' : 'N';
 
                if (createAttribute(file, attributesHeader, attr)) {
                    fseek(file, attributesHeader, SEEK_SET);
                    fread(&entity->attributesPointer, sizeof(long), 1, file);
                }
                break;
            }
 
            case LIST_ATTRIBUTES:
                printAttributes(file, attributesHeader);
                break;
 
            case ATTRIBUTE_MENU_EXIT:
                printf("Volviendo al menú de entidades...\n");
                break;
 
            default:
                printf("Opción no válida.\n");
                break;
        }
 
    } while (opcion != ATTRIBUTE_MENU_EXIT);
}
 
void entityMenu(FILE *file) {
    EntityMenuChoice opcion;
    char entityName[MAX_CHARS];
 
    do {
        printf("        GESTIÓN DE ENTIDADES           \n");
        printf("  [%d] Insertar entidad                 \n", INSERT_ENTITY);
        printf("  [%d] Listar entidades                 \n", LIST_ENTITIES);
        printf("  [%d] Eliminar entidad                 \n", DELETE_ENTITY);
        printf("  [%d] Gestionar atributos              \n", MANAGE_ATTRIBUTES);
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
 
            case MANAGE_ATTRIBUTES: {
                printf("\nNombre de la entidad: ");
                scanf("%49s", entityName);
 
                Entity entity;
                memset(&entity, 0, sizeof(Entity));
                strcpy(entity.name, entityName);
 
                long offset = findEntity(file, &entity);
                if (offset == NULL_POINTER) {
                    printf("Error: No se encontró la entidad '%s'.\n",
                           entityName);
                } else {
                    attributeMenu(file, &entity, offset);
                }
                break;
            }
 
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
