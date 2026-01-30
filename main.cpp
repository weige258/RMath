#include "rmath.hpp"

using namespace std;

int main()
{

    Vec4f v(1.22, 2, 3.2, 1);
    v*Mat4f::MakeIdentity();
    cout << sizeof(v) << v.SizeInBytes() << endl;
    return 0;
}