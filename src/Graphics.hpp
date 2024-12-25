/* 

MIT LICENSE

Copyright (C) 2024 jiansyuan

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/



#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <Eigen/Eigen>
#include <algorithm>
#include <cmath>

const int GRID_SIZE = 20; // O(N^3)

class G {
private:
    bool grid[GRID_SIZE][GRID_SIZE][GRID_SIZE];

public:
    float scale = 100.f;
    G();
    void initializeGrid();
    void addTriangleToGrid(const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&);
    bool pointInTriangle(const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&);
    bool pointIn3DModel(const Eigen::Vector3f& pt, const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3i>& faces);
    void printGrid();
};

G::G(){
    initializeGrid();
}



void G::initializeGrid() {
    memset(grid, 0, sizeof grid);
}

void G::addTriangleToGrid(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3) {

    int minX = std::floor(std::min({p1.x(), p2.x(), p3.x()})),
        maxX = std::ceil(std::max({p1.x(), p2.x(), p3.x()})),
        minY = std::floor(std::min({p1.y(), p2.y(), p3.y()})),
        maxY = std::ceil(std::max({p1.y(), p2.y(), p3.y()})),
        minZ = std::floor(std::min({p1.z(), p2.z(), p3.z()})),
        maxZ = std::ceil(std::max({p1.z(), p2.z(), p3.z()}));

    for (int x = minX; x <= maxX; ++x) 
        for (int y = minY; y <= maxY; ++y) 
            for (int z = minZ; z <= maxZ; ++z) {
                Eigen::Vector3f pt(x, y, z);
                if (pointInTriangle(pt, p1, p2, p3)) grid[x][y][z] = true;
            }

    return ;
        
}

bool G::pointInTriangle(const Eigen::Vector3f& pt, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3) {

    Eigen::Vector3f v0 = p2 - p1;
    Eigen::Vector3f v1 = p3 - p1;
    Eigen::Vector3f v2 = pt - p1;

    float   d00 = v0.dot(v0),
            d01 = v0.dot(v1),
            d11 = v1.dot(v1),
            d20 = v2.dot(v0),
            d21 = v2.dot(v1);
    float   denom = d00 * d11 - d01 * d01;

    float   v = (d11 * d20 - d01 * d21) / denom,
            w = (d00 * d21 - d01 * d20) / denom,
            u = 1.f - v - w;

    return (u >= 0.f) && (v >= 0.f) && (w >= 0.f);
}

bool G::pointIn3DModel(const Eigen::Vector3f& pt, const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3i>& faces) {

    int intersectionCount = 0;

    for (const auto& face : faces) {
        Eigen::Vector3f p1 = vertices[face[0]],
                        p2 = vertices[face[1]],
                        p3 = vertices[face[2]];

        if (pointInTriangle(pt, p1, p2, p3))  intersectionCount++;
    }

    return intersectionCount &1;
}

void G::printGrid() {

    for (int z = 0; z < GRID_SIZE; ++z) 
        for (int y = 0; y < GRID_SIZE; ++y) 
            for (int x = 0; x < GRID_SIZE; ++x) 
                if (grid[x][y][z])  
                    std::cout << "Block at (" << x << ", " << y << ", " << z << ")\n";
    return ;
}

#endif
