#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include "range.hpp"
#include <vector>

using namespace std;

int main() {
    Mat<int, 3, 3> mat = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    mat[1,2]=2;
    cout << mat;
    cout<<mat[StaticRange<0, 3, 1>{}, StaticRange<2, 0, -1>{}];
    
    
    
    return 0;
}