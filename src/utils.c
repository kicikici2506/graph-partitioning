#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "../include/graph.h"

#define INITIAL_CAPACITY 16

// Funkcja pomocnicza do realokacji pamięci
 void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        free(ptr);
        return NULL;
    }
    return new_ptr;
}


// Funkcja pomocnicza do wczytywania liczb oddzielonych średnikami
 int* read_semicolon_separated_numbers(char* line, int* count) {
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
