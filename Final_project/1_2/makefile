# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target executable
TARGET = main

# Source files and objects
SRCS = main.cpp Tree.cpp
OBJS = $(SRCS:.cpp=.o)

# Include paths
INCLUDES = -I.

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Compile .cpp files to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)
