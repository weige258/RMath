#include "vec.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <list>

int main() {
    std::cout << "=== RMath Vector Library Test ===" << std::endl;

    // 测试基本构造函数
    std::cout << "\n1. Testing constructors:" << std::endl;
    Vec3f v1;                    // 默认构造，全为0
    Vec3f v2(5.0f);              // 单个数值构造
    Vec3f v3(1.0f, 2.0f, 3.0f);  // 多参数构造
    Vec3f v4(v3);                // 拷贝构造
    
    std::cout << "v1 (default): " << v1 << std::endl;
    std::cout << "v2 (fill 5.0f): " << v2 << std::endl;
    std::cout << "v3 (1,2,3): " << v3 << std::endl;
    std::cout << "v4 (copy of v3): " << v4 << std::endl;

    // 测试数组构造
    float arr[] = {4.0f, 5.0f, 6.0f};
    Vec3f v5(arr);
    std::cout << "v5 (from array {4,5,6}): " << v5 << std::endl;

    // 测试std::array构造
    std::array<float, 3> std_arr = {7.0f, 8.0f, 9.0f};
    Vec3f v6(std_arr);
    std::cout << "v6 (from std::array {7,8,9}): " << v6 << std::endl;

    // 测试std::vector构造
    std::vector<float> vec_data = {10.0f, 11.0f, 12.0f};
    Vec3f v7(vec_data);
    std::cout << "v7 (from std::vector {10,11,12}): " << v7 << std::endl;

    // 测试std::list构造
    std::list<float> list_data = {13.0f, 14.0f, 15.0f};
    Vec3f v8(list_data);
    std::cout << "v8 (from std::list {13,14,15}): " << v8 << std::endl;

    // 测试std::span构造
    std::span<const float, 3> span_data = std_arr;
    Vec3f v9(span_data);
    std::cout << "v9 (from std::span {7,8,9}): " << v9 << std::endl;

    // 测试类型转换构造
    Vec3d v10(v3);  // 从float转到double
    std::cout << "v10 (from Vec3f to Vec3d): " << v10 << std::endl;

    // 测试访问操作符
    std::cout << "\n2. Testing access operators:" << std::endl;
    std::cout << "v3[0]: " << v3[0] << ", v3[1]: " << v3[1] << ", v3[2]: " << v3[2] << std::endl;
    std::cout << "v3.x(): " << v3.x() << ", v3.y(): " << v3.y() << ", v3.z(): " << v3.z() << std::endl;

    // 测试算术运算符
    std::cout << "\n3. Testing arithmetic operators:" << std::endl;
    Vec3f a(1.0f, 2.0f, 3.0f);
    Vec3f b(4.0f, 5.0f, 6.0f);
    
    std::cout << "a: " << a << ", b: " << b << std::endl;
    std::cout << "a + b: " << (a + b) << std::endl;
    std::cout << "a - b: " << (a - b) << std::endl;
    std::cout << "a * b: " << (a * b) << std::endl;
    std::cout << "a / b: " << (a / b) << std::endl;
    std::cout << "a + 2.0f: " << (a + 2.0f) << std::endl;
    std::cout << "a - 1.0f: " << (a - 1.0f) << std::endl;
    std::cout << "a * 3.0f: " << (a * 3.0f) << std::endl;
    std::cout << "a / 2.0f: " << (a / 2.0f) << std::endl;

    // 测试复合赋值运算符
    std::cout << "\n4. Testing compound assignment operators:" << std::endl;
    Vec3f c(1.0f, 1.0f, 1.0f);
    std::cout << "c before: " << c << std::endl;
    c += b;
    std::cout << "c += b: " << c << std::endl;
    c -= Vec3f(1.0f, 1.0f, 1.0f);
    std::cout << "c -= (1,1,1): " << c << std::endl;
    c *= 2.0f;
    std::cout << "c *= 2.0f: " << c << std::endl;
    c /= 2.0f;
    std::cout << "c /= 2.0f: " << c << std::endl;

    // 测试叉乘 (仅适用于3D向量)
    std::cout << "\n5. Testing cross product:" << std::endl;
    Vec3f cross_a(1.0f, 0.0f, 0.0f);
    Vec3f cross_b(0.0f, 1.0f, 0.0f);
    std::cout << "cross_a: " << cross_a << ", cross_b: " << cross_b << std::endl;
    std::cout << "cross_a ^ cross_b: " << (cross_a ^ cross_b) << std::endl;

    // 测试取负
    std::cout << "\n6. Testing unary minus:" << std::endl;
    Vec3f neg_test(1.0f, -2.0f, 3.0f);
    std::cout << "neg_test: " << neg_test << std::endl;
    std::cout << "-neg_test: " << (-neg_test) << std::endl;

    // 测试比较操作符
    std::cout << "\n7. Testing comparison operators:" << std::endl;
    Vec3f eq1(1.0f, 2.0f, 3.0f);
    Vec3f eq2(1.0f, 2.0f, 3.0f);
    Vec3f neq(1.0f, 2.0f, 4.0f);
    std::cout << "eq1 == eq2: " << (eq1 == eq2) << std::endl;
    std::cout << "eq1 == neq: " << (eq1 == neq) << std::endl;

    // 测试模长计算
    std::cout << "\n8. Testing Length function:" << std::endl;
    Vec3f len_test(3.0f, 4.0f, 0.0f);
    std::cout << "len_test: " << len_test << std::endl;
    std::cout << "Length(len_test): " << Length(len_test) << std::endl; // 应该是5

    // 测试归一化
    std::cout << "\n9. Testing Normalize function:" << std::endl;
    Vec3f norm_test(3.0f, 4.0f, 0.0f);
    Vec3f normalized = Normalize(norm_test);
    std::cout << "norm_test: " << norm_test << std::endl;
    std::cout << "Normalize(norm_test): " << normalized << std::endl;
    std::cout << "Length(normalized): " << Length(normalized) << std::endl; // 应该接近1

    // 测试点积
    std::cout << "\n10. Testing Dot function:" << std::endl;
    Vec3f dot_a(1.0f, 2.0f, 3.0f);
    Vec3f dot_b(4.0f, 5.0f, 6.0f);
    std::cout << "dot_a: " << dot_a << ", dot_b: " << dot_b << std::endl;
    std::cout << "Dot(dot_a, dot_b): " << Dot(dot_a, dot_b) << std::endl; // 1*4+2*5+3*6=32

    // 测试拼接
    std::cout << "\n11. Testing Cat function:" << std::endl;
    Vec2f cat_a(1.0f, 2.0f);
    Vec2f cat_b(3.0f, 4.0f);
    auto cat_result = Cat(cat_a, cat_b);
    std::cout << "cat_a: " << cat_a << ", cat_b: " << cat_b << std::endl;
    std::cout << "Cat(cat_a, cat_b): " << cat_result << std::endl;

    // 测试距离计算
    std::cout << "\n12. Testing Distance function:" << std::endl;
    Vec3f dist_a(0.0f, 0.0f, 0.0f);
    Vec3f dist_b(3.0f, 4.0f, 0.0f);
    std::cout << "dist_a: " << dist_a << ", dist_b: " << dist_b << std::endl;
    std::cout << "Distance(dist_a, dist_b): " << Distance(dist_a, dist_b) << std::endl; // 应该是5

    // 测试线性插值
    std::cout << "\n13. Testing Lerp function:" << std::endl;
    Vec3f lerp_a(0.0f, 0.0f, 0.0f);
    Vec3f lerp_b(10.0f, 10.0f, 10.0f);
    auto lerp_result = Lerp(lerp_a, lerp_b, 0.5f);
    std::cout << "lerp_a: " << lerp_a << ", lerp_b: " << lerp_b << std::endl;
    std::cout << "Lerp(a, b, 0.5): " << lerp_result << std::endl; // 应该是(5,5,5)

    // 测试投影
    std::cout << "\n14. Testing Project function:" << std::endl;
    Vec3f proj_a(1.0f, 2.0f, 0.0f);
    Vec3f proj_b(3.0f, 0.0f, 0.0f);
    auto proj_result = Project(proj_a, proj_b);
    std::cout << "proj_a: " << proj_a << ", proj_b: " << proj_b << std::endl;
    std::cout << "Project(proj_a, proj_b): " << proj_result << std::endl;

    // 测试反射
    std::cout << "\n15. Testing Reflect function:" << std::endl;
    Vec3f reflect_i(1.0f, -1.0f, 0.0f);  // 入射向量
    Vec3f reflect_n(0.0f, 1.0f, 0.0f);   // 法向量
    auto reflect_result = Reflect(reflect_i, reflect_n);
    std::cout << "reflect_i: " << reflect_i << ", reflect_n: " << reflect_n << std::endl;
    std::cout << "Reflect(i, n): " << reflect_result << std::endl;

    // 测试转换方法
    std::cout << "\n16. Testing conversion methods:" << std::endl;
    Vec3f conv_test(1.0f, 2.0f, 3.0f);
    
    auto arr_conv = conv_test.ToArray();
    std::cout << "ToArray(): [" << arr_conv[0] << ", " << arr_conv[1] << ", " << arr_conv[2] << "]" << std::endl;
    
    auto list_conv = conv_test.ToList();
    std::cout << "ToList(): ";
    for(auto val : list_conv) std::cout << val << " ";
    std::cout << std::endl;
    
    auto vec_conv = conv_test.ToVector();
    std::cout << "ToVector(): ";
    for(auto val : vec_conv) std::cout << val << " ";
    std::cout << std::endl;

    // 测试迭代器
    std::cout << "\n17. Testing iterators:" << std::endl;
    Vec3f iter_test(5.0f, 6.0f, 7.0f);
    std::cout << "Using iterators: ";
    for(auto it = iter_test.begin(); it != iter_test.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // 测试查询方法
    std::cout << "\n18. Testing query methods:" << std::endl;
    std::cout << "Vec3f::size(): " << Vec3f::size() << std::endl;
    std::cout << "Vec3f::size_in_bytes(): " << Vec3f::size_in_bytes() << std::endl;

    // 测试不同类型维度的向量
    std::cout << "\n19. Testing different vector types:" << std::endl;
    Vec2i vi2(1, 2);
    Vec4d vd4(1.0, 2.0, 3.0, 4.0);
    std::cout << "Vec2i(1,2): " << vi2 << std::endl;
    std::cout << "Vec4d(1,2,3,4): " << vd4 << std::endl;

    std::cout << "\n=== All tests completed successfully! ===" << std::endl;

    return 0;
}