// ============================================================================
// RMath 单元测试
// ----------------------------------------------------------------------------
// 覆盖: Range / Vec(构造、转换、访问、运算、算法、扩展) / VecView /
//       Mat(构造、转换、访问、运算、算法、扩展) / MatView /
//       Camera(视图/投影) / 标量工具 / SIMD 回归 / 异常路径
//
// 编译运行:
//   g++ -std=c++23 -mavx2 -mfma test.cpp -o test.exe && test.exe
// ============================================================================
#include "rmath.hpp"

#include <iostream>
#include <cmath>
#include <numbers>
#include <array>
#include <vector>
#include <list>
#include <span>
#include <stdexcept>

using namespace std;

// ============================ 测试框架 ============================
static int g_total = 0;
static int g_failed = 0;

static void Section(const char *name)
{
    cout << "\n========== " << name << " ==========\n";
}

static void Check(bool ok, const char *name)
{
    ++g_total;
    if (!ok)
    {
        ++g_failed;
        cout << "  [FAIL] " << name << "\n";
    }
    else
    {
        cout << "  [PASS] " << name << "\n";
    }
}

static bool Near(double a, double b, double eps = 1e-5)
{
    return std::fabs(a - b) <= eps;
}

// ==================== 编译期常量求值检查 ====================
static_assert(Range<0, 5, 2>::Size() == 3, "Range compile-time size");
static_assert(Range<5, 0, -2>::Size() == 3, "Range negative step compile-time");
static_assert(Cross(Vec3i{1, 0, 0}, Vec3i{0, 1, 0}) == Vec3i{0, 0, 1}, "Cross constexpr");
static_assert(Det(Mat2i{1, 2, 3, 4}) == -2, "Det constexpr");
static_assert(Trace(Mat2i{1, 2, 3, 4}) == 5, "Trace constexpr");
static_assert(Transpose(Mat2i{1, 2, 3, 4}) == Mat2i{1, 3, 2, 4}, "Transpose constexpr");
static_assert(Mat3d::MakeIdentity() == Mat3d{1, 0, 0, 0, 1, 0, 0, 0, 1}, "MakeIdentity constexpr");
static_assert(MakeDiagonal(Vec3d{1, 2, 3}) == Mat3d{1, 0, 0, 0, 2, 0, 0, 0, 3}, "MakeDiagonal constexpr");
static_assert(Diagonal(Mat3d{1, 2, 3, 4, 5, 6, 7, 8, 9}) == Vec3d{1, 5, 9}, "Diagonal constexpr");
static_assert(OuterProduct(Vec2i{1, 2}, Vec2i{3, 4}) == Mat2i{3, 4, 6, 8}, "OuterProduct constexpr");
static_assert(Sum(Vec3i{1, 2, 3}) == 6, "Sum constexpr");
static_assert(Clamp(5, 0, 10) == 5, "scalar Clamp constexpr");
static_assert(Remap(5, 0, 10, 0, 100) == 50, "Remap constexpr");
static_assert(Step(5, 3) == 0, "Step constexpr");
static_assert(Min(Vec3i{1, 2, 3}, Vec3i{0, 1, 4}) == Vec3i{0, 1, 3}, "Min constexpr");
static_assert(Max(Vec3i{1, 2, 3}, Vec3i{0, 1, 4}) == Vec3i{1, 2, 4}, "Max constexpr");
static_assert(MaxIndex(Vec3i{1, 3, 2}) == 1 && MinIndex(Vec3i{1, 3, 2}) == 0, "Index constexpr");
// 注: Midpoint 走 Vec 运算符; 用 N=1(永不触发 SIMD) 保证各配置下均可常量求值
static_assert(Midpoint(Vec<double, 1>{0}, Vec<double, 1>{2}) == Vec<double, 1>{1}, "Midpoint constexpr");

// ============================ Range ============================
static void test_range()
{
    Section("Range");

    Range<0, 5, 2> r;
    Check(r.Size() == 3, "Size");
    Check(r[0] == 0 && r[1] == 2 && r[2] == 4, "operator[]");

    vector<int> rv = r;
    list<int> rl = r;
    array<int, Range<0, 5, 2>::Size()> ra = r;
    Check(rv.size() == 3 && rv[2] == 4, "to vector");
    Check(rl.size() == 3 && *rl.begin() == 0, "to list");
    Check(ra[1] == 2, "to array");

    Range<5, 0, -1> rn;
    Check(rn.Size() == 5 && rn[0] == 5 && rn[4] == 1, "negative step");
    Range<0, 0, 1> re;
    Check(re.Size() == 0, "empty range");

    auto vals = Range<0, 6, 2>::Values();
    Check(vals[0] == 0 && vals[2] == 4, "Values()");

    int sum = 0;
    for (int v : Range<1, 4, 1>()) sum += v;
    Check(sum == 6, "iteration sum");
}

// ============================ Vec: 基础 ============================
static void test_vec_basic()
{
    Section("Vec: 构造 / 转换 / 访问");

    // 构造
    Vec3f a;                 // 零向量
    Vec3f b(2.0f);           // 填充标量
    Vec3f c{1, 2, 3};        // 初始化列表
    Vec3f d(1, 2, 3);        // 变参
    Check(a[0] == 0.0f && b[1] == 2.0f && c[2] == 3.0f && d[1] == 2.0f, "基本构造");

    // 容器构造
    float arr[3] = {4, 5, 6};
    Vec3f e(arr);
    array<float, 3> ar = {7, 8, 9};
    Vec3f f(ar);
    vector<float> vec = {1, 2, 3};
    Vec3f g(vec);
    list<float> lst = {2, 3, 4};
    Vec3f h(lst);
    span<const float, 3> sp(arr);
    Vec3f i(sp);
    Check(e[0] == 4.0f && f[2] == 9.0f && g[1] == 2.0f && h[0] == 2.0f && i[1] == 5.0f, "容器构造");

    // 转换到容器
    array<float, 3> ca = c;
    vector<float> cv = c;
    list<float> cl = c;
    span<const float, 3> cs = c;
    Check(ca[1] == 2.0f && cv[2] == 3.0f && *cl.begin() == 1.0f && cs[1] == 2.0f, "转换到容器");

    // 类型转换构造
    Vec3i vi{1, 2, 3};
    Vec3f vf(vi);
    Vec3d vd(vf);
    Check(vf[0] == 1.0f && vd[2] == 3.0, "类型转换构造");

    // 拷贝 / 移动 / 赋值
    Vec3f src{1, 2, 3};
    Vec3f cp(src);
    Vec3f mv(std::move(src));
    Check(cp[0] == 1.0f && mv[1] == 2.0f, "拷贝/移动构造");

    Vec3f as{1, 2, 3};
    Vec3f as2 = as;
    as2 = as;
    Check(as2[0] == 1.0f, "拷贝赋值");
    as2 = 5.0f;
    Check(as2[0] == 5.0f && as2[2] == 5.0f, "标量赋值");

    // 访问器
    Vec4f v{1, 2, 3, 4};
    Check(v.X() == 1.0f && v.Y() == 2.0f && v.Z() == 3.0f && v.W() == 4.0f, "X/Y/Z/W");
    v.X() = 10.0f;
    Check(v[0] == 10.0f, "X() 写");

    // 实例 Set 方法
    Vec4f sv{0, 0, 0, 0};
    sv.SetValue(1.0f);
    Check(sv[0] == 1.0f && sv[3] == 1.0f, "SetValue");
    sv.SetX(2.0f).SetY(3.0f).SetZ(4.0f).SetW(5.0f);
    Check(sv.X() == 2.0f && sv.Y() == 3.0f && sv.Z() == 4.0f && sv.W() == 5.0f, "SetX/SetY/SetZ/SetW");
    sv.SetXYZ(6.0f, 7.0f, 8.0f);
    Check(sv[0] == 6.0f && sv[1] == 7.0f && sv[2] == 8.0f, "SetXYZ 标量");
    sv.SetRGB(Vec3f{1, 2, 3});
    Check(sv[0] == 1.0f && sv[2] == 3.0f, "SetRGB 向量");

    // const 访问器
    const Vec4f cv4{1, 2, 3, 4};
    Check(cv4.X() == 1.0f && cv4.W() == 4.0f, "const 访问器");
    // const 编译期索引检查重载 (越界索引会在编译期报错)
    const Vec3f c3{1, 2, 3};
    Check(c3[Detail::CompileTimeIndexCheckVec<3>(std::integral_constant<std::size_t, 1>{})] == 2.0f,
          "const 编译期索引检查");
    Check(c3[Detail::CompileTimeIndexCheckVec<3>(std::integral_constant<std::size_t, 2>{})] == 3.0f,
          "const 编译期索引检查(2)");
    Vec3f nc3{1, 2, 3};
    Check(nc3[Detail::CompileTimeIndexCheckVec<3>(std::integral_constant<std::size_t, 0>{})] == 1.0f,
          "编译期索引检查");
    auto xyz = cv4.XYZ(); // 返回 Vec3f 拷贝
    Check(xyz.Size() == 3 && xyz[2] == 3.0f, "const XYZ()");
    auto rng = cv4[Range<1, 4, 2>{}]; // {1,3} -> {2,4}
    Check(rng.Size() == 2 && rng[0] == 2.0f && rng[1] == 4.0f, "const v[Range]");

    // 静态查询
    Check(Vec3f::Size() == 3 && Vec4f::SizeInBytes() == 16, "Size/SizeInBytes");

    // 指针转换 (explicit)
    const float *p = static_cast<const float *>(cv4);
    Check(p[0] == 1.0f && p[3] == 4.0f, "指针转换");

    // 迭代器 (v = {10,2,3,4}, v.X() 已被改为 10)
    float sum = 0;
    for (float x : v) sum += x;
    Check(Near(sum, 19.0f), "迭代器");
}

// ============================ Vec: 运算 ============================
static void test_vec_operators()
{
    Section("Vec: 运算符");

    Vec3d a{1, 2, 3}, b{4, 5, 6};
    auto s = a + b;    // {5,7,9}
    auto d = b - a;    // {3,3,3}
    auto m = a * b;    // {4,10,18}
    auto dv = b / Vec3d{1, 2, 3}; // {4,2.5,2}
    Check(Near(s[0], 5.0) && Near(d[1], 3.0) && Near(m[2], 18.0) && Near(dv[1], 2.5), "Vec 逐元素运算");

    auto su = a + 1.0;   // {2,3,4}
    auto ud = 1.0 + a;
    auto su2 = a - 1.0;  // {0,1,2}
    auto rs = 5.0 - a;   // {4,3,2}
    auto mu = a * 2.0;   // {2,4,6}
    auto um = 2.0 * a;
    auto di = a / 2.0;   // {0.5,1,1.5}
    auto ri = 6.0 / a;   // {6,3,2}
    Check(Near(su[0], 2.0) && Near(ud[1], 3.0) && Near(su2[0], 0.0) && Near(rs[2], 2.0) &&
          Near(mu[1], 4.0) && Near(um[2], 6.0) && Near(di[0], 0.5) && Near(ri[1], 3.0),
          "标量混合运算(双向)");

    auto neg = -a;
    Check(Near(neg[1], -2.0), "一元负号");

    // 复合赋值
    Vec3d c{1, 2, 3};
    c += Vec3d{1, 1, 1}; // {2,3,4}
    c -= 1.0;            // {1,2,3}
    c *= Vec3d{2, 2, 2}; // {2,4,6}
    c *= 2.0;            // {4,8,12}
    c /= Vec3d{2, 4, 6}; // {2,2,2}
    c /= 2.0;            // {1,1,1}
    Check(Near(c[0], 1.0) && Near(c[1], 1.0) && Near(c[2], 1.0), "复合赋值");

    // 比较
    Check(a == Vec3d{1, 2, 3}, "相等");
    Check(a != b, "不等");
    Check(a < b && b > a, "排序");

    // 混合类型运算 (回归: 不应有 SIMD 类型混读)
    Vec3f fa{1, 2, 3};
    Vec3d db{4, 5, 6};
    auto mx = fa + db;
    Check(mx[0] == 5.0 && mx[2] == 9.0, "混合类型 Vec3f+Vec3d");
    auto mxm = fa * db;
    Check(mxm[1] == 10.0, "混合类型 Vec3f*Vec3d");
}

// ============================ Vec: 算法 ============================
static void test_vec_math()
{
    Section("Vec: 计算函数");

    Vec3d a{3, 4, 0};
    Check(Near(Length(a), 5.0), "Length");
    Check(Near(LengthSquared(a), 25.0), "LengthSquared");

    auto n = Normalize(a);
    Check(Near(Length(n), 1.0) && Near(n[0], 0.6) && Near(n[1], 0.8), "Normalize");
    Check(Near(Length(Normalize(Vec3d{})), 0.0), "Normalize 零向量");

    Vec3d b{1, 2, 3}, c{4, 5, 6};
    Check(Near(Dot(b, c), 32.0), "Dot 2向量");
    Check(Near(Dot(b, c, Vec3d{1, 1, 1}), 32.0), "Dot 3向量");
    Check(Near(Dot(Vec3f{1, 2, 3}, Vec3d{4, 5, 6}), 32.0), "Dot 混合类型");

    auto cr3 = Cross(Vec3d{1, 0, 0}, Vec3d{0, 1, 0});
    Check(Near(cr3[2], 1.0), "Cross 3D");
    Check(Near(Cross(Vec2d{1, 0}, Vec2d{0, 1}), 1.0), "Cross 2D");

    Vec<double, 7> e1{1, 0, 0, 0, 0, 0, 0}, e2{0, 1, 0, 0, 0, 0, 0};
    auto cr7 = Cross(e1, e2); // 7D: e1 × e2 = e4
    Check(Near(cr7[3], 1.0), "Cross 7D");

    Vec3d x{1, 0, 0}, y{0, 1, 0};
    auto cx = x ^ y;
    Check(Near(cx[2], 1.0), "operator^");
    Vec3d xc{1, 0, 0};
    xc ^= y;
    Check(Near(xc[2], 1.0), "operator^=");

    auto h = Hadamard(Vec3d{1, 2, 3}, Vec3d{2, 3, 4}, Vec3d{3, 4, 5});
    Check(Near(h[0], 6.0) && Near(h[1], 24.0) && Near(h[2], 60.0), "Hadamard 3向量");

    auto cat = Cat(Vec2f{1, 2}, Vec3d{3, 4, 5});
    Check(cat.Size() == 5 && Near(cat[0], 1.0) && Near(cat[4], 5.0), "Cat 混合类型");

    Check(Near(Distance(Vec3d{0, 0, 0}, Vec3d{3, 4, 12}), 13.0), "Distance");
    Check(Near(DistanceSquared(Vec3d{0, 0, 0}, Vec3d{3, 4, 12}), 169.0), "DistanceSquared");

    auto lp = Lerp(Vec3d{0, 0, 0}, Vec3d{10, 10, 10}, 0.5);
    Check(Near(lp[0], 5.0) && Near(lp[2], 5.0), "Lerp");

    auto pr = Project(Vec3d{2, 2, 0}, Vec3d{4, 0, 0});
    Check(Near(pr[0], 2.0) && Near(pr[1], 0.0), "Project");

    auto rf = Reflect(Vec3d{1, -1, 0}, Vec3d{0, 1, 0});
    Check(Near(rf[0], 1.0) && Near(rf[1], 1.0), "Reflect");

    Check(Near(Radian(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}), std::numbers::pi / 2.0), "Radian");
    Check(Near(Degree(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}), 90.0), "Degree");
    Check(Near(Radian(Vec3d{1, 0, 0}, Vec3d{1, 0, 0}), 0.0), "Radian 同向量");
}

// ============================ Vec: 扩展函数 ============================
static void test_vec_ext()
{
    Section("Vec: 扩展函数");

    Vec3d v{-1, 2, -3};
    auto ab = Abs(v);
    Check(ab[0] == 1.0 && ab[2] == 3.0, "Abs");

    auto mn = Min(v, Vec3d{0, 1, -4});
    Check(mn[0] == -1.0 && mn[2] == -4.0, "Min");
    auto mx = Max(v, Vec3d{0, 1, -4});
    Check(mx[1] == 2.0 && mx[2] == -3.0, "Max");

    auto cl = Clamp(v, -2.0, 1.5);
    Check(cl[0] == -1.0 && cl[1] == 1.5 && cl[2] == -2.0, "Clamp 标量范围");
    auto clv = Clamp(v, Vec3d{-2, -2, -2}, Vec3d{1, 1, 1});
    Check(clv[1] == 1.0 && clv[2] == -2.0, "Clamp 向量范围");

    auto cm = ClampMagnitude(Vec3d{6, 8, 0}, 2.0);
    Check(Near(Length(cm), 2.0), "ClampMagnitude");

    Check(Near(Sum(v), -2.0), "Sum");
    Check(Near(Mean(v), -2.0 / 3.0), "Mean");

    Check(MaxComponent(v) == 2.0 && MinComponent(v) == -3.0, "Max/MinComponent");
    Check(MaxIndex(v) == 1 && MinIndex(v) == 2, "Max/MinIndex");

    auto nl = Nlerp(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}, 0.5);
    Check(Near(Length(nl), 1.0), "Nlerp 单位长度");

    auto sl = Slerp(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}, 0.5);
    Check(Near(Dot(sl, Vec3d{1, 0, 0}), std::cos(std::numbers::pi / 4.0), 1e-4), "Slerp 夹角=pi/4");
    auto sl2 = Slerp(Vec3d{1, 0, 0}, Vec3d{1, 0, 0}, 0.5);
    Check(Near(sl2[0], 1.0), "Slerp 近共线退化");

    auto il = InverseLerp(Vec3d{0, 0, 0}, Vec3d{10, 10, 10}, Vec3d{2, 4, 6});
    Check(Near(il[0], 0.2) && Near(il[1], 0.4) && Near(il[2], 0.6), "InverseLerp");

    auto ss = SmoothStep(Vec3d{0, 0, 0}, Vec3d{1, 1, 1}, 0.5);
    Check(Near(ss[0], 0.5), "SmoothStep 向量");

    auto op = OuterProduct(Vec3d{1, 2, 3}, Vec3d{4, 5, 6});
    Check(Near(op[1, 0], 8.0) && Near(op[2, 2], 18.0), "OuterProduct");

    Check(Near(TripleProduct(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}, Vec3d{0, 0, 1}), 1.0), "TripleProduct");

    auto pp = ProjectOnPlane(Vec3d{1, 2, 0}, Vec3d{0, 1, 0});
    Check(Near(pp[0], 1.0) && Near(pp[1], 0.0), "ProjectOnPlane");

    auto refr = Refract(Vec3d{1, -1, 0}, Vec3d{0, 1, 0}, 1.0); // eta=1 不弯折
    Check(Near(refr[0], 1.0) && Near(refr[1], -1.0), "Refract eta=1");
    auto tir = Refract(Vec3d{std::sqrt(3.0) / 2, -0.5, 0}, Vec3d{0, 1, 0}, 1.5); // 全反射
    Check(Near(Length(tir), 0.0), "Refract 全反射返回零向量");

    auto ff = Faceforward(Vec3d{0, 1, 0}, Vec3d{0, 1, 0}, Vec3d{0, 1, 0});
    Check(ff[1] == -1.0, "Faceforward 翻转");
    auto ff2 = Faceforward(Vec3d{0, 1, 0}, Vec3d{0, -1, 0}, Vec3d{0, 1, 0});
    Check(ff2[1] == 1.0, "Faceforward 保持");

    Check(IsParallel(Vec3d{1, 2, 3}, Vec3d{2, 4, 6}), "IsParallel 真");
    Check(!IsParallel(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}), "IsParallel 假");
    Check(IsOrthogonal(Vec3d{1, 0, 0}, Vec3d{0, 1, 0}), "IsOrthogonal 真");
    Check(!IsOrthogonal(Vec3d{1, 0, 0}, Vec3d{1, 1, 0}), "IsOrthogonal 假");

    auto mid = Midpoint(Vec3d{0, 0, 0}, Vec3d{2, 4, 6});
    Check(Near(mid[0], 1.0) && Near(mid[1], 2.0) && Near(mid[2], 3.0), "Midpoint");
}

// ============================ VecView ============================
static void test_vec_view()
{
    Section("VecView");

    Vec4f v{1, 2, 3, 4};
    auto view = v[Range<0, 4, 2>{}]; // {0,2} -> {1,3}
    Check(view.Size() == 2, "Size");
    Check(view[0] == 1.0f && view[1] == 3.0f, "operator[] 读");
    view[1] = 9.0f;
    Check(v[2] == 9.0f, "operator[] 写");

    const Vec4f &cv = v;
    auto cview = cv[Range<0, 4, 2>{}];
    Check(cview[1] == 9.0f, "const operator[]");

    Vec2f converted = view;
    Check(converted[0] == 1.0f && converted[1] == 9.0f, "转换回 Vec");

    view = {7, 8};
    Check(v[0] == 7.0f && v[2] == 8.0f, "赋值 initializer_list");

    view = Vec2f{5, 6};
    Check(v[0] == 5.0f && v[2] == 6.0f, "赋值 Vec");

    array<float, 2> ar = {3, 4};
    view = ar;
    Check(v[0] == 3.0f && v[2] == 4.0f, "赋值 array");

    vector<float> vec = {1, 2};
    view = vec;
    Check(v[0] == 1.0f && v[2] == 2.0f, "赋值 vector");

    list<float> lst = {8, 9};
    view = lst;
    Check(v[0] == 8.0f && v[2] == 9.0f, "赋值 list");
}

// ============================ Mat: 基础 ============================
static void test_mat_basic()
{
    Section("Mat: 构造 / 转换 / 访问");

    Mat2f m0;
    Mat2f m1(2.0f);
    Mat2f m2{1, 2, 3, 4};     // 扁平初始化
    Mat2f m3{{1, 2}, {3, 4}}; // 二维行初始化
    Check(m0[0] == 0.0f && m1[1] == 2.0f && m2[3] == 4.0f &&
          m3[0, 1] == 2.0f && m3[1, 0] == 3.0f, "构造");

    array<float, 4> ar = {1, 2, 3, 4};
    Mat2f m5(ar);
    vector<float> vec = {1, 2, 3, 4};
    Mat2f m6(vec);
    list<float> lst = {1, 2, 3, 4};
    Mat2f m7(lst);
    float arr[4] = {1, 2, 3, 4};
    span<const float, 4> sp(arr);
    Mat2f m8(sp);
    Check(m5[3] == 4.0f && m6[2] == 3.0f && m7[1] == 2.0f && m8[0] == 1.0f, "容器构造");

    array<float, 4> ca = m2;
    vector<float> cv = m2;
    list<float> cl = m2;
    span<const float, 4> cs = m2;
    Check(ca[1] == 2.0f && cv[2] == 3.0f && *cl.begin() == 1.0f && cs[0] == 1.0f, "转换到容器");

    Mat2d md(m2);
    Check(md[0, 0] == 1.0 && md[1, 1] == 4.0, "类型转换构造");

    Mat2f cp(m2);
    Mat2f mv(std::move(m2));
    Check(cp[0] == 1.0f && mv[1] == 2.0f, "拷贝/移动构造");

    Mat2f as{1, 2, 3, 4};
    as = 3.0f;
    Check(as[0] == 3.0f && as[3] == 3.0f, "标量赋值");

    // 实例 Set 方法
    Mat3f sm = Mat3f::MakeIdentity();
    sm.SetValue(0.0f);
    Check(sm[0] == 0.0f && sm[8] == 0.0f, "Mat SetValue");
    sm.SetIdentity();
    Check(IsIdentity(sm), "Mat SetIdentity");
    Mat3f sr{1, 2, 3, 4, 5, 6, 7, 8, 9};
    sr.SetRow(0, Vec3f{9, 8, 7});
    Check(Near(sr[0, 0], 9.0f) && Near(sr[0, 2], 7.0f) && Near(sr[1, 0], 4.0f), "Mat SetRow");
    sr.SetCol(1, Vec3f{1, 2, 3});
    Check(Near(sr[0, 1], 1.0f) && Near(sr[1, 1], 2.0f) && Near(sr[2, 1], 3.0f), "Mat SetCol");

    auto id = Mat3f::MakeIdentity();
    Check(Near(id[0, 0], 1.0) && Near(id[1, 1], 1.0) && Near(id[0, 1], 0.0), "MakeIdentity");

    Mat2f gm{1, 2, 3, 4};
    auto row0 = gm.GetRow(0); // Mat<1,2>{1,2}
    auto col1 = gm.GetCol(1); // Mat<2,1>{2,4}
    Check(Near(row0[0, 0], 1.0) && Near(row0[0, 1], 2.0), "GetRow");
    Check(Near(col1[0, 0], 2.0) && Near(col1[1, 0], 4.0), "GetCol");

    Check(Mat2f::Size() == 4 && Mat2f::RowSize() == 2 && Mat2f::ColSize() == 2, "静态查询");
    Check(std::get<0>(Mat2f::Shape()) == 2 && std::get<1>(Mat2f::Shape()) == 2, "Shape");
}

// ============================ Mat: 运算 ============================
static void test_mat_operators()
{
    Section("Mat: 运算符");

    Mat2d a{1, 2, 3, 4}, b{5, 6, 7, 8};
    auto s = a + b; // {6,8,10,12}
    auto d = b - a; // {4,4,4,4}
    auto m = a * b; // {19,22,43,50}
    Check(Near(s[0], 6.0) && Near(d[3], 4.0) && Near(m[0], 19.0) && Near(m[3], 50.0), "Mat + - *");

    auto su = a + 1.0;
    auto ud = 1.0 + a;
    auto su2 = a - 1.0;
    auto rs = 5.0 - a;
    auto mu = a * 2.0;
    auto um = 2.0 * a;
    Check(Near(su[0], 2.0) && Near(ud[1], 3.0) && Near(su2[0], 0.0) && Near(rs[0], 4.0) &&
          Near(mu[2], 6.0) && Near(um[3], 8.0), "Mat 标量运算");

    auto neg = -a;
    Check(Near(neg[0], -1.0), "Mat 一元负号");

    // Mat*Vec / Vec*Mat
    Mat2d mm{1, 2, 3, 4};
    Vec2d vv{1, 2};
    auto mv = mm * vv; // (5,11)
    auto vm = vv * mm; // (7,10)
    Check(Near(mv[0], 5.0) && Near(mv[1], 11.0), "Mat*Vec");
    Check(Near(vm[0], 7.0) && Near(vm[1], 10.0), "Vec*Mat");

    // Vec *= Mat
    Vec3d v3{1, 0, 0};
    Mat3d sw{0, 1, 0, 1, 0, 0, 0, 0, 1};
    v3 *= sw;
    Check(Near(v3[0], 0.0) && Near(v3[1], 1.0), "Vec *= Mat");

    // 复合赋值
    Mat2d c{1, 2, 3, 4};
    c += Mat2d{1, 1, 1, 1}; // {2,3,4,5}
    c -= 1.0;               // {1,2,3,4}
    c *= Mat2d{5, 6, 7, 8}; // {19,22,43,50}
    c *= 2.0;               // {38,44,86,100}
    Check(Near(c[0], 38.0) && Near(c[3], 100.0), "复合赋值");

    // 比较 / 迭代器
    Check(a == Mat2d{1, 2, 3, 4}, "相等");
    Check(a != b, "不等");
    double sum = 0;
    for (double x : a) sum += x;
    Check(Near(sum, 10.0), "迭代器");

    // 混合类型 (回归: common_type 累积, 无精度截断)
    auto mix = Mat3f{1, 1, 1, 1, 1, 1, 1, 1, 1} + Mat3d::MakeIdentity();
    Check(Near(mix[0, 0], 2.0), "Mat 混合类型 +");
    Mat3d ma = Mat3d::MakeIdentity();
    Mat3f mb{2, 0, 0, 0, 1, 0, 0, 0, 1};
    ma *= mb;
    Check(Near(ma[0, 0], 2.0) && Near(ma[1, 1], 1.0), "Mat3d *= Mat3f 精度");
    Vec3d vd{1, 0, 0};
    vd *= mb; // Vec3d *= Mat3f
    Check(Near(vd[0], 2.0), "Vec3d *= Mat3f 精度");
}

// ============================ Mat: 算法 ============================
static void test_mat_math()
{
    Section("Mat: 计算函数");

    Mat2d a{1, 2, 3, 4};
    auto t = Transpose(a); // {1,3,2,4}
    Check(Near(t[0, 1], 3.0) && Near(t[1, 0], 2.0), "Transpose");

    Mat<double, 2, 3> ns{1, 2, 3, 4, 5, 6};
    auto nt = Transpose(ns); // 3x2 {1,4,2,5,3,6}
    Check(nt.RowSize() == 3 && nt.ColSize() == 2 && Near(nt[2, 0], 3.0) && Near(nt[0, 1], 4.0), "Transpose 非方阵");

    Check(Near(Det(Mat<double, 1, 1>{5}), 5.0), "Det 1x1");
    Check(Near(Det(a), -2.0), "Det 2x2");
    Mat3d m3{1, 2, 3, 4, 5, 6, 7, 8, 10};
    Check(Near(Det(m3), -3.0), "Det 3x3");
    Check(Near(Det(Mat2d{1, 2, 2, 4}), 0.0), "Det 奇异矩阵");

    Check(Near(Cofactor(a, 0, 0), 4.0) && Near(Cofactor(a, 0, 1), -3.0), "Cofactor");
    auto adj = Adjoint(a); // {4,-2,-3,1}
    Check(Near(adj[0, 0], 4.0) && Near(adj[0, 1], -2.0) && Near(adj[1, 0], -3.0) && Near(adj[1, 1], 1.0), "Adjoint");

    Check(Near(Trace(a), 5.0), "Trace");
    Check(Rank(a) == 2 && IsFullRank(a), "Rank/IsFullRank 满秩");
    Check(Rank(Mat2d{1, 2, 2, 4}) == 1 && !IsFullRank(Mat2d{1, 2, 2, 4}), "Rank/IsFullRank 奇异");

    auto inv = Inverse(a); // {-2,1,1.5,-0.5}
    Check(Near(inv[0, 0], -2.0) && Near(inv[1, 1], -0.5), "Inverse 2x2 值");
    auto id2 = a * inv;
    Check(Near(id2[0, 0], 1.0, 1e-6) && Near(id2[1, 1], 1.0, 1e-6) && Near(id2[0, 1], 0.0, 1e-6), "Inverse A*A^-1=I");

    auto inv3 = Inverse(m3);
    auto id3 = m3 * inv3;
    Check(Near(id3[0, 0], 1.0, 1e-6) && Near(id3[2, 2], 1.0, 1e-6) &&
          Near(id3[1, 0], 0.0, 1e-6) && Near(id3[2, 1], 0.0, 1e-6), "Inverse 3x3 高斯-约当");

    auto hm = Hadamard(Mat2d{1, 2, 3, 4}, Mat2d{2, 3, 4, 5}, Mat2d{3, 4, 5, 6});
    Check(Near(hm[0], 6.0) && Near(hm[3], 120.0), "Hadamard 3矩阵");

    auto kr = KroneckerProduct(Mat2d{1, 0, 0, 1}, Mat2d{5, 6, 7, 8});
    Check(kr.RowSize() == 4 && kr.ColSize() == 4 && Near(kr[0, 0], 5.0) && Near(kr[3, 3], 8.0) &&
          Near(kr[0, 2], 0.0) && Near(kr[2, 0], 0.0), "KroneckerProduct");

    auto k3 = Kronecker(Mat2d{1, 2, 3, 4}, Mat2d{0, 1, 1, 0}, Mat2d{1, 0, 0, 1});
    Check(k3.RowSize() == 8 && k3.ColSize() == 8, "Kronecker 多参数");
}

// ============================ Mat: 扩展函数 ============================
static void test_mat_ext()
{
    Section("Mat: 扩展函数");

    Check(IsIdentity(Mat3d::MakeIdentity()), "MakeIdentity/IsIdentity");

    auto diag = MakeDiagonal(Vec3d{1, 2, 3});
    Check(IsDiagonal(diag) && Near(diag[1, 1], 2.0), "MakeDiagonal/IsDiagonal");
    auto dv = Diagonal(diag);
    Check(Near(dv[2], 3.0), "Diagonal");

    auto r2 = Mat2d::MakeRotation(std::numbers::pi / 2.0);
    Check(Near(r2[0, 1], -1.0) && Near(r2[1, 0], 1.0, 1e-6), "MakeRotation 2D");

    auto rx = Mat3d::MakeRotationX(std::numbers::pi / 2.0);
    Check(Near(rx[1, 2], -1.0, 1e-6) && Near(rx[2, 1], 1.0, 1e-6), "MakeRotationX");
    auto ry = Mat3d::MakeRotationY(std::numbers::pi / 2.0);
    Check(Near(ry[0, 2], 1.0, 1e-6) && Near(ry[2, 0], -1.0, 1e-6), "MakeRotationY");
    auto rz = Mat3d::MakeRotationZ(std::numbers::pi / 2.0);
    Check(Near(rz[0, 1], -1.0, 1e-6) && Near(rz[1, 0], 1.0, 1e-6), "MakeRotationZ");

    auto sc = Mat3d::MakeScale(Vec3d{2, 3, 4});
    Check(IsDiagonal(sc) && Near(sc[2, 2], 4.0), "MakeScale");
    auto tr = Mat4d::MakeTranslation(Vec3d{1, 2, 3});
    Check(Near(tr[3, 0], 1.0) && Near(tr[3, 2], 3.0), "MakeTranslation");

    Check(IsOrthogonal(Mat3d::MakeRotationX(0.3)), "IsOrthogonal 旋转矩阵");
    Check(IsSymmetric(Mat3d{1, 2, 3, 2, 4, 5, 3, 5, 6}), "IsSymmetric");
    Check(IsSkewSymmetric(Mat3d{0, 1, 2, -1, 0, 3, -2, -3, 0}), "IsSkewSymmetric");
    Check(IsSingular(Mat2d{1, 2, 2, 4}), "IsSingular");

    Check(Near(FrobeniusNorm(Mat2d{3, 4, 0, 0}), 5.0), "FrobeniusNorm");

    auto x = SolveLinearSystem(Mat2d{2, 1, 1, 3}, Vec2d{5, 10});
    Check(Near(x[0], 1.0) && Near(x[1], 3.0), "SolveLinearSystem");

    auto mp = MatrixPower(Mat2d{2, 0, 0, 2}, 3);
    Check(Near(mp[0, 0], 8.0) && Near(mp[3], 8.0), "MatrixPower");

    auto pinv = PseudoInverse(Mat2d{2, 0, 0, 2});
    Check(Near(pinv[0, 0], 0.5) && Near(pinv[3], 0.5), "PseudoInverse");

    // 正交矩阵: 转置 == 逆
    auto rot = Mat3d::MakeRotationX(0.3);
    auto rotT = Transpose(rot);
    auto rotI = Inverse(rot);
    Check(Near(rotT[0, 0], rotI[0, 0], 1e-6) && Near(rotT[1, 2], rotI[1, 2], 1e-6), "旋转矩阵 转置==逆");
}

// ============================ MatView ============================
static void test_mat_view()
{
    Section("MatView");

    Mat4f m = Mat4f::MakeIdentity();
    auto view = m[Range<0, 2>{}, Range<0, 2>{}];
    view = {5, 6, 7, 8};
    Check(Near(m[0, 0], 5.0) && Near(m[0, 1], 6.0) && Near(m[1, 0], 7.0) && Near(m[1, 1], 8.0), "赋值 initializer_list");

    Mat2f sub{1, 2, 3, 4};
    view = sub;
    Check(Near(m[0, 0], 1.0) && Near(m[1, 1], 4.0), "赋值 Mat");

    Mat2f extracted = view;
    Check(Near(extracted[0, 1], 2.0) && Near(extracted[1, 0], 3.0), "转换回 Mat");

    Mat4f m2 = Mat4f::MakeIdentity();
    auto v2 = m2[Range<0, 4, 2>{}, Range<0, 4, 2>{}];
    v2 = {1, 2, 3, 4};
    Check(Near(m2[0], 1.0) && Near(m2[10], 4.0), "跨步视图赋值");

    vector<float> vvec = {9, 10, 11, 12};
    v2 = vvec;
    Check(Near(m2[0], 9.0f) && Near(m2[10], 12.0f), "MatView 赋值 vector");
    list<float> llist = {1, 2, 3, 4};
    v2 = llist;
    Check(Near(m2[0], 1.0f) && Near(m2[10], 4.0f), "MatView 赋值 list");
}

// ============================ Camera ============================
static void test_camera()
{
    Section("Camera: 视图 / 投影");

    auto view = Mat4f::MakeView(Vec3f{0, 0, 5}, Vec3f{0, 0, -1}, Vec3f{0, 1, 0});
    Check(Near(view[3, 2], -5.0), "MakeView 3D 平移");
    Check(Near(view[0, 0], 1.0) && Near(view[1, 1], 1.0), "MakeView 3D 旋转");

    auto v2 = Mat4f::MakeView(Vec2f{0, 0}, 0.0f, 1.0f);
    Check(Near(v2[0, 0], 1.0) && Near(v2[1, 1], 1.0), "MakeView 2D 单位");
    auto v3 = Mat4f::MakeView(Vec2f{1, 0}, 0.0f, 1.0f);
    Check(Near(v3[0, 3], -1.0), "MakeView 2D 平移");
    auto v4 = Mat4f::MakeView(Vec2f{0, 0}, std::numbers::pi_v<float> / 2.0f, 1.0f);
    Check(Near(v4[0, 1], 1.0, 1e-4), "MakeView 2D 旋转");

    auto proj = Mat4f::MakeProjection(std::numbers::pi_v<float> / 2.0f, 1.0f, 0.1f, 100.0f);
    Check(Near(proj[0, 0], 1.0) && proj[3, 2] == -1.0f, "MakeProjection 透视");

    auto ortho = Mat4f::MakeProjection(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    Check(Near(ortho[0, 0], 1.0) && Near(ortho[1, 1], 1.0) && Near(ortho[2, 2], -1.0) && Near(ortho[3, 3], 1.0),
          "MakeProjection 正交 6参");

    auto ortho2 = Mat4f::MakeProjection(800.0f, 600.0f);
    Check(Near(ortho2[0, 0], 2.0f / 800.0f) && Near(std::abs(ortho2[1, 1]), 2.0f / 600.0f), "MakeProjection 正交 2参");

    auto lk = Mat4d::MakeLookAt(Vec3d{0, 0, 5}, Vec3d{0, 0, 0}, Vec3d{0, 1, 0});
    Check(Near(lk[3, 2], -5.0), "MakeLookAt");
}

// ============================ 标量工具 ============================
static void test_scalar()
{
    Section("标量辅助函数");

    Check(Near(Clamp(1.5, 0.0, 1.0), 1.0), "Clamp");
    Check(Near(Lerp(0.0, 10.0, 0.25), 2.5), "Lerp");
    Check(Near(SmoothStep(0.0, 1.0, 0.5), 0.5), "SmoothStep");
    Check(Step(5, 3) == 0 && Step(5, 7) == 1, "Step");
    Check(Near(Remap(5, 0, 10, 0, 100), 50.0), "Remap");
    Check(Near(DegToRad(180.0), std::numbers::pi), "DegToRad");
    Check(Near(RadToDeg(std::numbers::pi), 180.0), "RadToDeg");
    Check(ApproxEqual(1.0, 1.0 + 1e-8), "ApproxEqual 真");
    Check(!ApproxEqual(1.0, 2.0), "ApproxEqual 假");
}

// ============================ SIMD 回归 ============================
static void test_simd_regression()
{
    Section("SIMD 回归");

    // long 类型可编译且走标量路径
    Vec2l la{1, 2}, lb{3, 4};
    auto lc = la + lb;
    Check(lc[0] == 4 && lc[1] == 6, "Vec2l 加法");
    Check(Dot(la, lb) == 11, "Vec2l 点积");
    Mat2l lm{1, 2, 3, 4};
    Check(Det(lm) == -2, "Mat2l 行列式");

    // SIMD 路径基本正确性
    Vec4d v4{1, 2, 3, 4};
    Check(Near(Length(v4), std::sqrt(30.0)), "Vec4d Length (SIMD)");
    Vec4f f4{1, 1, 1, 1};
    Check(Near(Dot(f4, f4), 4.0), "Vec4f Dot (SIMD)");
    Vec4f g4{2, 2, 2, 2};
    auto gs = f4 + g4;
    Check(Near(gs[0], 3.0) && Near(gs[3], 3.0), "Vec4f 加法 (SIMD)");
    Vec<float, 8> h8{1, 2, 3, 4, 5, 6, 7, 8};
    Check(Near(Sum(h8), 36.0), "Vec8f Sum (AVX)");
}

// ============================ 异常路径 ============================
static void test_exceptions()
{
    Section("异常路径");

    bool caught = false;
    try
    {
        Vec3f bad{1, 2};
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    Check(caught, "Vec 初始化列表长度不符抛异常");

    caught = false;
    try
    {
        Mat2f bad{1, 2, 3};
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    Check(caught, "Mat 初始化列表长度不符抛异常");

    caught = false;
    try
    {
        Inverse(Mat2d{1, 2, 2, 4});
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    Check(caught, "Inverse 奇异矩阵抛异常");

    caught = false;
    try
    {
        SolveLinearSystem(Mat2d{1, 2, 2, 4}, Vec2d{1, 1});
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    Check(caught, "SolveLinearSystem 奇异抛异常");

    caught = false;
    try
    {
        MatrixPower(Mat2d::MakeIdentity(), -1);
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    Check(caught, "MatrixPower 负指数抛异常");
}

// ============================ 输出运算符 ============================
static void test_ostream()
{
    Section("输出运算符");

    Vec3f v{1, 2, 3};
    Mat2f m{1, 2, 3, 4};
    Mat3f rot = Mat3f::MakeRotationX(0.5f);
    auto vv = v[Range<0, 3, 1>{}];
    auto mv = m[Range<0, 2>{}, Range<0, 2>{}];

    cout << "  Vec:      " << v << "\n";
    cout << "  VecView:  " << vv << "\n";
    cout << "  Mat:\n" << m << "\n";
    cout << "  MatView:\n" << mv << "\n";
    cout << "  Rotation:\n" << rot << "\n";
    Check(true, "ostream 输出无崩溃");
}

// ============================ main ============================
int main()
{
    test_range();
    test_vec_basic();
    test_vec_operators();
    test_vec_math();
    test_vec_ext();
    test_vec_view();
    test_mat_basic();
    test_mat_operators();
    test_mat_math();
    test_mat_ext();
    test_mat_view();
    test_camera();
    test_scalar();
    test_simd_regression();
    test_exceptions();
    test_ostream();

    cout << "\n========================================\n";
    cout << "Total: " << g_total << ", Failed: " << g_failed << "\n";
    cout << (g_failed == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return g_failed;
}
