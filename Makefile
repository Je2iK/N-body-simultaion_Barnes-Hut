# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -Wall -Wextra -pthread
INCLUDES = -Iinclude
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lpthread

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Utils.cpp \
          $(SRC_DIR)/Cell.cpp \
          $(SRC_DIR)/BarnesHutSimulator.cpp \
          $(SRC_DIR)/BruteForceSimulator.cpp \
          $(SRC_DIR)/Menu.cpp \
          $(SRC_DIR)/Benchmark.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/nbody_simulation

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "✅ Build complete: $(TARGET)"

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "🧹 Cleaned build artifacts"

# Run
run: all
	./$(TARGET)

# Phony targets
.PHONY: all clean run directories
