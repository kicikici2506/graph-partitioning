#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

#define INITIAL_CAPACITY 16
#define MAX_LINE_LENGTH 1024

// Funkcja pomocnicza do realokacji pamięci
static void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        free(ptr);
        return NULL;
    }
    return new_ptr;
}

// Inicjalizacja listy sąsiedztwa
static int init_adjacency_list(AdjacencyList* list) {
    list->neighbors = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!list->neighbors) return -1;
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return 0;
}

// Dodawanie sąsiada do listy
static int add_neighbor(AdjacencyList* list, int neighbor) {
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity * 2;
        int* new_neighbors = (int*)safe_realloc(list->neighbors, new_capacity * sizeof(int));
        if (!new_neighbors) return -1;
        list->neighbors = new_neighbors;
        list->capacity = new_capacity;
    }
    list->neighbors[list->count++] = neighbor;
    return 0;
}

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

// Funkcja pomocnicza do wczytywania liczb oddzielonych średnikami
static int* read_semicolon_separated_numbers(char* line, int* count) {
    int capacity = INITIAL_CAPACITY;
    int* numbers = (int*)malloc(capacity * sizeof(int));
    if (!numbers) return NULL;
    *count = 0;

    // Usuń znaki białe i '%' z końca linii
    char* end = line + strlen(line) - 1;
    while (end > line && (isspace(*end) || *end == '%' || *end == ';')) {
        *end = '\0';
        end--;
    }

    // Zamień wszystkie znaki białe na pojedyncze spacje
    char* src = line;
    char* dst = line;
    int space = 0;
    while (*src) {
        if (isspace(*src)) {
            if (!space) {
                *dst++ = ' ';
                space = 1;
            }
        } else {
            *dst++ = *src;
            space = 0;
        }
        src++;
    }
    *dst = '\0';

    // Podziel string na tokeny
    char* token = strtok(line, "; ");
    while (token) {
        // Pomiń puste tokeny
        if (strlen(token) > 0) {
            if (*count >= capacity) {
                capacity *= 2;
                int* new_numbers = (int*)safe_realloc(numbers, capacity * sizeof(int));
                if (!new_numbers) {
                    free(numbers);
                    return NULL;
                }
                numbers = new_numbers;
            }
            
            // Konwertuj string na liczbę
            char* endptr;
            long val = strtol(token, &endptr, 10);
            if (*endptr == '\0') {  // Upewnij się, że cały token został przekonwertowany
                numbers[*count] = (int)val;
                (*count)++;
            }
        }
        token = strtok(NULL, "; ");
    }

    return numbers;
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

// Struktura pomocnicza do przechowywania informacji o zysku
typedef struct {
    int vertex;
    int gain;
} GainInfo;

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