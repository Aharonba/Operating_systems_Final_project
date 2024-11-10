#include <iostream>
#include <limits>
#include "Graph.hpp"
#include "Tree.hpp"

// Helper function to print the shortest path matrix
void printShortestPathMatrix(const std::unordered_map<int, std::unordered_map<int, int>> &matrix)
{
    for (const auto &[start, distances] : matrix)
    {
        std::cout << "Shortest paths from vertex " << start << ":\n";
        for (const auto &[end, distance] : distances)
        {
            std::cout << "  To vertex " << end << ": " << distance << "\n";
        }
    }
}

int main()
{
    // Step 1: Define and create the Graph with vertices and edges
    Graph graph(7); // Example with 7 vertices
    graph.addEdge(0, 5, 10);
    graph.addEdge(5, 4, 25);
    graph.addEdge(4, 3, 22);
    graph.addEdge(3, 2, 12);
    graph.addEdge(2, 1, 16);
    graph.addEdge(1, 6, 14);

    // Step 2: Generate the MST from the graph's edges
    // Here we assume all edges in `graph` are part of the MST for testing purposes
    Tree tree(graph.getEdges());

    // Step 3: Calculate and display MST properties
    std::cout << "MST Properties:\n";
    std::cout << "  - Total weight of MST: " << tree.calculateTotalWeight() << "\n";
    std::cout << "  - Longest shortest path in MST: " << tree.calculateLongestDistance() << "\n";
    std::cout << "  - Average shortest path distance in MST: " << tree.calculateAverageDistance() << "\n";

    // Step 5: Test the shortest distance between specific vertices
    int startVertex = 4;
    int endVertex = 6;
    int shortestDistance = tree.calculateShortestDistance(startVertex, endVertex);
    if (shortestDistance != std::numeric_limits<int>::max())
    {
        std::cout << "\nShortest distance between vertices " << startVertex << " and " << endVertex
                  << ": " << shortestDistance << "\n";
    }
    else
    {
        std::cout << "\nNo path exists between vertices " << startVertex << " and " << endVertex << " in the MST.\n";
    }

    return 0;
}
