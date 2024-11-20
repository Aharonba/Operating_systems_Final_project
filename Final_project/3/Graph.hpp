#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>

class Graph
{
private:
    int vertexCount;
    std::vector<std::tuple<int, int, int, int>> edges; // Stores edges as (from, to, weight, id)
    int edgeCounter = 0;                               // Unique ID for each undirected edge

public:
    Graph(int n) : vertexCount(n) {}

    // Adds an edge between two vertices in both directions (undirected) with the same ID
    void addEdge(int from, int to, int weight)
    {
        int currentId = edgeCounter++;                   // Assign a unique ID to this undirected edge
        edges.emplace_back(from, to, weight, currentId); // Edge from -> to
        edges.emplace_back(to, from, weight, currentId); // Edge to -> from, with the same ID
    }

    int getVertexCount() const
    {
        return vertexCount;
    }

    const std::vector<std::tuple<int, int, int, int>> &getEdges() const
    {
        return edges;
    }
};
