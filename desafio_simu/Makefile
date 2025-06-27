# Makefile simplificado para el proyecto EscapeTheGrid
# Enfocado en PLATFORM_DESKTOP (Linux/MinGW asumido por el sandbox)

PROJECT_NAME := game
SRC_DIR      := src
OBJ_DIR      := obj
BIN_DIR      := .

# Compilador C++
CXX          := g++
# Flags para el compilador C++
CXXFLAGS     := -std=c++14 -Wall -Wextra -g -O2 -DPLATFORM_DESKTOP
# Flags para incluir directorios
# Intentar localizar raylib.h
# -I$(SRC_DIR): Incluye el directorio de fuentes (src/).
# -I.: Incluye el directorio actual (donde está el Makefile, es decir, desafio_simu/).
# -Iinclude: Busca en desafio_simu/include/
# -Ilib: Busca en desafio_simu/lib/ (menos probable para .h, pero por si acaso)
# -I../include: Busca en el directorio padre de desafio_simu, en una carpeta include/
# -I../../include: Busca dos niveles arriba, en una carpeta include/ (común en ejemplos de raylib)
INCLUDE_PATHS := -I$(SRC_DIR) -I. -Iinclude -Ilib -I../include -I../../include

# Ruta a la librería Raylib y otras librerías
# LDFLAGS: Flags para el enlazador.
# -Llib: Busca librerías en desafio_simu/lib/
# -L/usr/local/lib: Directorio estándar donde buscar librerías.
# LDLIBS: Librerías a enlazar.
# -lraylib: Enlaza la librería Raylib.
# -lGL -lm -lpthread -ldl -lrt -lX11: Librerías requeridas por Raylib en Linux.
LDFLAGS      := -Llib -L/usr/local/lib
LDLIBS       := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Encuentra todos los archivos .cpp en el directorio SRC_DIR
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
# Genera los nombres de los archivos objeto .o reemplazando $(SRC_DIR)/%.cpp con $(OBJ_DIR)/%.o
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# El ejecutable final
TARGET := $(BIN_DIR)/$(PROJECT_NAME)

.PHONY: all clean run help

# Regla por defecto: compila todo
all: $(TARGET)

# Regla para enlazar el ejecutable final a partir de los archivos objeto
$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Enlazado exitoso: $@"

# Regla para compilar archivos .cpp a archivos .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@
	@echo "Compilado: $< -> $@"

# Regla para limpiar archivos compilados
clean:
	@echo "Limpiando archivos compilados..."
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	@echo "Limpieza completada."

# Regla para ejecutar el juego
run: all
	@echo "Ejecutando $(TARGET)..."
	./$(TARGET)

# Mensaje de ayuda
help:
	@echo "Uso: make [all|clean|run|help]"
	@echo "  all    - Compila el proyecto (default)."
	@echo "  clean  - Elimina los archivos compilados."
	@echo "  run    - Compila y ejecuta el proyecto."
	@echo "  help   - Muestra este mensaje de ayuda."

.NOTPARALLEL:
.SUFFIXES:
