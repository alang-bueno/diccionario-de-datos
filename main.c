#include "data_dictionary.h"

int main(void) {
    const char *archivo = "mi_diccionario.bin";

    printf("=== Diccionario de Datos ===\n\n");

    int resultado = createDataDictionary(archivo);

    if (resultado) {
        printf("\nEl archivo está listo para recibir entidades y atributos.\n");
    } else {
        printf("\nNo se pudo inicializar el diccionario.\n");
    }

    return 0;
}
