#include <iostream>
#include <array>
#include <concepts>
#include <list>
#include "vec.hpp"

using namespace std;

int main()
{
    int a = Dot(Vec(1, 2, 3), Vec(4, 0.2, 0));
    cout << a << endl;
    return 0;
}