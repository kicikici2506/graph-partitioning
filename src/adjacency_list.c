#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

// Stała definiująca początkowy rozmiar tablicy sąsiadów
#define INITIAL_CAPACITY 16 

// Funkcja inicjalizująca listę sąsiedztwa dla wierzchołka
// Parametr list - wskaźnik do struktury listy sąsiedztwa
// Zwraca 0 w przypadku sukcesu, -1 w przypadku błędu alokacji
int init_adjacency_list(AdjacencyList* list) {
    // Alokacja pamięci na początkową tablicę sąsiadów
    list->neighbors = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!list->neighbors) return -1;
    
    // Inicjalizacja pól struktury
    list->count = 0;        // Początkowa liczba sąsiadów
    list->capacity = INITIAL_CAPACITY;  // Początkowy rozmiar tablicy
    
    return 0;
}

// Funkcja dodająca nowego sąsiada do listy sąsiedztwa
// Parametr list - wskaźnik do struktury listy sąsiedztwa
// Parametr neighbor - indeks wierzchołka-sąsiada do dodania
// Zwraca 0 w przypadku sukcesu, -1 w przypadku błędu alokacji
int add_neighbor(AdjacencyList* list, int neighbor) {
    // Sprawdzenie czy tablica jest pełna
    if (list->count >= list->capacity) {
        // Podwojenie rozmiaru tablicy
        int new_capacity = list->capacity * 2;
        
        // Realokacja pamięci z zachowaniem istniejących danych
        int* new_neighbors = (int*)safe_realloc(list->neighbors, new_capacity * sizeof(int));
        if (!new_neighbors) return -1;
        
        // Aktualizacja wskaźnika i pojemności
        list->neighbors = new_neighbors;
        list->capacity = new_capacity;
    }
    
    // Dodanie nowego sąsiada na koniec listy
    list->neighbors[list->count++] = neighbor;
    return 0;
}