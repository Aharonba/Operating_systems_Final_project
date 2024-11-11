#pragma once

#include "MSTSolver.hpp"
#include <memory>
#include <vector>
#include <tuple>

// MSTContext provides a context for executing a selected MST algorithm.
// This class uses a strategy pattern, where the strategy (solver) can be set
// dynamically, allowing different MST algorithms to be used interchangeably.
class MSTContext
{
private:
    std::unique_ptr<MSTSolver> solver; // Pointer to the current MST solver strategy

public:
    // Sets a new MST solver strategy. The solver can be any class derived from MSTSolver.
    // The use of unique_ptr ensures only one active solver at a time and transfers ownership.
    void setSolver(std::unique_ptr<MSTSolver> newSolver)
    {
        solver = std::move(newSolver);
    }

    // Computes the Minimum Spanning Tree (MST) using the current solver strategy.
    // Takes a list of edges and the vertex count as input, returns the computed MST.
    std::vector<std::tuple<int, int, int, int>> computeMST(
        const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
    {
        if (solver)
        {
            return solver->computeMST(edges, vertexCount); // Delegates MST computation to the solver
        }
        return {}; // Returns an empty MST if no solver is set
    }
};
