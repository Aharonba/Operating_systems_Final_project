#pragma once
#include "MSTSolver.hpp"
#include "MSTResult.hpp"

class PrimSolver : public MSTSolver
{
public:
    MSTResult computeMST(const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount) override;
};
