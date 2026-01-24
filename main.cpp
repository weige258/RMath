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
    cout << v << endl;
    return 0;
}