#pragma once
#include <string>

// Enum class representing types of Minimum Spanning Tree (MST) algorithms
enum class MSTAlgorithmType
{
    Prim,      // Prim's algorithm
    Kruskal,   // Kruskal's algorithm
    Invalid    // Invalid type, used for unsupported algorithms
};

// Helper function that converts a string to the corresponding MSTAlgorithmType
// This is useful for interpreting user input or command arguments
inline MSTAlgorithmType stringToAlgorithmType(const std::string &algorithm)
{
    if (algorithm == "Prim")
        return MSTAlgorithmType::Prim;       
    if (algorithm == "Kruskal")
        return MSTAlgorithmType::Kruskal;    
    return MSTAlgorithmType::Invalid;        // Returns Invalid if input doesn't match known algorithms
}
