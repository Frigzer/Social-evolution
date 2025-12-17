#include "Grid.hpp"

Grid::Grid(int w, int h)
    : width(w), height(h), agents(w* h) {}

Agent& Grid::get(int x, int y) {
    return agents[y * width + x];
}

const Agent& Grid::get(int x, int y) const {
    return agents[y * width + x];
}

/*
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
*/

static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int modWrap(int v, int m) {
    int r = v % m;
    return (r < 0) ? r + m : r;
}

static int reflect(int v, int m) {
    // odbicie: -1 -> 0, m -> m-1, m+1 -> m-2 itd.
    if (m <= 1) return 0;
    while (v < 0 || v >= m) {
        if (v < 0) v = -v - 1;
        if (v >= m) v = 2 * m - v - 1;
    }
    return v;
}

bool Grid::inBounds(int x, int y) const {
    return (x >= 0 && x < width&& y >= 0 && y < height);
}

int Grid::mapX(int x) const {
    switch (boundary) {
    case BoundaryMode::Torus:   return modWrap(x, width);
    case BoundaryMode::Clamp:   return clampInt(x, 0, width - 1);
    case BoundaryMode::Reflect: return reflect(x, width);
    }
    return modWrap(x, width);
}

int Grid::mapY(int y) const {
    switch (boundary) {
    case BoundaryMode::Torus:   return modWrap(y, height);
    case BoundaryMode::Clamp:   return clampInt(y, 0, height - 1);
    case BoundaryMode::Reflect: return reflect(y, height);
    }
    return modWrap(y, height);
}

std::vector<Agent*> Grid::getNeighbors(int x, int y) {
    std::vector<Agent*> neighbors;

    if (neighborhood == NeighborhoodType::Moore) {
        neighbors.reserve(8);

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;

                int rx = x + dx;
                int ry = y + dy;

                if (boundary == BoundaryMode::Absorbing) {
                    if (!inBounds(rx, ry)) continue;
                    neighbors.push_back(&get(rx, ry));
                }
                else {
                    neighbors.push_back(&get(mapX(rx), mapY(ry)));
                }
            }
        }
    }
    else { // Von Neumann
        neighbors.reserve(4);

        const int offsets[4][2] = {
            { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
        };

        for (auto& o : offsets) {
            int rx = x + o[0];
            int ry = y + o[1];

            if (boundary == BoundaryMode::Absorbing) {
                if (!inBounds(rx, ry)) continue;
                neighbors.push_back(&get(rx, ry));
            }
            else {
                neighbors.push_back(&get(mapX(rx), mapY(ry)));
            }
        }
    }

    return neighbors;
}

