#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/graph.h"

// Funkcja wyświetlająca instrukcję użycia programu
void print_usage(const char* program_name) {
    printf("Użycie: %s -i plik_wejściowy.csrrg -o plik_wyjściowy.txt -p liczba_części -m margines [-b]\n\n", program_name);
    printf("Opcje:\n");
    printf("  -i plik_wejściowy   Ścieżka do pliku wejściowego w formacie CSRRG\n");
    printf("  -o plik_wyjściowy   Ścieżka do pliku wyjściowego (domyślnie: output.txt)\n");
    printf("  -p liczba_części    Liczba części na które podzielić graf (domyślnie: 2)\n");
    printf("  -m margines         Maksymalna dozwolona różnica wielkości między częściami w %% (domyślnie: 20)\n");
    printf("  -b                  Zapisz wynik w formacie binarnym\n");
    printf("  -h                  Wyświetl tę pomoc\n");
}

int main(int argc, char *argv[]) {
    // Inicjalizacja zmiennych z wartościami domyślnymi
    const char* input_file = NULL;        // Ścieżka do pliku wejściowego
    const char* output_file = "output.txt"; // Domyślna ścieżka do pliku wyjściowego
    int num_parts = 2;                    // Domyślna liczba części grafu
    double margin_percentage = 20.0;      // Domyślny margines procentowy
    bool binary_output = false;           // Flaga określająca format wyjściowy
    
    // Parsowanie argumentów wiersza poleceń
    int opt;
    while ((opt = getopt(argc, argv, "hi:o:p:m:b")) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'p':
                num_parts = atoi(optarg);
                if (num_parts <= 0) {
                    fprintf(stderr, "Błąd: Liczba części musi być większa od 0\n");
                    return 1;
                }
                break;
            case 'm':
                margin_percentage = atof(optarg);
                if (margin_percentage < 0) {
                    fprintf(stderr, "Błąd: Margines procentowy nie może być ujemny\n");
                    return 1;
                }
                break;
            case 'b':
                binary_output = true;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Sprawdzenie czy podano wymagany plik wejściowy
    if (!input_file) {
        fprintf(stderr, "Błąd: Nie podano pliku wejściowego (-i)\n");
        print_usage(argv[0]);
        return 1;
    }

    // Wczytanie grafu z pliku
    Graph* graph = NULL;
    if (load_graph_from_file(input_file, &graph) != 0) {
        fprintf(stderr, "Błąd: Nie udało się wczytać grafu z pliku: %s\n", input_file);
        return 1;
    }

    // Wyświetlenie informacji o wczytanym grafie
    printf("Wczytano graf z pliku: %s\n", input_file);
    print_graph_info(graph);

    // Podział grafu na określoną liczbę części
    VertexGroup* groups = NULL;
    if (graph->total_vertices > 1000) {
        printf("\nRozpoczynam podział dużego grafu (%d wierzchołków)...\n", graph->total_vertices);
        printf("To może potrwać kilka minut. Proszę czekać...\n\n");
    }
    
    if (divide_graph(graph, num_parts, margin_percentage, &groups) != 0) {
        fprintf(stderr, "Błąd: Nie udało się podzielić grafu\n");
        destroy_graph(graph);
        return 1;
    }

    // Obliczenie różnicy rozmiaru między grupami
    double size_diff = calculate_size_difference(groups, num_parts);
    if (size_diff > margin_percentage) {
        printf("Uwaga: Różnica wielkości (%.2f%%) przekracza określony margines (%.2f%%)\n",
               size_diff, margin_percentage);
    }

    // Obliczenie liczby krawędzi między grupami
    int cross_edges = calculate_edges_between_groups(graph, groups, num_parts);

    // Wyświetlenie informacji o podziale
    print_division_info(groups, num_parts);
    printf("Liczba krawędzi między grupami: %d\n", cross_edges);
    printf("Różnica wielkości między grupami: %.2f%%\n", size_diff);

    // Zapisanie wyniku podziału do pliku
    if (save_graph_division(output_file, graph, groups, num_parts, binary_output) != 0) {
        fprintf(stderr, "Błąd: Nie udało się zapisać podziału do pliku: %s\n", output_file);
    } else {
        printf("\nPodział zapisano do pliku: %s\n", output_file);
    }

    // Zwolnienie zaalokowanej pamięci
    for (int i = 0; i < num_parts; i++) {
        free(groups[i].vertices);
    }
    free(groups);
    destroy_graph(graph);

    return 0;
}