#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>
#include <algorithm>

class Graph
{
private:
    int vertexCount;
    std::vector<std::tuple<int, int, int, int>> edges; // Stores edges as (from, to, weight, id)
    int edgeCounter = 0;                               // Unique ID for each undirected edge

public:
    Graph() : vertexCount(0) {} // Default constructor
    Graph(int n) : vertexCount(n) {}

    // Adds an edge between two vertices in both directions (undirected) with the same ID
    void addEdge(int from, int to, int weight)
    {
        int currentId = edgeCounter++;                   // Assign a unique ID to this undirected edge
        edges.emplace_back(from, to, weight, currentId); // Edge from -> to
        edges.emplace_back(to, from, weight, currentId); // Edge to -> from, with the same ID
    }

    // Removes an edge between two vertices in both directions (undirected)
    void removeEdge(int from, int to)
    {
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                   [from, to](const std::tuple<int, int, int, int> &edge)
                                   {
                                       int u, v;
                                       std::tie(u, v, std::ignore, std::ignore) = edge;
                                       return (u == from && v == to) || (u == to && v == from);
                                   }),
                    edges.end());
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
