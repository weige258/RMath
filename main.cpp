#include "vec.hpp"
#include <iostream>
#include "mat.hpp"
#include "range.hpp"
#include <vector>

using namespace std;

int main() {

    
   Mat<float,4,4> view_matrix = Mat<float,4,4>::MakeIdentity();
   view_matrix.SetViewMatrix(Vec<float,3>(1,2,2),Vec<float,3>(0,0,1),Vec<float,3>(0,1,1));
   Mat4f projection_matrix = Mat4f::MakeIdentity();
   projection_matrix.SetPerspectiveMartrix(90.0f, 1.0f, 0.1f, 100.0f);
   cout << Vec4f(1,2,3,1)*view_matrix*Inverse(view_matrix)<< endl; 
   cout << Vec4f(1,2,3,1)*view_matrix*projection_matrix<< endl; 
    return 0;
}