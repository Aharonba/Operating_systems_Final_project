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
    // The type of solver created depends on the value of `algorithmType`.
    static std::unique_ptr<MSTSolver> createSolver(MSTAlgorithmType algorithmType)
    {
        switch (algorithmType)
        {
        case MSTAlgorithmType::Prim:
            std::cout << "Creating Prim Solver\n"; // Debug output for Prim Solver creation
            return std::make_unique<PrimSolver>();
        case MSTAlgorithmType::Kruskal:
            std::cout << "Creating Kruskal Solver\n"; // Debug output for Kruskal Solver creation
            return std::make_unique<KruskalSolver>();
        default:
            std::cout << "Invalid algorithm type\n"; // Debug output for unsupported type
            return nullptr;                          // Return nullptr if the algorithm type is not supported
        }
    }
};
