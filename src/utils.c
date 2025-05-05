#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

// Stała definiująca początkowy rozmiar tablicy
#define INITIAL_CAPACITY 16

// Funkcja bezpiecznej realokacji pamięci
// Parametr ptr - wskaźnik do realokowanej pamięci
// Parametr size - nowy rozmiar w bajtach
// Zwraca wskaźnik do nowej pamięci lub NULL w przypadku błędu
void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        free(ptr);  // Zwolnienie starej pamięci w przypadku błędu
        return NULL;
    }
    return new_ptr;
}

// Funkcja wczytująca liczby oddzielone średnikami z linii tekstu
// Parametr line - wskaźnik do linii tekstu
// Parametr count - wskaźnik do zmiennej przechowującej liczbę wczytanych liczb
// Zwraca tablicę wczytanych liczb lub NULL w przypadku błędu
int* read_semicolon_separated_numbers(char* line, int* count) {
    // Inicjalizacja tablicy wynikowej
    int capacity = INITIAL_CAPACITY;
    int* numbers = (int*)malloc(capacity * sizeof(int));
    if (!numbers) return NULL;
    *count = 0;

    // Usuwanie niepotrzebnych znaków z końca linii
    // Usuwa białe znaki, znaki '%' i średniki
    char* end = line + strlen(line) - 1;
    while (end > line && (isspace(*end) || *end == '%' || *end == ';')) {
        *end = '\0';
        end--;
    }

    // Normalizacja białych znaków w linii
    // Zamienia wszystkie ciągi białych znaków na pojedyncze spacje
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

    // Podział linii na tokeny oddzielone średnikami lub spacjami
    char* token = strtok(line, "; ");
    while (token) {
        // Pomijanie pustych tokenów
        if (strlen(token) > 0) {
            // Sprawdzenie czy potrzebna jest realokacja tablicy
            if (*count >= capacity) {
                capacity *= 2;
                int* new_numbers = (int*)safe_realloc(numbers, capacity * sizeof(int));
                if (!new_numbers) {
                    free(numbers);
                    return NULL;
                }
                numbers = new_numbers;
            }
            
            // Konwersja tokenu na liczbę
            char* endptr;
            long val = strtol(token, &endptr, 10);
            // Sprawdzenie czy konwersja się powiodła
            if (*endptr == '\0') {  // Cały token został przekonwertowany
                numbers[*count] = (int)val;
                (*count)++;
            }
        }
        token = strtok(NULL, "; ");
    }

    return numbers;
}
