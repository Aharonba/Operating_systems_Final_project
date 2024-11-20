CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 --coverage

# Source files for Server and Client
SERVER_SOURCES = Server.cpp PrimSolver.cpp KruskalSolver.cpp Tree.cpp union_find.cpp PAO.cpp
CLIENT_SOURCES = Client.cpp

# Header files
HEADERS = Server.hpp Client.hpp MSTResult.hpp MSTFactory.hpp MSTAlgorithmType.hpp Tree.hpp \
          PrimSolver.hpp KruskalSolver.hpp union_find.hpp Graph.hpp PAO.hpp

# Object files for Server and Client
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

# Targets for Server and Client
SERVER_TARGET = server_program
CLIENT_TARGET = client_program

# Default target to build both programs
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Build Server Program
$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJECTS)

# Build Client Program
$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJECTS)

# Compile each .cpp file into .o files with dependency on headers
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Valgrind target to check for memory leaks in the server program
valgrind: $(SERVER_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(SERVER_TARGET)

# Helgrind target to check for thread issues in the server program
helgrind: $(SERVER_TARGET)
	valgrind --tool=helgrind ./$(SERVER_TARGET)
	
# Cachegrind target to profile cache usage in the server program
cachegrind: $(SERVER_TARGET)
	valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.out ./$(SERVER_TARGET)

coverage: $(SERVER_TARGET) $(CLIENT_TARGET)
	$(CXX) $(CXXFLAGS) --coverage -o $(SERVER_TARGET) $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) --coverage -o $(CLIENT_TARGET) $(CLIENT_OBJECTS)
	./$(SERVER_TARGET)  # Run the server program for coverage
	./$(CLIENT_TARGET)  # Run the client program for coverage
	gcov $(SERVER_SOURCES)  # Generate coverage report for server
	gcov $(CLIENT_SOURCES)  # Generate coverage report for client
	lcov --capture --directory . --output-file coverage.info  # Capture coverage data
	genhtml coverage.info --output-directory out  # Generate HTML coverage report
	
# Clean target
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_TARGET) $(CLIENT_TARGET) *.gcda *.gcno
