#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Użycie: %s <plik_wejściowy.csrrg> <liczba_części>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    int num_parts = atoi(argv[2]);
    
    if (num_parts <= 0) {
        printf("Liczba części musi być większa od 0\n");
        return 1;
    }

    // Wczytaj graf
    Graph* graph = NULL;
    if (load_graph_from_file(input_file, &graph) != 0) {
        printf("Błąd podczas wczytywania pliku %s\n", input_file);
        return 1;
    }

    // Wyświetl informacje o grafie
    printf("Wczytano graf:\n");
    print_graph_info(graph);

    // Podziel graf
    VertexGroup* groups = NULL;
    if (divide_graph(graph, num_parts, 20.0, &groups) != 0) {
        printf("Błąd podczas dzielenia grafu\n");
        destroy_graph(graph);
        return 1;
    }

    // Wyświetl informacje o podziale
    print_division_info(groups, num_parts);

    // Zapisz wynik do pliku
    const char* output_file = "output.txt";
    if (save_graph_division(output_file, graph, groups, num_parts, false) != 0) {
        printf("Błąd podczas zapisywania wyniku do pliku %s\n", output_file);
    } else {
        printf("\nWynik zapisano do pliku: %s\n", output_file);
    }

    // Zwolnij pamięć
    for (int i = 0; i < num_parts; i++) {
        free(groups[i].vertices);
    }
    free(groups);
    destroy_graph(graph);

    return 0;
}