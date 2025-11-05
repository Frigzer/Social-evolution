#pragma once
#include <vector>
#include "Agent.hpp"

class Grid {
public:
    int width, height;
    std::vector<Agent> agents;

    Grid(int w, int h);
    Agent& get(int x, int y);
    const Agent& get(int x, int y) const;
    std::vector<Agent*> getNeighbors(int x, int y);
};
