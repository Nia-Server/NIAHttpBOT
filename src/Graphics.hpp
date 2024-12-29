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
#include <bitset>
#include <cmath>

#define getIndex(x,y,z) ((x*sizeX+y)*sizeY+z)


class G {

private:
    int sizeX, sizeY, sizeZ;
    float scale;
    std::pair<float, float> rangeX, rangeY, rangeZ;
    std::vector<uint8_t> grid;
    std::vector<std::tuple<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector3f> > faces;

    G();

    inline void initializeGrid();
    inline bool mollerTrumbore(const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&,const Eigen::Vector3f&, const Eigen::Vector3f&); 

public:

    G(int,int,int);

    inline void clear();
    
    inline void setResultSize(int, int, int);
    inline void addVertice(const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&);
  
    void autoAdjust();
    void printGrid();
    void calcGrid();
    const std::vector<uint8_t>& getGrid();
    //bool pointInTriangle(const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&, const Eigen::Vector3f&);
    //bool pointIn3DModel(const Eigen::Vector3f&, const std::vector<Eigen::Vector3f>&, const std::vector<Eigen::Vector3i>&);

};

G::G(){ 
    initializeGrid();
}

G::G(int x, int y, int z){
    initializeGrid();
    setResultSize(x, y, z);
}

inline void G::initializeGrid() {
    sizeX=sizeY=sizeZ=0,
    scale = 1.f,
    rangeX=rangeY=rangeZ={+std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()},
    grid.clear(),
    faces.clear();
}

inline void G::clear(){
    initializeGrid();
}

inline void G::setResultSize(int x, int y, int z){
    sizeX=x, sizeY=y, sizeZ=z;
    grid.resize(x*y*z, 0);
}

inline void G::addVertice(const Eigen::Vector3f&a, const Eigen::Vector3f&b, const Eigen::Vector3f&c){

    faces.push_back({a, b, c});
    rangeX.first = std::min({rangeX.first, a[0], b[0], c[0]}),
    rangeX.second = std::max({rangeX.second, a[0], b[0], c[0]}),
    rangeY.first = std::min({rangeY.first, a[1], b[1], c[1]}),
    rangeY.second = std::max({rangeY.second, a[1], b[1], c[1]}),
    rangeZ.first = std::min({rangeZ.first, a[2], b[2], c[2]}),
    rangeZ.second = std::max({rangeZ.second, a[2], b[2], c[2]});

}

void G::autoAdjust(){

    float  
        lenX = std::abs(rangeX.second-rangeX.first), 
        lenY = std::abs(rangeY.second-rangeY.first),
        lenZ = std::abs(rangeZ.second-rangeZ.first);
    
    float 
        rateX = sizeX / lenX,
        rateY = sizeY / lenY,
        rateZ = sizeZ / lenZ;
    
    scale = std::min({rateX, rateY, rateZ});

    if(rangeX.first < 0.f) 
        for(auto &i : faces) 
            std::get<0>(i)[0]+=-rangeX.first, std::get<1>(i)[0]+=-rangeX.first, std::get<2>(i)[0]+=-rangeX.first;

    if(rangeY.first < 0.f) 
        for(auto &i : faces) 
            std::get<0>(i)[1]+=-rangeY.first, std::get<1>(i)[1]+=-rangeY.first, std::get<2>(i)[1]+=-rangeY.first;
    
    if(rangeZ.first < 0.f) 
        for(auto &i : faces) 
            std::get<0>(i)[2]+=-rangeZ.first, std::get<1>(i)[2]+=-rangeZ.first, std::get<2>(i)[2]+=-rangeZ.first;

    for(auto &i : faces)
        std::get<0>(i)*=scale, std::get<1>(i)*=scale, std::get<2>(i)*=scale;
    
    return ;
}


// bool G::pointInTriangle(const Eigen::Vector3f& pt, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3) {

//     Eigen::Vector3f v0 = p2 - p1,
//                     v1 = p3 - p1,
//                     v2 = pt - p1;

//     float   d00 = v0.dot(v0),
//             d01 = v0.dot(v1),
//             d11 = v1.dot(v1),
//             d20 = v2.dot(v0),
//             d21 = v2.dot(v1);
//     float   denom = d00 * d11 - d01 * d01;
    
//     float   v = (d11 * d20 - d01 * d21) / denom,
//             w = (d00 * d21 - d01 * d20) / denom;

//     float   u = 1.f - v - w;

//     return (u >= 0.f) && (v >= 0.f) && (w >= 0.f);
// }

inline bool G::mollerTrumbore(
    const Eigen::Vector3f& p0, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2,
    const Eigen::Vector3f& ro, const Eigen::Vector3f& rd = {11,45,14}) {
    
    // const float EPS = 1e-6;

    Eigen::Vector3f e1 = p1 - p0, 
                    e2 = p2 - p0, 
                    s  = ro - p0;
    
    Eigen::Vector3f s1 = rd.cross(e2),
                    s2 = s.cross(e1);

    float s1e1 = s1.dot(e1);

    float   t  = s2.dot(e2) / s1e1,
            b1 = s1.dot(s)  / s1e1,
            b2 = s2.dot(rd) / s1e1;

    return t>=0.f && b1>=0.f && b2>=0.f && (1-b1-b2)>=0.f;
}

void G::calcGrid(){
    for(int x = 0; x<sizeX; x++)
        for(int y = 0; y<sizeY; y++)
            for(int z = 0; z<sizeZ; z++) {
                int res = 0;
                Eigen::Vector3f xyz(x, y, z);
                for(auto &i : faces) 
                    res += mollerTrumbore(std::get<0>(i), std::get<1>(i), std::get<2>(i), xyz);
                grid[getIndex(x, y, z)] = res & 1;
            }

}

void G::printGrid() {

    for (int z = 0; z < sizeZ; ++z) 
        for (int y = 0; y < sizeY; ++y) 
            for (int x = 0; x < sizeX; ++x) 
                if (grid[getIndex(x, y, z)])  
                    std::cout << "Block at (" << x << ", " << y << ", " << z << ")\n";
    return ;
}

const std::vector<uint8_t>& G::getGrid(){
    return grid;
}

#undef getIndex

#endif
