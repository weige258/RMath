#include "rmath.hpp"

using namespace std;

int main()
{
    Vec4f v{1.0f, 2.0f, 3.0f, 4.0f};
    Mat4f m;

    Range<1,4,2> r;

    cout<<v[Range<0, 4, 2>{}];
    m[Range<0, 4, 2>{}, Range<0, 4, 2>{}]={1.0f, 3.0f, 2.0f, 4.0f};
    cout<<m[Range<0, 4, 2>{}, Range<0, 4, 2>{}];
    return 0;
}