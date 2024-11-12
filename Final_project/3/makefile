CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target Executable
TARGET = mst_program

SRCS = main.cpp \
       PrimSolver.cpp \
       KruskalSolver.cpp \
       Tree.cpp \
       union_find.cpp

# Object Files
OBJS = $(SRCS:.cpp=.o)

# Default Target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f *.o $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)
