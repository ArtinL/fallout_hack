CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = fallout_hack
SRC = hack.cpp
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link the target
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp hack.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run with debug flag
debug: $(TARGET)
	./$(TARGET) --debug

# Run with no delay
fast: $(TARGET)
	./$(TARGET) --nodelay

# Run with both debug and no delay flags
test: $(TARGET)
	./$(TARGET) --debug --nodelay

# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run debug fast test clean
