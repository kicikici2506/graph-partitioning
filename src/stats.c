#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

// Funkcje pomocnicze do wyświetlania informacji
void print_graph_info(const Graph* graph) {
    if (!graph) return;

    printf("\nInformacje o grafie:\n");
    printf("----------------\n");
    printf("Całkowita liczba wierzchołków: %d\n", graph->total_vertices);
    printf("Maksymalna liczba wierzchołków w wierszu: %d\n", graph->max_vertices);
    printf("Liczba wierszy: %d\n", graph->num_rows);
    
    // Wyświetl informacje o wierszach
    printf("\nStruktura wierszy:\n");
    for (int i = 0; i < graph->num_rows; i++) {
        int start = graph->row_pointers[i];
        int end = (i < graph->num_rows - 1) ? graph->row_pointers[i + 1] : graph->total_vertices;
        printf("Wiersz %d (wierzchołki %d-%d):", i + 1, start + 1, end);
        for (int j = start; j < end; j++) {
            printf(" %d", graph->vertex_indices[j]);
        }
        printf("\n");
    }

    // Wyświetl statystyki połączeń
    int total_edges = 0;
    int max_degree = 0;
    double avg_degree = 0.0;

    for (int i = 0; i < graph->total_vertices; i++) {
        int degree = graph->adj_list[i].count;
        total_edges += degree;
        if (degree > max_degree) {
            max_degree = degree;
        }
    }
    total_edges /= 2; // Każda krawędź była liczona dwukrotnie
    avg_degree = (double)total_edges * 2 / graph->total_vertices;

    printf("\nStatystyki połączeń:\n");
    printf("Całkowita liczba krawędzi: %d\n", total_edges);
    printf("Maksymalny stopień wierzchołka: %d\n", max_degree);
    printf("Średni stopień wierzchołka: %.2f\n", avg_degree);
}

void print_division_info(const VertexGroup* groups, int num_groups) {
    if (!groups || num_groups <= 0) return;

    printf("\nInformacje o podziale:\n");
    printf("-------------------\n");
    
    // Statystyki grup
    int min_size = groups[0].count;
    int max_size = groups[0].count;
    double avg_size = 0.0;
    
    for (int i = 0; i < num_groups; i++) {
        int size = groups[i].count;
        if (size < min_size) min_size = size;
        if (size > max_size) max_size = size;
        avg_size += size;
    }
    avg_size /= num_groups;

    // Wyświetl informacje o każdej grupie
    for (int i = 0; i < num_groups; i++) {
        printf("\nGrupa %d:\n", i + 1);
        printf("  Rozmiar: %d wierzchołków\n", groups[i].count);
        printf("  Indeks pierwszego wierzchołka: %d\n", groups[i].first_vertex);
        printf("  Wierzchołki:");
        
        // Wyświetl maksymalnie 10 pierwszych wierzchołków w grupie
        int display_count = groups[i].count > 10 ? 10 : groups[i].count;
        for (int j = 0; j < display_count; j++) {
            printf(" %d", groups[i].vertices[j]);
        }
        if (groups[i].count > 10) {
            printf(" ... (pozostałe %d)", groups[i].count - 10);
        }
        printf("\n");
    }

    // Wyświetl statystyki podziału
    printf("\nStatystyki podziału:\n");
    printf("Minimalna wielkość grupy: %d wierzchołków\n", min_size);
    printf("Maksymalna wielkość grupy: %d wierzchołków\n", max_size);
    printf("Średnia wielkość grupy: %.2f wierzchołków\n", avg_size);
    printf("Różnica wielkości: %.2f%%\n", ((double)(max_size - min_size) / min_size) * 100.0);
}

// Funkcja obliczająca różnicę wielkości między grupami
double calculate_size_difference(const VertexGroup* groups, int num_groups) {
    if (!groups || num_groups <= 1) return 0.0;

    int min_size = groups[0].count;
    int max_size = groups[0].count;

    for (int i = 1; i < num_groups; i++) {
        if (groups[i].count < min_size) min_size = groups[i].count;
        if (groups[i].count > max_size) max_size = groups[i].count;
    }

    return ((double)(max_size - min_size) / min_size) * 100.0;
}

// Funkcja obliczająca liczbę krawędzi między grupami
int calculate_edges_between_groups(const Graph* graph, const VertexGroup* groups, int num_groups) {
    if (!graph || !groups || num_groups <= 1) return 0;

    int cross_edges = 0;

    // Dla każdej grupy
    for (int g1 = 0; g1 < num_groups; g1++) {
        // Dla każdego wierzchołka w grupie
        for (int i = 0; i < groups[g1].count; i++) {
            int vertex = groups[g1].vertices[i];
            AdjacencyList* adj = &graph->adj_list[vertex];

            // Dla każdego sąsiada tego wierzchołka
            for (int j = 0; j < adj->count; j++) {
                int neighbor = adj->neighbors[j];
                
                // Sprawdź, czy sąsiad jest w innej grupie
                bool is_cross_edge = true;
                for (int k = 0; k < groups[g1].count; k++) {
                    if (groups[g1].vertices[k] == neighbor) {
                        is_cross_edge = false;
                        break;
                    }
                }

                if (is_cross_edge) {
                    cross_edges++;
                }
            }
        }
    }

    // Dzielimy przez 2, bo każda krawędź została policzona dwukrotnie
    return cross_edges / 2;
}