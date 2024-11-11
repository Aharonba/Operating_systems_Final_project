#include "PrimSolver.hpp"
#include "Tree.hpp"
#include <limits>
#include <vector>
#include <set>

constexpr int INF = std::numeric_limits<int>::max();

struct Edge
{
    int weight = INF, from = -1, to = -1, id;

    bool operator<(const Edge &other) const
    {
        return std::make_pair(weight, to) < std::make_pair(other.weight, other.to);
    }

    Edge() = default;
    Edge(int w, int from, int to, int id) : weight(w), from(from), to(to), id(id) {}
};

// Integrate Tree to provide MST metrics along with MST edges
MSTResult PrimSolver::computeMST(const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
{
    // Step 1: Build adjacency list
    std::vector<std::vector<Edge>> adj(vertexCount);
    for (const auto &[from, to, weight, id] : edges)
    {
        adj[from].emplace_back(Edge(weight, from, to, id));
    }

    // Step 2: Initialize Prim's algorithm data structures
    std::vector<Edge> minEdge(vertexCount, {INF, -1, -1, -1});
    minEdge[0].weight = 0;
    std::set<Edge> q;
    q.insert({0, 0, 0, -1});
    std::vector<bool> selected(vertexCount, false);
    std::vector<std::tuple<int, int, int, int>> mst;

    // Step 3: Prim's algorithm loop
    while (!q.empty())
    {
        int v = q.begin()->to;
        Edge currentEdge = *q.begin();
        q.erase(q.begin());

        if (selected[v])
            continue;

        selected[v] = true;

        if (currentEdge.from != currentEdge.to)
        {
            mst.emplace_back(currentEdge.from, v, currentEdge.weight, currentEdge.id);
        }

        for (const auto &e : adj[v])
        {
            if (!selected[e.to] && e.weight < minEdge[e.to].weight)
            {
                q.erase(minEdge[e.to]);
                minEdge[e.to] = e;
                q.insert(e);
            }
        }
    }

    // Calculate metrics using Tree
    Tree mstTree(mst);

    // Return MSTResult, including precomputed shortest distances
    return MSTResult{
        mst,
        mstTree.calculateTotalWeight(),
        mstTree.calculateLongestDistance(),
        mstTree.calculateAverageDistance(),
        mstTree.getShortestPathMatrix()};
}
