#pragma once
#include "MSTSolver.hpp"
#include <tuple>
#include <vector>
#include <set>

class PrimSolver : public MSTSolver {
public:
    std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>>& edges, int vertexCount) override;
};
