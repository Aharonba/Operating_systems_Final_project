#pragma once
#include "MSTSolver.hpp"
#include "MSTResult.hpp"

class KruskalSolver : public MSTSolver
{
public:
    // Update return type to MSTResult
    MSTResult computeMST(const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount) override;
};
