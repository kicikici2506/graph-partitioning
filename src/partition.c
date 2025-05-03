#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

#define INITIAL_CAPACITY 16




// Funkcja porównująca do sortowania zysków
static int compare_gains(const void* a, const void* b) {
    return ((GainInfo*)b)->gain - ((GainInfo*)a)->gain;
}

// Obliczanie kosztu zewnętrznego (D) dla wierzchołka
static int calculate_external_cost(const Graph* graph, int vertex, 
                                 const int* group_vertices, int group_size) {
    int cost = 0;
    AdjacencyList* adj = &graph->adj_list[vertex];
    
    for (int i = 0; i < adj->count; i++) {
        int neighbor = adj->neighbors[i];
        bool in_group = false;
        
        for (int j = 0; j < group_size; j++) {
            if (group_vertices[j] == neighbor) {
                in_group = true;
                break;
            }
        }
        
        if (!in_group) {
            cost++;
        }
    }
    
    return cost;
}

// Obliczanie kosztu wewnętrznego (I) dla wierzchołka
static int calculate_internal_cost(const Graph* graph, int vertex, 
                                 const int* group_vertices, int group_size) {
    int cost = 0;
    AdjacencyList* adj = &graph->adj_list[vertex];
    
    for (int i = 0; i < adj->count; i++) {
        int neighbor = adj->neighbors[i];
        for (int j = 0; j < group_size; j++) {
            if (group_vertices[j] == neighbor) {
                cost++;
                break;
            }
        }
    }
    
    return cost;
}

// Obliczanie zysku dla pary wierzchołków
static int calculate_pair_gain(const Graph* graph, int v1, int v2,
                             const VertexGroup* group1, const VertexGroup* group2) {
    int D1 = calculate_external_cost(graph, v1, group1->vertices, group1->count);
    int D2 = calculate_external_cost(graph, v2, group2->vertices, group2->count);
    int I1 = calculate_internal_cost(graph, v1, group1->vertices, group1->count);
    int I2 = calculate_internal_cost(graph, v2, group2->vertices, group2->count);
    
    // Sprawdź, czy v1 i v2 są połączone
    bool connected = false;
    AdjacencyList* adj = &graph->adj_list[v1];
    for (int i = 0; i < adj->count; i++) {
        if (adj->neighbors[i] == v2) {
            connected = true;
            break;
        }
    }
    
    return D1 + D2 - 2 * I1 - 2 * I2 + (connected ? 2 : 0);
}

// Aktualizacja grup po zamianie wierzchołków
static void swap_vertices(VertexGroup* group1, int idx1, 
                         VertexGroup* group2, int idx2) {
    int temp = group1->vertices[idx1];
    group1->vertices[idx1] = group2->vertices[idx2];
    group2->vertices[idx2] = temp;
}


// Funkcja dzieląca graf na części
int divide_graph(Graph* graph, int num_parts, double margin_percentage, VertexGroup** groups) {
    if (!graph || num_parts <= 0 || margin_percentage < 0 || !groups) return -1;

    // Alokuj pamięć na grupy
    *groups = (VertexGroup*)malloc(num_parts * sizeof(VertexGroup));
    if (!*groups) return -1;

    // Inicjalizuj grupy
    int base_size = graph->total_vertices / num_parts;
    int extra = graph->total_vertices % num_parts;
    int current_vertex = 0;

    for (int i = 0; i < num_parts; i++) {
        (*groups)[i].vertices = (int*)malloc(graph->total_vertices * sizeof(int));
        if (!(*groups)[i].vertices) {
            for (int j = 0; j < i; j++) free((*groups)[j].vertices);
            free(*groups);
            return -1;
        }

        int group_size = base_size + (i < extra ? 1 : 0);
        (*groups)[i].count = group_size;
        (*groups)[i].first_vertex = current_vertex;
        
        for (int j = 0; j < group_size; j++) {
            (*groups)[i].vertices[j] = current_vertex++;
        }
    }

    // Alokuj pamięć na struktury pomocnicze - zoptymalizowana wersja
    int max_batch_size = 1000; // Maksymalny rozmiar partii do przetwarzania
    int batch_size = graph->total_vertices < max_batch_size ? graph->total_vertices : max_batch_size;
    
    GainInfo* gains1 = (GainInfo*)malloc(batch_size * sizeof(GainInfo));
    GainInfo* gains2 = (GainInfo*)malloc(batch_size * sizeof(GainInfo));
    bool* locked = (bool*)calloc(graph->total_vertices, sizeof(bool));
    
    if (!gains1 || !gains2 || !locked) {
        free(gains1);
        free(gains2);
        free(locked);
        for (int i = 0; i < num_parts; i++) free((*groups)[i].vertices);
        free(*groups);
        return -1;
    }

    // Iteracyjna optymalizacja podziału
    bool improved;
    // Dostosuj liczbę przejść do rozmiaru grafu
    int max_passes = (int)(5 + log(graph->total_vertices) / log(2));
    int pass = 0;

    do {
        improved = false;
        pass++;

        // Dla każdej pary grup
        for (int i = 0; i < num_parts && !improved; i++) {
            for (int j = i + 1; j < num_parts && !improved; j++) {
                VertexGroup* group1 = &(*groups)[i];
                VertexGroup* group2 = &(*groups)[j];
                
                // Reset tablicy locked
                memset(locked, 0, graph->total_vertices * sizeof(bool));
                
                int total_gain = 0;
                int best_total_gain = 0;
                int best_k = 0;
                
                // Przetwarzaj wierzchołki partiami
                for (int k = 0; k < group1->count && k < group2->count; k += batch_size) {
                    int current_batch_size = batch_size;
                    if (k + batch_size > group1->count || k + batch_size > group2->count) {
                        current_batch_size = (group1->count < group2->count) ? 
                                           group1->count - k : group2->count - k;
                    }
                    
                    // Oblicz zyski dla bieżącej partii
                    int gains_count = 0;
                    for (int v1 = k; v1 < k + current_batch_size && v1 < group1->count; v1++) {
                        if (!locked[group1->vertices[v1]]) {
                            for (int v2 = k; v2 < k + current_batch_size && v2 < group2->count; v2++) {
                                if (!locked[group2->vertices[v2]]) {
                                    int gain = calculate_pair_gain(graph, 
                                        group1->vertices[v1], 
                                        group2->vertices[v2],
                                        group1, group2);
                                        
                                    if (gains_count < batch_size) {
                                        gains1[gains_count].vertex = v1;
                                        gains2[gains_count].vertex = v2;
                                        gains1[gains_count].gain = gain;
                                        gains_count++;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (gains_count == 0) continue;
                    
                    // Sortuj według zysków
                    qsort(gains1, gains_count, sizeof(GainInfo), compare_gains);
                    
                    // Wybierz najlepszą parę
                    int best_gain = gains1[0].gain;
                    int best_v1 = gains1[0].vertex;
                    int best_v2 = gains2[0].vertex;
                    
                    // Wykonaj zamianę tylko jeśli jest korzystna
                    if (best_gain > 0) {
                        swap_vertices(group1, best_v1, group2, best_v2);
                        locked[group1->vertices[best_v1]] = true;
                        locked[group2->vertices[best_v2]] = true;
                        
                        total_gain += best_gain;
                        improved = true;
                        
                        // Aktualizuj najlepszy wynik
                        if (total_gain > best_total_gain) {
                            best_total_gain = total_gain;
                            best_k = k + 1;
                        }
                    }
                }
            }
        }
    } while (improved && pass < max_passes);

    // Zwolnij pamięć pomocniczą
    free(gains1);
    free(gains2);
    free(locked);

    return 0;
}