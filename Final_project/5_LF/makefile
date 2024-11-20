CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 --coverage  # Add --coverage for gcov

# Source files for Server and Client
SERVER_SOURCES = Server.cpp PrimSolver.cpp KruskalSolver.cpp Tree.cpp union_find.cpp LFP.cpp
CLIENT_SOURCES = Client.cpp

# Header files
HEADERS = Server.hpp Client.hpp MSTResult.hpp MSTFactory.hpp MSTAlgorithmType.hpp Tree.hpp \
          PrimSolver.hpp KruskalSolver.hpp union_find.hpp Graph.hpp PAO.hpp LFP.hpp

# Object files for Server and Client
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

# Targets for Server and Client executables
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

# Memory check with Valgrind memcheck
start_server_memcheck: $(SERVER_TARGET)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(SERVER_TARGET)

# Data race check with Valgrind Helgrind
start_server_helgrind: $(SERVER_TARGET)
	valgrind --tool=helgrind ./$(SERVER_TARGET)

# Profiling cache usage with Valgrind Cachegrind
start_server_cachegrind: $(SERVER_TARGET)
	valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.out ./$(SERVER_TARGET)

# Code Coverage: Run tests and generate coverage report
coverage: $(SERVER_TARGET) $(CLIENT_TARGET)
	$(CXX) $(CXXFLAGS) --coverage -o $(SERVER_TARGET) $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) --coverage -o $(CLIENT_TARGET) $(CLIENT_OBJECTS)
	./$(SERVER_TARGET)  # Run the server program for coverage
	./$(CLIENT_TARGET)  # Run the client program for coverage
	lcov --capture --directory . --output-file coverage_files/coverage.info --no-external  # Capture coverage data
	genhtml coverage_files/coverage.info --output-directory coverage_files/html_report  # Generate HTML report

# Move all .gcda and .gcno files to a separate directory
move-coverage-files:
	mkdir -p coverage_files  # Create the directory if it doesn't exist
	mv *.gcda *.gcno coverage_files/  # Move coverage files

# Clean target to remove all generated files
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_TARGET) $(CLIENT_TARGET) *.gcda *.gcno 
