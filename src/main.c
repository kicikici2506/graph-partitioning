#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/graph.h"

void print_usage(const char* program_name) {
    printf("Użycie: %s <plik_wejściowy> [liczba_części] [margines_procentowy]\n", program_name);
    printf("  plik_wejściowy     - ścieżka do pliku z grafem wejściowym\n");
    printf("  liczba_części      - liczba części na które podzielić graf (domyślnie: 2)\n");
    printf("  margines_procentowy - maksymalna dozwolona różnica wielkości między częściami w %% (domyślnie: 10.0)\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Parametry domyślne
    const char* input_file = argv[1];
    int num_parts = 2;
    double margin_percentage = 10.0;
    bool binary_output = false;

    // Parsowanie argumentów
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0) {
            binary_output = true;
        } else if (i == 2 && argv[i][0] != '-') {
            num_parts = atoi(argv[i]);
            if (num_parts <= 0) {
                fprintf(stderr, "Błąd: Liczba części musi być dodatnia\n");
                return 1;
            }
        } else if (i == 3 && argv[i][0] != '-') {
            margin_percentage = atof(argv[i]);
            if (margin_percentage < 0) {
                fprintf(stderr, "Błąd: Margines procentowy nie może być ujemny\n");
                return 1;
            }
        }
    }

    // Wczytaj graf
    Graph* graph = NULL;
    if (load_graph_from_file(input_file, &graph) != 0) {
        fprintf(stderr, "Błąd: Nie udało się wczytać grafu z pliku: %s\n", input_file);
        return 1;
    }

    // Wyświetl informacje o grafie
    print_graph_info(graph);

    // Podziel graf
    VertexGroup* groups = NULL;
    if (divide_graph(graph, num_parts, margin_percentage, &groups) != 0) {
        fprintf(stderr, "Błąd: Nie udało się podzielić grafu\n");
        destroy_graph(graph);
        return 1;
    }

    // Sprawdź różnicę rozmiaru między grupami
    double size_diff = calculate_size_difference(groups, num_parts);
    if (size_diff > margin_percentage) {
        printf("Uwaga: Różnica wielkości (%.2f%%) przekracza określony margines (%.2f%%)\n",
               size_diff, margin_percentage);
    }

    // Oblicz liczbę krawędzi między grupami
    int cross_edges = calculate_edges_between_groups(graph, groups, num_parts);

    // Wyświetl informacje o podziale
    print_division_info(groups, num_parts);
    printf("Liczba krawędzi między grupami: %d\n", cross_edges);
    printf("Różnica wielkości między grupami: %.2f%%\n", size_diff);

    // Zapisz wynik do pliku
    const char* output_file = binary_output ? "graph_division.bin" : "graph_division.txt";
    if (save_graph_division(output_file, graph, groups, num_parts, binary_output) != 0) {
        fprintf(stderr, "Uwaga: Nie udało się zapisać podziału do pliku: %s\n", output_file);
    } else {
        printf("Podział zapisano do pliku: %s\n", output_file);
    }

    // Zwolnij pamięć
    for (int i = 0; i < num_parts; i++) {
        free(groups[i].vertices);
    }
    free(groups);
    destroy_graph(graph);

    return 0;
}