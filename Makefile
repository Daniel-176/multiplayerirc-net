CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -IClient
LDFLAGS  := -lixwebsocket

SRC      := main.cpp
BUILD_DIR := build
DIST_DIR  := dist

OBJ      := $(BUILD_DIR)/main.o
TARGET   := $(DIST_DIR)/mpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ) | $(DIST_DIR)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)
