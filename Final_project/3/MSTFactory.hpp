#pragma once
#include "MSTSolver.hpp"
#include "PrimSolver.hpp"
#include "KruskalSolver.hpp"
#include <memory>

// Enum to specify which MST algorithm to use
enum class MSTAlgorithmType
{
    Prim,
    Kruskal
};

class MSTFactory
{
public:
    // Returns an appropriate MST solver based on the given type
    static std::unique_ptr<MSTSolver> createSolver(MSTAlgorithmType type)
    {
        if (type == MSTAlgorithmType::Prim)
        {
            return std::make_unique<PrimSolver>();
        }
        else if (type == MSTAlgorithmType::Kruskal)
        {
            return std::make_unique<KruskalSolver>();
        }
        return nullptr; // Null if algorithm type is invalid
    }
};
