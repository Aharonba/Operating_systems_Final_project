#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>

class Tree {
private:
    std::vector<std::tuple<int, int, int, int>> mstEdges; // MST edges represented as (from, to, weight, id)
    std::unordered_map<int, std::unordered_map<int, int>> shortestPathMatrix; // Stores shortest paths between all pairs of vertices

    // Private helper function to calculate all-pairs shortest paths using Dijkstra's algorithm
    void calculateAllPairsShortestPaths();

public:
    Tree(const std::vector<std::tuple<int, int, int, int>>& mst); 

    int calculateTotalWeight() const; 
    int calculateLongestDistance() const; 
    double calculateAverageDistance() const; 
    int calculateShortestDistance(int start, int end) const; 

    const std::vector<std::tuple<int, int, int, int>>& getMSTEdges() const; 
};
