#include "Grid.hpp"

Grid::Grid(int w, int h)
    : width(w), height(h), agents(w* h) {}

Agent& Grid::get(int x, int y) {
    return agents[y * width + x];
}

const Agent& Grid::get(int x, int y) const {
    return agents[y * width + x];
}

std::vector<Agent*> Grid::getNeighbors(int x, int y) {
    std::vector<Agent*> neighbors;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + width) % width;
            int ny = (y + dy + height) % height;
            neighbors.push_back(&get(nx, ny));
        }
    }
    return neighbors;
}
