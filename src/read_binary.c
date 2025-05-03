#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Użycie: %s plik_binarny.bin\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    VertexGroup* groups = NULL;
    int num_groups;

    if (load_graph_division(filename, &groups, &num_groups) != 0) {
        fprintf(stderr, "Błąd: Nie udało się odczytać pliku binarnego\n");
        return 1;
    }

    // Wyświetl informacje o podziale
    printf("Odczytano podział z pliku: %s\n", filename);
    printf("Liczba grup: %d\n", num_groups);
    
    for (int i = 0; i < num_groups; i++) {
        printf("Grupa %d (%d wierzchołków):", i + 1, groups[i].count);
        for (int j = 0; j < groups[i].count; j++) {
            printf(" %d", groups[i].vertices[j]);
        }
        printf("\n");
    }

    // Zwolnij pamięć
    for (int i = 0; i < num_groups; i++) {
        free(groups[i].vertices);
    }
    free(groups);

    return 0;
} 