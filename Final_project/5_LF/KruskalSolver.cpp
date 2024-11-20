#include "KruskalSolver.hpp"
#include "Tree.hpp"
#include "union_find.hpp"
#include <algorithm>

MSTResult KruskalSolver::computeMST(
    const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
{
    UnionFind uf(vertexCount);
    std::vector<std::tuple<int, int, int, int>> mst;
    auto sortedEdges = edges;

    // Sort edges by weight
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const auto &a, const auto &b)
              { return std::get<2>(a) < std::get<2>(b); });

    // Add edges if they don’t form a cycle
    for (const auto &[from, to, weight, id] : sortedEdges)
    {
        if (uf.unite(from, to))
        {
            mst.emplace_back(from, to, weight, id);
        }
    }

    // Calculate metrics using Tree
    Tree mstTree(mst);

    // Create and return the MSTResult, passing the shortest distance map
    return MSTResult{
        mst,
        mstTree.calculateTotalWeight(),
        mstTree.calculateLongestDistance(),
        mstTree.calculateAverageDistance(),
        mstTree.getShortestPathMatrix() 
    };
}
