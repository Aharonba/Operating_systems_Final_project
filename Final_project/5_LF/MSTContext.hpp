#pragma once

#include "MSTSolver.hpp"
#include "MSTResult.hpp"
#include <memory>
#include <vector>
#include <tuple>

// MSTContext manages the execution of a selected MST algorithm.
// The strategy pattern is employed here, allowing dynamic switching of the solver strategy.
class MSTContext
{
private:
    std::unique_ptr<MSTSolver> solver; // Pointer to the current MST solver strategy

public:
    // Sets the MST solver strategy dynamically, using a derived MSTSolver class.
    void setSolver(std::unique_ptr<MSTSolver> newSolver)
    {
        solver = std::move(newSolver);
    }

    // Computes the MST along with additional metrics using the current solver strategy.
    // Inputs: list of edges and the vertex count.
    // Output: MSTResult containing the MST edges and metrics.
    MSTResult computeMST(const std::vector<std::tuple<int, int, int, int>> &edges, int vertexCount)
    {
        if (solver)
        {
            return solver->computeMST(edges, vertexCount); // Delegates MST computation to the solver
        }
        return {}; // Returns an empty MSTResult if no solver is set
    }
};
