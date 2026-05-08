CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2

SRC := src/main.cpp src/game.cpp src/board.cpp src/board_view.cpp src/piece.cpp src/square.cpp
OBJ := $(SRC:.cpp=.o)
TARGET := src/echecs

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
