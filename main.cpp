#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include "range.hpp"
#include <vector>

using namespace std;

int main() {
    Vec3f v1(1.0f, 2.0f, 3.0f);
    Vec3f v2(4.0f, 5.0f, 6.0f);
    Vec3f v3 = v1 + v2;
    Mat<float,3,3> m(1,2,2,4,5,6.2,7,8,1);
    cout << v3 * Inverse(m) * m << endl;
    
    
    return 0;
}