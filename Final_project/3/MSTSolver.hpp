#pragma once
#include <tuple>
#include <vector>

class MSTSolver {
public:
    virtual ~MSTSolver() = default;

    // Method to compute MST. It takes the graph's edges and vertex count as inputs and
    // returns a vector of tuples representing the MST edges (start, end, weight, id).
    virtual std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>>& edges, int vertexCount) = 0;
};
