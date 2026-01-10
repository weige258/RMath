#include <iostream>
#include <array>
#include <concepts>
#include <list>
#include "vec.hpp"

using namespace std;

int main()
{
    // 测试从列表推导类型

    Vec v1={1.0f, 2.0f, 3.0f, 4.0f};
    cout << "v1.x(): " << v1.x() << ", v1.y(): " << v1.y() << endl;
    
    return 0;
}