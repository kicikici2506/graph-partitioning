#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

// Program do odczytu i wyświetlania podziału grafu zapisanego w formacie binarnym
int main(int argc, char *argv[]) {
    // Sprawdzenie liczby argumentów wiersza poleceń
    if (argc != 2) {
        printf("Użycie: %s plik_binarny.bin\n", argv[0]);
        return 1;
    }

    // Pobranie nazwy pliku z argumentów
    const char* filename = argv[1];
    VertexGroup* groups = NULL;  // Wskaźnik na tablicę grup wierzchołków
    int num_groups;              // Liczba grup w podziale

    // Wczytanie podziału grafu z pliku binarnego
    if (load_graph_division(filename, &groups, &num_groups) != 0) {
        fprintf(stderr, "Błąd: Nie udało się odczytać pliku binarnego\n");
        return 1;
    }

    // Wyświetlenie informacji o wczytanym podziale
    printf("Odczytano podział z pliku: %s\n", filename);
    printf("Liczba grup: %d\n", num_groups);
    
    // Wyświetlenie szczegółów każdej grupy
    for (int i = 0; i < num_groups; i++) {
        printf("Grupa %d (%d wierzchołków):", i + 1, groups[i].count);
        // Wyświetlenie wszystkich wierzchołków w grupie
        for (int j = 0; j < groups[i].count; j++) {
            printf(" %d", groups[i].vertices[j]);
        }
        printf("\n");
    }

    // Zwolnienie zaalokowanej pamięci
    // Najpierw zwalniamy tablice wierzchołków dla każdej grupy
    for (int i = 0; i < num_groups; i++) {
        free(groups[i].vertices);
    }
    // Następnie zwalniamy tablicę grup
    free(groups);

    return 0;
} 