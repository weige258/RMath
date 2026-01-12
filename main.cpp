#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include <vector>

using namespace std;

int main() {
    Vec<double, 2> vec2d = {1, 2.0f};
    Mat<int, 2, 2> mat = {{1, 2}, {3, 4.0}};
    Range<5>(0,1,1);
    return 0;
}