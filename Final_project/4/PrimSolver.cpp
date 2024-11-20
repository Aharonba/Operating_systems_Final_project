#include "PrimSolver.hpp"
#include <limits>
#include <vector>
#include <set>

constexpr int INF = std::numeric_limits<int>::max();

struct Edge
{
    int weight = INF, from = -1, to = -1, id;

    // Comparison for ordering edges by weight (priority queue)
    bool operator<(const Edge &other) const
    {
        return std::make_pair(weight, to) < std::make_pair(other.weight, other.to);
    }

    Edge() = default;
    Edge(int w, int from, int to, int id) : weight(w), from(from), to(to), id(id) {}
};

std::vector<std::tuple<int, int, int, int>> PrimSolver::computeMST(
    const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
{

    // Step 1: Build adjacency list
    std::vector<std::vector<Edge>> adj(vertexCount);
    for (const auto &[from, to, weight, id] : edges)
    {
        adj[from].emplace_back(Edge(weight, from, to, id));
    }

    // Step 2: Initialize Prim's algorithm data structures
    std::vector<Edge> minEdge(vertexCount, {INF, -1, -1, -1});
    minEdge[0].weight = 0; // Start with the first vertex
    std::set<Edge> q;
    q.insert({0, 0, 0, -1}); // Starting edge (weight 0, connecting vertex 0)
    std::vector<bool> selected(vertexCount, false);
    std::vector<std::tuple<int, int, int, int>> mst;

    // Step 3: Main Prim's algorithm loop
    while (!q.empty())
    {
        int v = q.begin()->to; // Vertex with minimum-weight edge
        Edge currentEdge = *q.begin();
        q.erase(q.begin());

        if (selected[v])
            continue;

        selected[v] = true;

        // Add edge to MST if it's valid (not the starting dummy edge)
        if (currentEdge.from != currentEdge.to)
        {
            mst.emplace_back(currentEdge.from, v, currentEdge.weight, currentEdge.id);
        }

        // Update edges for vertices adjacent to `v`
        for (const auto &e : adj[v])
        {
            if (!selected[e.to] && e.weight < minEdge[e.to].weight)
            {
                q.erase(minEdge[e.to]); // Remove previous min edge for `to`
                minEdge[e.to] = e;      // Update `minEdge` for `to` with new edge `e`
                q.insert(e);            // Insert new min edge into `q`
            }
        }
    }

    return mst;
}
