#include "KruskalSolver.hpp"
#include <algorithm>

std::vector<std::tuple<int, int, int, int>> KruskalSolver::computeMST(
    const std::vector<std::tuple<int, int, int, int>>& edges, int vertexCount) {
    
    UnionFind uf(vertexCount); // Union-Find structure to detect cycles
    std::vector<std::tuple<int, int, int, int>> mst;
    auto sortedEdges = edges;

    // Step 1: Sort edges by weight
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const auto& a, const auto& b) { return std::get<2>(a) < std::get<2>(b); });

    // Step 2: Add edges if they don’t form a cycle
    for (const auto& [from, to, weight, id] : sortedEdges) {
        if (uf.unite(from, to)) { // Add edge if it connects two different components
            mst.emplace_back(from, to, weight, id);
        }
    }
    return mst; // Return the computed MST
}
