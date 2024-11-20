// Tree.hpp
#pragma once
#include <vector>
#include <tuple>
#include <unordered_map>

class Tree {
private:
    std::vector<std::tuple<int, int, int, int>> mstEdges;
    std::unordered_map<int, std::unordered_map<int, int>> shortestPathMatrix;

    void calculateAllPairsShortestPaths();

public:
    Tree(const std::vector<std::tuple<int, int, int, int>> &mst);

    int calculateTotalWeight() const;
    int calculateLongestDistance() const;
    double calculateAverageDistance() const;
    int calculateShortestDistance(int start, int end) const;

    // Getter for the shortest path matrix
    const std::unordered_map<int, std::unordered_map<int, int>>& getShortestPathMatrix() const {
        return shortestPathMatrix;
    }

    // const std::vector<std::tuple<int, int, int, int>>& getMSTEdges() const;
};
