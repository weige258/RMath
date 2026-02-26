#include "rmath.hpp"

using namespace std;

int main()
{
    Vec4f v{1.0f, 2.0f, 3.0f, 4.0f};
    Mat4f m;
    cout << m*v;
    
    return 0;
}