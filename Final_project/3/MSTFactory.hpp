#pragma once
#include "MSTSolver.hpp"
#include "PrimSolver.hpp"
#include "KruskalSolver.hpp"
#include <memory>

enum class MSTAlgorithmType { Prim, Kruskal };

class MSTFactory {
public:
    // Creates the appropriate solver based on the algorithm type
    static std::unique_ptr<MSTSolver> createSolver(MSTAlgorithmType type) {
        if (type == MSTAlgorithmType::Prim) {
            return std::make_unique<PrimSolver>();
        } else if (type == MSTAlgorithmType::Kruskal) {
            return std::make_unique<KruskalSolver>();
        }
        return nullptr; // Return nullptr if no valid type is provided
    }
};
