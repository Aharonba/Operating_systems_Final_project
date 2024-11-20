#pragma once

#include <memory>
#include "MSTSolver.hpp"
#include "PrimSolver.hpp"
#include "KruskalSolver.hpp"
#include "MSTAlgorithmType.hpp"
#include <iostream>

// MSTFactory is a factory class responsible for creating instances of MSTSolver
// based on the specified MSTAlgorithmType. This allows for flexible algorithm selection
// without hard-coding specific algorithm instances.
class MSTFactory
{
public:
    // Static method that creates and returns a unique pointer to an MSTSolver.
    static std::unique_ptr<MSTSolver> createSolver(MSTAlgorithmType algorithmType)
    {
        switch (algorithmType)
        {
        case MSTAlgorithmType::Prim:
            std::cout << "Creating Prim Solver\n";
            return std::make_unique<PrimSolver>();
        case MSTAlgorithmType::Kruskal:
            std::cout << "Creating Kruskal Solver\n";
            return std::make_unique<KruskalSolver>();
        default:
            std::cout << "Invalid algorithm type\n";
            return nullptr; // Return nullptr if the algorithm type is not supported
        }
    }
};
