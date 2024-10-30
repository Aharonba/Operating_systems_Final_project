// MSTFactory.hpp
#pragma once
#include <memory>
#include "MSTSolver.hpp"
#include "PrimSolver.hpp"
#include "KruskalSolver.hpp"
#include "MSTAlgorithmType.hpp"

class MSTFactory {
public:
    static std::unique_ptr<MSTSolver> createSolver(MSTAlgorithmType algorithmType) {
        switch (algorithmType) {
            case MSTAlgorithmType::Prim:
                return std::make_unique<PrimSolver>();
            case MSTAlgorithmType::Kruskal:
                return std::make_unique<KruskalSolver>();
            default:
                return nullptr;
        }
    }
};
