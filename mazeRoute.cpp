#include "mazeRoute.h"

#include <cmath>
#include <queue>

namespace {
constexpr int dx[] = {0, 1, 0, -1};
constexpr int dy[] = {-1, 0, 1, 0};
constexpr double pi = 3.14159265358979323846;
constexpr double turnTolerance = 12 * pi / 180;
}

void MazeRoute::build(const std::vector<std::vector<int>>& walls, int width, int height) {
    width_ = height_ = 0;
    steps_.clear();
    directions_.clear();
    if (width <= 0 || height <= 0 || walls.size() != 2u * height) return;
    for (const auto& row : walls)
        if (row.size() != static_cast<size_t>(width)) return;

    width_ = width;
    height_ = height;
    steps_.assign(width * height, -1);
    directions_.assign(width * height, -1);
    auto inside = [=](int x, int y) { return x >= 0 && y >= 0 && x < width && y < height; };
    auto blocked = [&](int x, int y, int direction) {
        switch (direction) {
        case 0: return walls[2 * y][x] != 0;
        case 1: return x + 1 < width && walls[2 * y + 1][x + 1] != 0;
        case 2: return y + 1 < height && walls[2 * (y + 1)][x] != 0;
        default: return walls[2 * y + 1][x] != 0;
        }
    };

    std::queue<int> pending;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int d = 0; d < 4; ++d) {
                if (!inside(x + dx[d], y + dy[d]) && !blocked(x, y, d)) {
                    const int index = y * width + x;
                    steps_[index] = 1;
                    directions_[index] = d;
                    pending.push(index);
                    break;
                }
            }
        }
    }
    // Reverse breadth-first search from all open perimeter edges.
    while (!pending.empty()) {
        const int index = pending.front();
        pending.pop();
        const int x = index % width, y = index / width;
        for (int d = 0; d < 4; ++d) {
            const int nx = x + dx[d], ny = y + dy[d];
            if (!inside(nx, ny) || blocked(x, y, d)) continue;
            const int neighbor = ny * width + nx;
            if (steps_[neighbor] >= 0) continue;
            steps_[neighbor] = steps_[index] + 1;
            directions_[neighbor] = (d + 2) % 4;
            pending.push(neighbor);
        }
    }
}

RouteHint MazeRoute::hint(double x, double y, double yaw, double originX,
                         double originY, double cellSize) const {
    if (width_ == 0 || cellSize <= 0) return RouteHint::Unavailable;
    const double column = (x - originX) / cellSize, row = (y - originY) / cellSize;
    if (column < -0.5 || row < -0.5 || column > width_ + 0.5 || row > height_ + 0.5)
        return RouteHint::Unavailable;
    // Victory triggers slightly beyond the perimeter. Continue along the
    // boundary cell's exit step until the game registers the exit.
    int cx = static_cast<int>(std::floor(column)), cy = static_cast<int>(std::floor(row));
    if (cx < 0) cx = 0;
    if (cy < 0) cy = 0;
    if (cx >= width_) cx = width_ - 1;
    if (cy >= height_) cy = height_ - 1;
    const int direction = directions_[cy * width_ + cx];
    if (direction < 0) return RouteHint::Unavailable;
    const double targetX = originX + (cx + dx[direction] + 0.5) * cellSize;
    const double targetY = originY + (cy + dy[direction] + 0.5) * cellSize;
    const double angle = std::remainder(std::atan2(targetY - y, targetX - x) - yaw, 2 * pi);
    if (std::abs(angle) <= turnTolerance) return RouteHint::Forward;
    return angle > 0 ? RouteHint::Right : RouteHint::Left;
}
