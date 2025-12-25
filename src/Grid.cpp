#include "Grid.hpp"

static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int modWrap(int v, int m) {
    int r = v % m;
    return (r < 0) ? r + m : r;
}

static int reflectIndex(int v, int m) {
    // odbicie: -1 -> 0, m -> m-1, m+1 -> m-2 itd.
    if (m <= 1) return 0;
    while (v < 0 || v >= m) {
        if (v < 0) v = -v - 1;
        if (v >= m) v = 2 * m - v - 1;
    }
    return v;
}

Grid::Grid(int w, int h)
    : width(w), height(h), agents(w* h, nullptr) {}

Agent*& Grid::get(int x, int y) {
    return agents[y * width + x];
}

const Agent* Grid::get(int x, int y) const {
    return agents[y * width + x];
}

bool Grid::inBounds(int x, int y) const {
    return (x >= 0 && x < width&& y >= 0 && y < height);
}

bool Grid::isEmpty(int x, int y) const {
    return get(x, y) == nullptr;
}

int Grid::mapX(int x) const {
    switch (boundary) {
    case BoundaryMode::Torus:   return modWrap(x, width);
    case BoundaryMode::Clamp:   return clampInt(x, 0, width - 1);
    case BoundaryMode::Reflect: return reflectIndex(x, width);
    case BoundaryMode::Absorbing:
        // dla Absorbing mapX/mapY nie są używane (obsługujemy to w getNeighborCoords)
        return x;
    }
    return modWrap(x, width);
}

int Grid::mapY(int y) const {
    switch (boundary) {
    case BoundaryMode::Torus:   return modWrap(y, height);
    case BoundaryMode::Clamp:   return clampInt(y, 0, height - 1);
    case BoundaryMode::Reflect: return reflectIndex(y, height);
    case BoundaryMode::Absorbing:
        return y;
    }
    return modWrap(y, height);
}

std::vector<std::pair<int, int>> Grid::getNeighborCoords(int x, int y) const {
    std::vector<std::pair<int, int>> out;

    if (neighborhood == NeighborhoodType::Moore) {
        out.reserve(8);
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;

                int rx = x + dx;
                int ry = y + dy;

                if (boundary == BoundaryMode::Absorbing) {
                    if (!inBounds(rx, ry)) continue;
                    out.emplace_back(rx, ry);
                }
                else {
                    out.emplace_back(mapX(rx), mapY(ry));
                }
            }
    }
    else { // VonNeumann
        out.reserve(4);
        const int off[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
        for (auto& o : off) {
            int rx = x + o[0], ry = y + o[1];
            if (boundary == BoundaryMode::Absorbing) {
                if (!inBounds(rx, ry)) continue;
                out.emplace_back(rx, ry);
            }
            else {
                out.emplace_back(mapX(rx), mapY(ry));
            }
        }
    }

    return out;
}

