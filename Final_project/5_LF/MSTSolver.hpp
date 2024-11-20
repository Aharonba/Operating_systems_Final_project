#pragma once
#include <vector>
#include <tuple>
#include "MSTResult.hpp"

class MSTSolver
{
public:
    virtual ~MSTSolver() = default;

    // Change return type to MSTResult
    virtual MSTResult computeMST(const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount) = 0;
};
