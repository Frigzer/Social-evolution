#pragma once
#include <vector>
#include "Agent.hpp"

enum class BoundaryMode {
    Torus,
    Clamp,
    Reflect,
    Absorbing
};

enum class NeighborhoodType {
    Moore,        // 8 s¹siadów
    VonNeumann    // 4 s¹siadów
};


class Grid {
public:
    int width, height;

    BoundaryMode boundary = BoundaryMode::Torus;
    NeighborhoodType neighborhood = NeighborhoodType::Moore;

    std::vector<Agent*> agents;

    Grid(int w, int h);

    Agent*& get(int x, int y);
    const Agent* get(int x, int y) const;

    bool inBounds(int x, int y) const;
    bool isEmpty(int x, int y) const;

    int mapX(int x) const;
    int mapY(int y) const;
    
    std::vector<std::pair<int, int>> getNeighborCoords(int x, int y) const;
};
