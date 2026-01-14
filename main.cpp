#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include "range.hpp"
#include <vector>

using namespace std;

int main() {
    Mat<float,4,4> mat1=2;
    Mat<float,4,4> mat2=Mat<float,4,4>::MakeIdentity();
    cout << Hadamard(mat1,mat1,mat2) << endl;
    return 0;
}