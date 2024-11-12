#pragma once
#include "MSTSolver.hpp"
#include <memory>

class MSTContext
{
private:
    std::unique_ptr<MSTSolver> solver; // Holds the current MST solving strategy

public:
    // Sets the MST solver strategy at runtime
    void setSolver(std::unique_ptr<MSTSolver> newSolver)
    {
        solver = std::move(newSolver);
    }

    // Computes the MST using the current solver, if set
    std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
    {
        if (solver)
        {
            return solver->computeMST(edges, vertexCount);
        }
        return {}; // Return empty result if no solver is available
    }
};
