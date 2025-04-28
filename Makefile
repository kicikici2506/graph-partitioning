CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = -lm

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Lista plików źródłowych
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Nazwa programu wynikowego
TARGET = $(BIN_DIR)/graph_divider

# Domyślny cel
all: directories $(TARGET)

# Tworzenie katalogów
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Linkowanie
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Kompilacja
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Czyszczenie
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Uruchamianie
run: all
	./$(TARGET)

-include $(DEPS)

.PHONY: all clean run directories