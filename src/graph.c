#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

#define INITIAL_CAPACITY 16
#define MAX_LINE_LENGTH 1024


// Tworzenie nowego grafu
Graph* create_graph(int max_vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;

    graph->max_vertices = max_vertices;
    graph->total_vertices = 0;
    graph->num_rows = 0;
    graph->vertex_indices = NULL;
    graph->row_pointers = NULL;
    graph->adj_list = NULL;

    return graph;
}

// Zwalnianie pamięci grafu
void destroy_graph(Graph* graph) {
    if (!graph) return;

    if (graph->vertex_indices) free(graph->vertex_indices);
    if (graph->row_pointers) free(graph->row_pointers);
    
    if (graph->adj_list) {
        for (int i = 0; i < graph->total_vertices; i++) {
            if (graph->adj_list[i].neighbors) {
                free(graph->adj_list[i].neighbors);
            }
        }
        free(graph->adj_list);
    }

    free(graph);
}

// Wczytywanie grafu z pliku w formacie CSRRG
int load_graph_from_file(const char* filename, Graph** graph) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // Wczytaj liczbę wierzchołków
    int num_vertices = 0;
    if ((read = getline(&line, &len, file)) != -1) {
        num_vertices = atoi(line);
    }
    if (num_vertices <= 0) {
        free(line);
        fclose(file);
        return -1;
    }

    *graph = create_graph(num_vertices);
    if (!*graph) {
        free(line);
        fclose(file);
        return -1;
    }

    // Wczytaj indeksy kolumn
    if ((read = getline(&line, &len, file)) == -1) {
        free(line);
        destroy_graph(*graph);
        fclose(file);
        return -1;
    }

    int col_count;
    int* col_indices = read_semicolon_separated_numbers(line, &col_count);
    if (!col_indices) {
        free(line);
        destroy_graph(*graph);
        fclose(file);
        return -1;
    }

    // Wczytaj wskaźniki wierszy
    if ((read = getline(&line, &len, file)) == -1) {
        free(line);
        free(col_indices);
        destroy_graph(*graph);
        fclose(file);
        return -1;
    }

    int row_count;
    (*graph)->row_pointers = read_semicolon_separated_numbers(line, &row_count);
    if (!(*graph)->row_pointers || row_count != num_vertices + 1) {
        free(line);
        free(col_indices);
        destroy_graph(*graph);
        fclose(file);
        return -1;
    }

    // Wczytaj wartości (opcjonalne)
    if ((read = getline(&line, &len, file)) != -1) {
        // Ignorujemy wartości, ale sprawdzamy czy są poprawne
        int val_count;
        int* values = read_semicolon_separated_numbers(line, &val_count);
        if (values) free(values);
    }

    // Inicjalizuj struktury grafu
    (*graph)->total_vertices = num_vertices;
    (*graph)->vertex_indices = (int*)malloc(num_vertices * sizeof(int));
    (*graph)->adj_list = (AdjacencyList*)malloc(num_vertices * sizeof(AdjacencyList));
    
    if (!(*graph)->vertex_indices || !(*graph)->adj_list) {
        free(line);
        free(col_indices);
        destroy_graph(*graph);
        fclose(file);
        return -1;
    }

    // Inicjalizuj vertex_indices
    for (int i = 0; i < num_vertices; i++) {
        (*graph)->vertex_indices[i] = i;
    }

    // Inicjalizuj listy sąsiedztwa
    for (int i = 0; i < num_vertices; i++) {
        if (init_adjacency_list(&(*graph)->adj_list[i]) != 0) {
            free(line);
            free(col_indices);
            destroy_graph(*graph);
            fclose(file);
            return -1;
        }
    }

    // Konwertuj format CSR na listy sąsiedztwa
    for (int i = 0; i < num_vertices; i++) {
        int start = (*graph)->row_pointers[i];
        int end = (*graph)->row_pointers[i + 1];
        for (int j = start; j < end; j++) {
            if (j < col_count) {  // Dodane zabezpieczenie przed wyjściem poza zakres
                if (add_neighbor(&(*graph)->adj_list[i], col_indices[j]) != 0) {
                    free(line);
                    free(col_indices);
                    destroy_graph(*graph);
                    fclose(file);
                    return -1;
                }
            }
        }
    }

    free(line);
    free(col_indices);
    fclose(file);
    return 0;
}

// Zapisywanie wyniku do pliku
int save_graph_division(const char* filename, const Graph* graph,
                       VertexGroup* groups, int num_groups, bool binary_output) {
    FILE* file = fopen(filename, binary_output ? "wb" : "w");
    if (!file) return -1;

    if (binary_output) {
        // Zapisz w formacie binarnym
        fwrite(&num_groups, sizeof(int), 1, file);
        for (int i = 0; i < num_groups; i++) {
            fwrite(&groups[i].count, sizeof(int), 1, file);
            for (int j = 0; j < groups[i].count; j++) {
                int vertex_index = graph->vertex_indices[groups[i].vertices[j]];
                fwrite(&vertex_index, sizeof(int), 1, file);
            }
        }
    } else {
        // Zapisz w formacie tekstowym
        fprintf(file, "%d\n", num_groups);
        for (int i = 0; i < num_groups; i++) {
            fprintf(file, "Group %d (%d vertices):", i + 1, groups[i].count);
            for (int j = 0; j < groups[i].count; j++) {
                fprintf(file, " %d", graph->vertex_indices[groups[i].vertices[j]]);
            }
            fprintf(file, "\n");
        }
    }

    fclose(file);
    return 0;
}

// Funkcja do odczytu podziału grafu z pliku binarnego
int load_graph_division(const char* filename, VertexGroup** groups, int* num_groups) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Błąd: Nie można otworzyć pliku %s\n", filename);
        return -1;
    }

    // Odczytaj liczbę grup
    if (fread(num_groups, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Błąd: Nie można odczytać liczby grup\n");
        fclose(file);
        return -1;
    }

    // Alokuj pamięć na grupy
    *groups = (VertexGroup*)malloc(*num_groups * sizeof(VertexGroup));
    if (!*groups) {
        fprintf(stderr, "Błąd: Nie można zaalokować pamięci na grupy\n");
        fclose(file);
        return -1;
    }

    // Odczytaj dane dla każdej grupy
    for (int i = 0; i < *num_groups; i++) {
        // Odczytaj liczbę wierzchołków w grupie
        if (fread(&(*groups)[i].count, sizeof(int), 1, file) != 1) {
            fprintf(stderr, "Błąd: Nie można odczytać liczby wierzchołków w grupie %d\n", i);
            for (int j = 0; j < i; j++) free((*groups)[j].vertices);
            free(*groups);
            fclose(file);
            return -1;
        }

        // Alokuj pamięć na wierzchołki
        (*groups)[i].vertices = (int*)malloc((*groups)[i].count * sizeof(int));
        if (!(*groups)[i].vertices) {
            fprintf(stderr, "Błąd: Nie można zaalokować pamięci na wierzchołki grupy %d\n", i);
            for (int j = 0; j < i; j++) free((*groups)[j].vertices);
            free(*groups);
            fclose(file);
            return -1;
        }

        // Odczytaj wierzchołki
        if (fread((*groups)[i].vertices, sizeof(int), (*groups)[i].count, file) != (*groups)[i].count) {
            fprintf(stderr, "Błąd: Nie można odczytać wierzchołków grupy %d\n", i);
            for (int j = 0; j <= i; j++) free((*groups)[j].vertices);
            free(*groups);
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}


