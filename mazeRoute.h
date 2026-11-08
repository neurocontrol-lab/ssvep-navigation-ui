#pragma once

#include <vector>

enum class RouteHint { Unavailable, Forward, Left, Right };

// Even wall rows encode the north edge, odd rows the west edge. The
// collision map has no additional south/east boundary walls.
class MazeRoute {
public:
    void build(const std::vector<std::vector<int>>& walls, int width, int height);
    RouteHint hint(double x, double y, double yaw, double originX,
                   double originY, double cellSize) const;

private:
    int width_ = 0, height_ = 0;
    std::vector<int> steps_;
    std::vector<int> directions_;
};
