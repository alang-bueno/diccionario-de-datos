#include "data_dictionary.h"

static int readMenuOption(void) {
    int val;
    while (scanf("%d", &val) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Entrada inválida. Ingresa un número: ");
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return val;
}

static void closeDictionary(FILE **file) {
    printf("\nError crítico de E/S: el diccionario puede estar dañado.\n");
    printf("Cerrando el archivo para prevenir mayor corrupción.\n");
    fclose(*file);
    *file = NULL;
}

static AttributeType askAttributeType(void) {
    printf("  Tipos disponibles:\n");
    printf("    [%d] Integer   (4 bytes)\n",   Integer);
    printf("    [%d] Decimal   (8 bytes)\n",   Decimal);
    printf("    [%d] Character (1 byte)\n",    Character);
    printf("    [%d] String    (hasta %d bytes)\n", String, MAX_CHARS);
    printf("  Tipo: ");
    return (AttributeType)readMenuOption();
}

static int askAttributeLength(AttributeType type) {
    if (type == Integer)   return 4;
    if (type == Decimal)   return 8;
    if (type == Character) return 1;

    int length;
    printf("  Length (máximo %d, 0 = usar %d): ", MAX_CHARS, MAX_CHARS);
    length = readMenuOption();
    return (length <= 0 || length > MAX_CHARS) ? MAX_CHARS : length;
}

void recordMenu(FILE **file, Entity *entity, long entityOffset) {
    RecordMenuChoice opcion;
    long dataRecordsHeader = entityOffset + (long)offsetof(Entity, dataPointer);
    long attributesHeader  = entityOffset + (long)offsetof(Entity, attributesPointer);

    do {
        printf("   REGISTROS DE: %-21s│\n", entity->name);
        printf("  [%d] Insertar registro               \n", INSERT_RECORD);
        printf("  [%d] Listar registros                \n", LIST_RECORDS);
        printf("  [%d] Modificar registro              \n", MODIFY_RECORD);
        printf("  [%d] Eliminar registro               \n", DELETE_RECORD);
        printf("  [%d] Volver                          \n", RECORD_MENU_EXIT);
        printf("Selecciona una opción: ");

        opcion = (RecordMenuChoice)readMenuOption();

        switch (opcion) {

            case INSERT_RECORD:
                if (!hasAttributes(*file, attributesHeader)) {
                    printf("Error: '%s' no tiene atributos definidos.\n"
                           "Define al menos un atributo antes de insertar registros.\n", entity->name);
                    break;
                }
                printf("\n--- Ingresa los valores del nuevo registro ---\n");
                if (createDataRecord(*file, attributesHeader, dataRecordsHeader) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = RECORD_MENU_EXIT;
                }
                break;

            case LIST_RECORDS:
                printDataRecords(*file, attributesHeader, dataRecordsHeader);
                break;

            case MODIFY_RECORD:
                printf("\n--- Modificar registro ---\n");
                if (modifyDataRecord(*file, attributesHeader, dataRecordsHeader) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = RECORD_MENU_EXIT;
                }
                break;

            case DELETE_RECORD:
                if (removeDataRecord(*file, attributesHeader, dataRecordsHeader) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = RECORD_MENU_EXIT;
                }
                break;

            case RECORD_MENU_EXIT:
                printf("Volviendo al menú de entidades...\n");
                break;

            default:
                printf("Opción no válida.\n");
                break;
        }

    } while (opcion != RECORD_MENU_EXIT && *file != NULL);
}

void attributeMenu(FILE **file, Entity *entity, long entityOffset) {
    AttributeMenuChoice opcion;
    long attributesHeader  = entityOffset + (long)offsetof(Entity, attributesPointer);
    long dataRecordsHeader = entityOffset + (long)offsetof(Entity, dataPointer);

    do {
        printf("   ATRIBUTOS DE: %-21s│\n", entity->name);
        printf("──────────────────────────────────────\n");
        printf("  [%d] Insertar atributo               \n", INSERT_ATTRIBUTE);
        printf("  [%d] Listar atributos                \n", LIST_ATTRIBUTES);
        printf("  [%d] Eliminar atributo               \n", DELETE_ATTRIBUTE);
        printf("  [%d] Modificar atributo              \n", MODIFY_ATTRIBUTE);
        printf("  [%d] Volver                          \n", ATTRIBUTE_MENU_EXIT);
        printf("Selecciona una opción: ");

        opcion = (AttributeMenuChoice)readMenuOption();

        switch (opcion) {

        case INSERT_ATTRIBUTE: {
            if (hasDataRecords(*file, dataRecordsHeader)) {
                printf("Error: La entidad '%s' tiene registros de datos.\n"
                       "No se pueden agregar atributos mientras existan registros.\n", entity->name);
                break;
            }
            Attribute attr;
            memset(&attr, 0, sizeof(Attribute));
            attr.nextAttribute = NULL_POINTER;

            printf("\n  Nombre del atributo: ");
            scanf("%49s", attr.name);

            attr.type   = askAttributeType();
            attr.length = askAttributeLength(attr.type);

            if (hasPrimaryKey(*file, attributesHeader)) {
                attr.isPrimaryKey = 'N';
            } else {
                char pk;
                printf("  ¿Es llave primaria? (Y/N): ");
                scanf(" %c", &pk);
                attr.isPrimaryKey = (pk == 'y' || pk == 'Y') ? 'Y' : 'N';
            }

            int r = createAttribute(*file, attributesHeader, attr);
            if (r == DD_SUCCESS) {
                fseek(*file, attributesHeader, SEEK_SET);
                fread(&entity->attributesPointer, sizeof(long), 1, *file);
            } else if (r == DD_FATAL) {
                closeDictionary(file);
                opcion = ATTRIBUTE_MENU_EXIT;
            }
            break;
        }

            case LIST_ATTRIBUTES:
                printAttributes(*file, attributesHeader);
                break;

            case DELETE_ATTRIBUTE: {
                if (hasDataRecords(*file, dataRecordsHeader)) {
                    printf("Error: La entidad '%s' tiene registros de datos.\n"
                           "No se pueden eliminar atributos mientras existan registros.\n",
                           entity->name);
                    break;
                }
                char attrName[MAX_CHARS];
                printf("\nNombre del atributo a eliminar: ");
                scanf("%49s", attrName);
                if (removeAttribute(*file, attributesHeader, attrName) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = ATTRIBUTE_MENU_EXIT;
                }
                break;
            }

            case MODIFY_ATTRIBUTE: {
                if (hasDataRecords(*file, dataRecordsHeader)) {
                    printf("Error: La entidad '%s' tiene registros de datos.\n"
                           "No se pueden modificar atributos mientras existan registros.\n",
                           entity->name);
                    break;
                }
                char attrName[MAX_CHARS];
                printf("\nNombre del atributo a modificar: ");
                scanf("%49s", attrName);
                if (modifyAttribute(*file, attributesHeader, attrName) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = ATTRIBUTE_MENU_EXIT;
                }
                break;
            }

            case ATTRIBUTE_MENU_EXIT:
                printf("Volviendo al menú de entidades...\n");
                break;

            default:
                printf("Opción no válida.\n");
                break;
        }

    } while (opcion != ATTRIBUTE_MENU_EXIT && *file != NULL);
}

void entityMenu(FILE **file) {
    EntityMenuChoice opcion;
    char entityName[MAX_CHARS];

    do {
        printf("        GESTIÓN DE ENTIDADES           \n");
        printf("  [%d] Insertar entidad                 \n", INSERT_ENTITY);
        printf("  [%d] Listar entidades                 \n", LIST_ENTITIES);
        printf("  [%d] Eliminar entidad                 \n", DELETE_ENTITY);
        printf("  [%d] Gestionar atributos              \n", MANAGE_ATTRIBUTES);
        printf("  [%d] Gestionar registros              \n", MANAGE_RECORDS);
        printf("  [%d] Modificar entidad               \n", MODIFY_ENTITY);
        printf("  [%d] Volver al menú principal         \n", ENTITY_MENU_EXIT);
        printf("Selecciona una opción: ");

        opcion = (EntityMenuChoice)readMenuOption();

        switch (opcion) {

            case INSERT_ENTITY:
                printf("\nNombre de la nueva entidad: ");
                scanf("%49s", entityName);
                if (createEntity(*file, entityName) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = ENTITY_MENU_EXIT;
                }
                break;

            case LIST_ENTITIES:
                printEntities(*file);
                break;

            case DELETE_ENTITY:
                printf("\nNombre de la entidad a eliminar: ");
                scanf("%49s", entityName);
                if (removeEntity(*file, entityName) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = ENTITY_MENU_EXIT;
                }
                break;

            case MANAGE_ATTRIBUTES: {
                printf("\nNombre de la entidad: ");
                scanf("%49s", entityName);

                Entity entity;
                memset(&entity, 0, sizeof(Entity));
                strcpy(entity.name, entityName);

                long offset = findEntity(*file, &entity);
                if (offset == NULL_POINTER) {
                    printf("Error: No se encontró la entidad '%s'.\n", entityName);
                } else {
                    attributeMenu(file, &entity, offset);
                    if (*file == NULL) opcion = ENTITY_MENU_EXIT;
                }
                break;
            }

            case MANAGE_RECORDS: {
                printf("\nNombre de la entidad: ");
                scanf("%49s", entityName);

                Entity entity;
                memset(&entity, 0, sizeof(Entity));
                strcpy(entity.name, entityName);

                long offset = findEntity(*file, &entity);
                if (offset == NULL_POINTER) {
                    printf("Error: No se encontró la entidad '%s'.\n", entityName);
                } else {
                    recordMenu(file, &entity, offset);
                    if (*file == NULL) opcion = ENTITY_MENU_EXIT;
                }
                break;
            }

            case ENTITY_MENU_EXIT:
                printf("Volviendo al menú principal...\n");
                break;

            case MODIFY_ENTITY:
                printf("\nNombre de la entidad a modificar: ");
                scanf("%49s", entityName);
                if (modifyEntity(*file, entityName) == DD_FATAL) {
                    closeDictionary(file);
                    opcion = ENTITY_MENU_EXIT;
                }
                break;

            default:
                printf("Opción no válida. Intenta de nuevo.\n");
                break;
        }

    } while (opcion != ENTITY_MENU_EXIT && *file != NULL);
}


int main() {
    MenuChoice opcion;
    char fileName[MAX_CHARS];
    FILE *file = NULL;

    printf("║     DICCIONARIO DE DATOS        ║\n");

    do {
        printf("  [%d] Crear diccionario                \n", CREATE_DATA_DICTIONARY);
        printf("  [%d] Abrir diccionario existente      \n", OPEN_DATA_DICTIONARY);
        printf("  [%d] Salir                            \n", EXIT);
        printf("Selecciona una opción: ");

        opcion = (MenuChoice)readMenuOption();

        switch (opcion) {

            case CREATE_DATA_DICTIONARY:
                printf("\nNombre del nuevo diccionario: ");
                scanf("%49s", fileName);
                if (createDataDictionary(fileName))
                    printf("Listo. Ya puedes abrirlo con la opción %d.\n",
                           OPEN_DATA_DICTIONARY);
                printf("\n");
                break;

            case OPEN_DATA_DICTIONARY:
                printf("\nNombre del diccionario a abrir: ");
                scanf("%49s", fileName);

                file = openDataDictionary(fileName);
                if (file != NULL) {
                    entityMenu(&file);
                    if (file != NULL) fclose(file);
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
