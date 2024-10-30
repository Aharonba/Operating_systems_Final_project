#include "MSTFactory.hpp"
#include "MSTContext.hpp"
#include "Graph.hpp"
#include <iostream>

void printMST(const std::vector<std::tuple<int, int, int, int>> &mst, const std::string &algorithm)
{
    std::cout << "MST using " << algorithm << ":\n";
    for (const auto &[from, to, weight, id] : mst)
    {
        std::cout << "Edge " << id << ": " << from << " - " << to << " (Weight: " << weight << ")\n";
    }
    std::cout << "Total edges in MST: " << mst.size() << "\n\n";
}

int main()
{
    //Step 1: Create a more complex Graph with multiple vertices and edges
    Graph graph(7); // Example with 7 vertices
    graph.addEdge(0, 5, 10);
    graph.addEdge(5, 4, 25);
    graph.addEdge(6, 4, 24);
    graph.addEdge(1, 6, 14);
    graph.addEdge(0, 1, 28);
    graph.addEdge(4, 3, 22);
    graph.addEdge(3, 2, 12);
    graph.addEdge(2, 1, 16);
    graph.addEdge(3, 6, 18);

    MSTContext context;

    // Step 2: Compute and print MST using Prim's algorithm
    context.setSolver(MSTFactory::createSolver(MSTAlgorithmType::Prim));
    auto mstPrim = context.computeMST(graph.getEdges(), graph.getVertexCount());
    printMST(mstPrim, "Prim's Algorithm");

    // Step 3: Compute and print MST using Kruskal's algorithm
    context.setSolver(MSTFactory::createSolver(MSTAlgorithmType::Kruskal));
    auto mstKruskal = context.computeMST(graph.getEdges(), graph.getVertexCount());
    printMST(mstKruskal, "Kruskal's Algorithm");

    return 0;
}
