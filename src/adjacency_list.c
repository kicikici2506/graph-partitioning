#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

#define INITIAL_CAPACITY 16 

// Inicjalizacja listy sąsiedztwa
 int init_adjacency_list(AdjacencyList* list) {
    list->neighbors = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!list->neighbors) return -1;
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return 0;
}

// Dodawanie sąsiada do listy
 int add_neighbor(AdjacencyList* list, int neighbor) {
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