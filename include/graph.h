#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 16

// Struktura reprezentująca sąsiadów węzła
typedef struct {
    int* neighbors;     // Lista sąsiadów
    int count;         // Liczba sąsiadów
    int capacity;      // Pojemność tablicy sąsiadów
} AdjacencyList;

// Struktura reprezentująca grupę węzłów
typedef struct {
    int* vertices;      // Węzły w grupie
    int count;         // Liczba węzłów
    int first_vertex;  // Wskaźnik na pierwszy węzeł
    int capacity;      // Pojemność tablicy węzłów
} VertexGroup;

// Struktura reprezentująca graf
typedef struct {
    int max_vertices;        // Maksymalna liczba węzłów w wierszu
    int total_vertices;      // Całkowita liczba węzłów
    int* vertex_indices;     // Tablica wszystkich indeksów węzłów
    int* row_pointers;      // Wskaźniki na pierwsze indeksy węzłów w wierszach
    int num_rows;           // Liczba wierszy
    AdjacencyList* adj_list; // Lista sąsiedztwa dla każdego węzła
} Graph;

// Struktura pomocnicza do przechowywania informacji o zysku
typedef struct {
    int vertex;
    int gain;
} GainInfo;

// Funkcje do operacji na grafie
Graph* create_graph(int max_vertices);
void destroy_graph(Graph* graph);
int load_graph_from_file(const char* filename, Graph** graph);
int save_graph_division(const char* filename, const Graph* graph, 
                       VertexGroup* groups, int num_groups, bool binary_output);

// Funkcje do podziału grafu
int divide_graph(Graph* graph, int num_parts, double margin_percentage, VertexGroup** groups);
int calculate_edges_between_groups(const Graph* graph, const VertexGroup* groups, int num_groups);
double calculate_size_difference(const VertexGroup* groups, int num_groups);

// Funkcje pomocnicze
void print_graph_info(const Graph* graph);
void print_division_info(const VertexGroup* groups, int num_groups);

// Funkcje do operacji na listach sąsiedztwa
int init_adjacency_list(AdjacencyList* list);
int add_neighbor(AdjacencyList* list, int neighbor);

// Funkcje pomocnicze do alokacji pamięci
void* safe_realloc(void* ptr, size_t size);

int* read_semicolon_separated_numbers(char* line, int* count);

// Funkcja do odczytu podziału grafu z pliku binarnego
int load_graph_division(const char* filename, VertexGroup** groups, int* num_groups);

#endif // GRAPH_H