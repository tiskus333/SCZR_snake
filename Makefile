CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17

BIN		:= bin
SRC		:= src
INCLUDE	:= include
OPENCV	:= `pkg-config opencv --cflags --libs`

LIBRARIES	:= $(OPENCV)
EXECUTABLE	:= snake.o


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*