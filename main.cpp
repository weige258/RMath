#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include "range.hpp"
#include <vector>
#include <span>

using namespace std;

int main()
{

    Vec4f v(1.22, 2, 3.2, 1);
    v.xyz() = {1, 2, 3};
    Mat4d m=Mat4d::MakeIdentity();
    m[Range<0,2,1>(),Range<0,4,1>()]={1.0,21.0,10.0,4.0,1.0,2.0,3.2,4.0};
    cout<<(v*m*Inverse(m))<<endl;
    cout<<Degree(Vec3f(1,0,0),Vec3f(0,1,0))<<endl;
    return 0;
}