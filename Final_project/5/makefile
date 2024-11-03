# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source files for Server and Client
SERVER_SOURCES = Server.cpp PrimSolver.cpp KruskalSolver.cpp Tree.cpp union_find.cpp
CLIENT_SOURCES = Client.cpp

# Header files
HEADERS = Server.hpp Client.hpp MSTResult.hpp MSTFactory.hpp MSTAlgorithmType.hpp Tree.hpp \
          PrimSolver.hpp KruskalSolver.hpp union_find.hpp Graph.hpp

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

# Clean target
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_TARGET) $(CLIENT_TARGET)
