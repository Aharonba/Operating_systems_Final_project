#include "Tree.hpp"
#include <limits>
#include <queue>
#include <unordered_set>
#include <vector>
#include <algorithm>

// Constructor initializes the MST edges and calculates all-pairs shortest paths
Tree::Tree(const std::vector<std::tuple<int, int, int, int>> &mst) : mstEdges(mst)
{
    calculateAllPairsShortestPaths();
}

// Private function to compute shortest paths between all vertex pairs using Dijkstra's algorithm
void Tree::calculateAllPairsShortestPaths()
{
    // Build adjacency list from MST edges
    std::unordered_map<int, std::vector<std::pair<int, int>>> adjList;
    for (const auto &[v1, v2, weight, id] : mstEdges)
    {
        adjList[v1].emplace_back(v2, weight); // Connect v1 to v2 with weight
        adjList[v2].emplace_back(v1, weight); // Connect v2 to v1 with weight (undirected graph)
    }

    // Run Dijkstra's algorithm from each vertex to find shortest paths to all other vertices
    for (const auto &[start, _] : adjList)
    {
        // Initialize distances for Dijkstra’s algorithm
        std::unordered_map<int, int> distances;
        for (const auto &[v, _] : adjList)
            distances[v] = std::numeric_limits<int>::max();
        distances[start] = 0; // Distance to itself is 0

        // Priority queue to track the shortest path during traversal (min-heap)
        using QueueElem = std::pair<int, int>; // Pair of (distance, vertex)
        std::priority_queue<QueueElem, std::vector<QueueElem>, std::greater<QueueElem>> pq;
        pq.emplace(0, start);

        // Main loop of Dijkstra's algorithm
        while (!pq.empty())
        {
            auto [currentDist, u] = pq.top();
            pq.pop();

            // Skip processing if we already found a shorter path
            if (currentDist > distances[u])
                continue;

            // Update distances for each neighbor of u
            for (const auto &[v, weight] : adjList[u])
            {
                int newDist = currentDist + weight;
                if (newDist < distances[v])
                { // Found a shorter path to v
                    distances[v] = newDist;
                    pq.emplace(newDist, v);
                }
            }
        }

        // Store distances from 'start' to all other vertices in the shortestPathMatrix
        shortestPathMatrix[start] = distances;
    }
}

// Function to calculate the total weight of the MST by summing the weights of unique edges
int Tree::calculateTotalWeight() const
{
    int totalWeight = 0;
    std::unordered_set<int> countedEdges; // Track unique edge IDs to avoid double-counting

    for (const auto &edge : mstEdges)
    {
        int edgeId = std::get<3>(edge); // Extract edge ID
        if (countedEdges.find(edgeId) == countedEdges.end())
        {                                     // Only add if edge hasn't been counted yet
            totalWeight += std::get<2>(edge); // Add weight
            countedEdges.insert(edgeId);      // Mark as counted
        }
    }
    return totalWeight;
}

// Function to find the longest shortest path in the MST (max distance between any two vertices)
int Tree::calculateLongestDistance() const
{
    int longestDistance = 0;
    for (const auto &[_, distances] : shortestPathMatrix)
    {
        for (const auto &[_, dist] : distances)
        {
            if (dist != std::numeric_limits<int>::max())
            { // Ignore unreachable distances
                longestDistance = std::max(longestDistance, dist);
            }
        }
    }
    return longestDistance;
}

// Function to calculate the average shortest path distance between all vertex pairs
double Tree::calculateAverageDistance() const
{
    int totalDistance = 0;
    int pairCount = 0;
    for (const auto &[_, distances] : shortestPathMatrix)
    {
        for (const auto &[_, dist] : distances)
        {
            if (dist > 0 && dist != std::numeric_limits<int>::max())
            { // Ignore self-distances and unreachable pairs
                totalDistance += dist;
                ++pairCount;
            }
        }
    }
    return pairCount > 0 ? static_cast<double>(totalDistance) / pairCount : 0.0;
}

// Function to get the shortest distance between two specific vertices (start and end)
int Tree::calculateShortestDistance(int start, int end) const
{
    auto it = shortestPathMatrix.find(start);
    if (it != shortestPathMatrix.end())
    {
        auto distIt = it->second.find(end);
        if (distIt != it->second.end())
        {
            return distIt->second; // Return precomputed shortest path from start to end
        }
    }
    return std::numeric_limits<int>::max(); // Return infinity if no path is found
}

// // Accessor function to get the edges in the MST
// const std::vector<std::tuple<int, int, int, int>> &Tree::getMSTEdges() const
// {
//     return mstEdges;
// }
