#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/graph.h"

void print_usage(const char* program_name) {
    printf("Usage: %s <input_file> [num_parts] [margin_percentage] [-b]\n", program_name);
    printf("  input_file        - path to the input graph file\n");
    printf("  num_parts         - number of parts to divide the graph into (default: 2)\n");
    printf("  margin_percentage - maximum allowed size difference between parts in %% (default: 10.0)\n");
    printf("  -b               - output in binary format (optional)\n");
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
                fprintf(stderr, "Error: Number of parts must be positive\n");
                return 1;
            }
        } else if (i == 3 && argv[i][0] != '-') {
            margin_percentage = atof(argv[i]);
            if (margin_percentage < 0) {
                fprintf(stderr, "Error: Margin percentage must be non-negative\n");
                return 1;
            }
        }
    }

    // Wczytaj graf
    Graph* graph = NULL;
    if (load_graph_from_file(input_file, &graph) != 0) {
        fprintf(stderr, "Error: Failed to load graph from file: %s\n", input_file);
        return 1;
    }

    // Wyświetl informacje o grafie
    print_graph_info(graph);

    // Podziel graf
    VertexGroup* groups = NULL;
    if (divide_graph(graph, num_parts, margin_percentage, &groups) != 0) {
        fprintf(stderr, "Error: Failed to divide graph\n");
        destroy_graph(graph);
        return 1;
    }

    // Sprawdź różnicę rozmiaru między grupami
    double size_diff = calculate_size_difference(groups, num_parts);
    if (size_diff > margin_percentage) {
        printf("Warning: Size difference (%.2f%%) exceeds specified margin (%.2f%%)\n",
               size_diff, margin_percentage);
    }

    // Oblicz liczbę krawędzi między grupami
    int cross_edges = calculate_edges_between_groups(graph, groups, num_parts);

    // Wyświetl informacje o podziale
    print_division_info(groups, num_parts);
    printf("Cross-edges between groups: %d\n", cross_edges);
    printf("Size difference between groups: %.2f%%\n", size_diff);

    // Zapisz wynik do pliku
    const char* output_file = binary_output ? "graph_division.bin" : "graph_division.txt";
    if (save_graph_division(output_file, graph, groups, num_parts, binary_output) != 0) {
        fprintf(stderr, "Warning: Failed to save division to file: %s\n", output_file);
    } else {
        printf("Division saved to: %s\n", output_file);
    }

    // Zwolnij pamięć
    for (int i = 0; i < num_parts; i++) {
        free(groups[i].vertices);
    }
    free(groups);
    destroy_graph(graph);

    return 0;
}