#pragma once
#include "MSTSolver.hpp"
#include <memory>

class MSTContext {
private:
    std::unique_ptr<MSTSolver> solver; // Pointer to the current strategy

public:
    // Allows setting a new MST solver at runtime
    void setSolver(std::unique_ptr<MSTSolver> newSolver) {
        solver = std::move(newSolver);
    }

    // Uses the current MST solver to compute the MST
    std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>>& edges, int vertexCount) {
        if (solver) {
            return solver->computeMST(edges, vertexCount);
        }
        return {}; // Return an empty MST if no solver is set
    }
};
