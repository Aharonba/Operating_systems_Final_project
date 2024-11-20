#pragma once
#include "MSTSolver.hpp"
#include "union_find.hpp"

class KruskalSolver : public MSTSolver {
public:
    std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>>& edges, int vertexCount) override;
};


