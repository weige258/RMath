#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include <vector>

using namespace std;

int main() {
    Vec<double, 2> vec2d = {1, 2.0f};
    Mat<int, 2, 2> mat = {{1, 2}, {3, 4.0}};

    cout << vec2d << endl;
    cout << mat << endl;
    return 0;
}