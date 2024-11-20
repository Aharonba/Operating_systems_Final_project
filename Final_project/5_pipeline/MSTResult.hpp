#pragma once
#include <vector>
#include <unordered_map>
#include <tuple>

struct MSTResult
{
    std::vector<std::tuple<int, int, int, int>> mstEdges;
    int totalWeight;
    int longestDistance;
    double averageDistance;

    // Map of shortest distances between each pair of vertices
    std::unordered_map<int, std::unordered_map<int, int>> shortestDistances;
};
