#pragma once
#include <string>

enum class MSTAlgorithmType
{
    Prim,
    Kruskal,
    Invalid
};

inline MSTAlgorithmType stringToAlgorithmType(const std::string &algorithm)
{
    if (algorithm == "Prim")
        return MSTAlgorithmType::Prim;
    if (algorithm == "Kruskal")
        return MSTAlgorithmType::Kruskal;
    return MSTAlgorithmType::Invalid; // Default return
}
