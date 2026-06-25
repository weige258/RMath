#include "rmath.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <span>
#include <cmath>
#include <numbers>

using namespace std;

static bool Near(double a, double b, double eps = 1e-5)
{
    return std::fabs(a - b) <= eps;
}

int main()
{
    int total = 0;
    int failed = 0;

    auto check = [&](bool ok, const char *name)
    {
        ++total;
        if (!ok)
        {
            ++failed;
            cout << "[FAIL] " << name << "\n";
        }
        else
        {
            cout << "[PASS] " << name << "\n";
        }
    };

    // ==================== Range tests ====================
    {
        Range<0, 5, 2> r;
        check(r.Size() == 3, "Range Size");
        check(r[0] == 0 && r[1] == 2 && r[2] == 4, "Range operator[]");

        vector<int> rv = r;
        list<int> rl = r;
        array<int, Range<0, 5, 2>::Size()> ra = r;
        check(rv.size() == 3 && rv[0] == 0 && rv[2] == 4, "Range to vector");
        check(rl.size() == 3, "Range to list");
        check(ra[1] == 2, "Range to array");

        Range<5, 0, -1> rn;
        check(rn.Size() == 5, "Range negative step Size");
        check(rn[0] == 5 && rn[4] == 1, "Range negative step operator[]");

        Range<5, 0, -2> rn2;
        check(rn2.Size() == 3, "Range negative step Size skip");
        check(rn2[0] == 5 && rn2[1] == 3 && rn2[2] == 1, "Range negative step skip values");

        Range<0, 0, 1> re;
        check(re.Size() == 0, "Range empty Size");

        Range<3, 3, -1> ren;
        check(ren.Size() == 0, "Range negative step empty Size");

        auto vals = Range<0, 6, 2>::Values();
        check(vals[0] == 0 && vals[1] == 2 && vals[2] == 4, "Range Values()");

        Range<1, 4, 1> rit;
        int sum = 0;
        for (auto v : rit) sum += v;
        check(sum == 6, "Range iterator sum");
    }

    // ==================== Vec constructors and conversions ====================
    {
        Vec4f v0;
        Vec4f v1(2.0f);
        Vec4f v2{1.0f, 2.0f, 3.0f, 4.0f};
        Vec4f v3(1.0f, 2.0f, 3.0f, 4.0f);

        float arr[4] = {1, 2, 3, 4};
        Vec4f v4(arr);
        array<float, 4> a = {5, 6, 7, 8};
        Vec4f v5(a);

        vector<float> vec = {1, 2, 3, 4};
        list<float> lst = {1, 2, 3, 4};
        span<const float, 4> sp(arr);
        Vec4f v6(vec);
        Vec4f v7(lst);
        Vec4f v8(sp);

        array<float, 4> out_arr = v2;
        vector<float> out_vec = v2;
        list<float> out_list = v2;
        span<const float, 4> out_span = v2;

        check(v0[0] == 0.0f && v1[1] == 2.0f, "Vec basic constructors");
        check(v2[2] == 3.0f && v3[3] == 4.0f, "Vec init/variadic constructors");
        check(v4[0] == 1.0f && v5[3] == 8.0f, "Vec array/ptr constructors");
        check(v6[1] == 2.0f && v7[2] == 3.0f && v8[3] == 4.0f, "Vec list/vector/span constructors");
        check(out_arr[0] == 1.0f && out_vec[1] == 2.0f && *out_list.begin() == 1.0f, "Vec conversions");
        check(out_span[2] == 3.0f, "Vec span conversion");
    }

    // ==================== Vec type conversion constructor ====================
    {
        Vec3i vi{1, 2, 3};
        Vec3f vf(vi);
        check(Near(vf[0], 1.0) && Near(vf[1], 2.0) && Near(vf[2], 3.0), "Vec int to float conversion");

        Vec3d vd(vf);
        check(Near(vd[0], 1.0) && Near(vd[2], 3.0), "Vec float to double conversion");
    }

    // ==================== Vec copy/move ====================
    {
        Vec3f a{1, 2, 3};
        Vec3f b(a);
        check(b[0] == 1.0f && b[2] == 3.0f, "Vec copy constructor");

        Vec3f c(std::move(a));
        check(c[1] == 2.0f, "Vec move constructor");

        Vec3f d;
        d = b;
        check(d[0] == 1.0f, "Vec copy assignment");

        Vec3f e;
        e = std::move(c);
        check(e[2] == 3.0f, "Vec move assignment");
    }

    // ==================== Vec accessors X/Y/Z/W ====================
    {
        Vec4f v{1, 2, 3, 4};
        check(v.X() == 1.0f, "Vec X()");
        check(v.Y() == 2.0f, "Vec Y()");
        check(v.Z() == 3.0f, "Vec Z()");
        check(v.W() == 4.0f, "Vec W()");

        v.X() = 10.0f;
        check(v[0] == 10.0f, "Vec X() write");

        Vec3f v3{5, 6, 7};
        auto xyz = v3.XYZ();
        check(xyz.Size() == 3, "Vec XYZ() size");

        Vec4f color{0.1f, 0.2f, 0.3f, 1.0f};
        auto rgb = color.RGB();
        check(rgb.Size() == 3, "Vec RGB() size");
    }

    // ==================== Vec static query methods ====================
    {
        check(Vec3f::Size() == 3, "Vec Size()");
        check(Vec4f::SizeInBytes() == 4 * sizeof(float), "Vec SizeInBytes()");
    }

    // ==================== Vec operations ====================
    {
        Vec3f a{1.0f, 2.0f, 3.0f};
        Vec3f b{2.0f, 3.0f, 4.0f};

        auto add = a + b;
        auto sub = b - a;
        auto mul = a * b;
        auto div = b / a;
        auto add_s = a + 2.0f;
        auto s_add = 2.0f + a;
        auto neg = -a;

        a += b;
        b -= Vec3f{1.0f, 1.0f, 1.0f};

        auto dot = Dot(a, b);
        auto cross = Cross(Vec3f{1, 0, 0}, Vec3f{0, 1, 0});
        auto had = Hadamard(Vec3f{1, 2, 3}, Vec3f{2, 3, 4});
        auto cat = Cat(Vec2f{1, 2}, Vec3f{3, 4, 5});
        auto dist = Distance(Vec3f{0, 0, 0}, Vec3f{3, 4, 0});
        auto lerp = Lerp(Vec3f{0, 0, 0}, Vec3f{10, 0, 0}, 0.5);
        auto proj = Project(Vec3f{2, 0, 0}, Vec3f{1, 0, 0});
        auto refl = Reflect(Vec3f{1, -1, 0}, Vec3f{0, 1, 0});
        auto rad = Radian(Vec2f{1, 0}, Vec2f{0, 1});
        auto deg = Degree(Vec2f{1, 0}, Vec2f{0, 1});

        check(add[0] == 3.0f && sub[1] == 1.0f, "Vec add/sub");
        check(mul[2] == 12.0f && Near(div[0], 2.0), "Vec mul/div");
        check(add_s[0] == 3.0f && s_add[1] == 4.0f, "Vec scalar add");
        check(neg[2] == -3.0f, "Vec unary -");
        check(Near(dot, 34.0), "Dot");
        check(Near(cross[2], 1.0), "Cross");
        check(Near(had[1], 6.0), "Hadamard");
        check(cat.Size() == 5 && Near(cat[4], 5.0), "Cat");
        check(Near(dist, 5.0), "Distance");
        check(Near(lerp[0], 5.0), "Lerp");
        check(Near(proj[0], 2.0) && Near(proj[1], 0.0), "Project");
        check(Near(refl[1], 1.0), "Reflect");
        check(Near(rad, std::numbers::pi / 2.0), "Radian");
        check(Near(deg, 90.0), "Degree");
    }

    // ==================== Vec scalar sub/mul/div ====================
    {
        Vec3f a{5.0f, 5.0f, 5.0f};
        auto sub_s = a - 2.0f;
        auto s_sub = 10.0f - a;
        auto mul_s = a * 2.0f;
        auto s_mul = 3.0f * a;
        auto div_s = a / 5.0f;
        auto s_div = 10.0f / a;

        check(Near(sub_s[0], 3.0), "Vec scalar sub");
        check(Near(s_sub[0], 5.0), "Vec scalar reverse sub");
        check(Near(mul_s[1], 10.0), "Vec scalar mul");
        check(Near(s_mul[2], 15.0), "Vec scalar reverse mul");
        check(Near(div_s[0], 1.0), "Vec scalar div");
        check(Near(s_div[0], 2.0), "Vec scalar reverse div");
    }

    // ==================== Vec compound assignment ====================
    {
        Vec3f a{1, 2, 3};
        a += 10.0f;
        check(Near(a[0], 11.0) && Near(a[2], 13.0), "Vec += scalar");

        a -= 5.0f;
        check(Near(a[0], 6.0), "Vec -= scalar");

        Vec3f b{2, 2, 2};
        a *= b;
        check(Near(a[0], 12.0) && Near(a[2], 16.0), "Vec *= Vec");

        a *= 2.0f;
        check(Near(a[0], 24.0), "Vec *= scalar");

        Vec3f c{2, 2, 2};
        a /= c;
        check(Near(a[0], 12.0), "Vec /= Vec");

        a /= 3.0f;
        check(Near(a[0], 4.0), "Vec /= scalar");
    }

    // ==================== Vec unary minus on const ====================
    {
        const Vec3f a{1, 2, 3};
        auto neg = -a;
        check(Near(neg[0], -1.0) && Near(neg[2], -3.0), "Vec unary - on const");
    }

    // ==================== Vec comparison operators ====================
    {
        Vec3f a{1, 2, 3};
        Vec3f b{1, 2, 3};
        Vec3f c{1, 2, 4};

        check(a == b, "Vec equality");
        check(a != c, "Vec inequality");
        check(a < c, "Vec less than");
        check(c > a, "Vec greater than");
    }

    // ==================== Vec iterator ====================
    {
        Vec3f a{1, 2, 3};
        float sum = 0;
        for (auto val : a) sum += val;
        check(Near(sum, 6.0), "Vec iterator");
    }

    // ==================== Vec pointer conversion ====================
    {
        Vec3f a{1, 2, 3};
        float *p = static_cast<float *>(a);
        check(Near(p[0], 1.0) && Near(p[2], 3.0), "Vec pointer conversion");

        const Vec3f b{4, 5, 6};
        const float *cp = static_cast<const float *>(b);
        check(Near(cp[1], 5.0), "Vec const pointer conversion");
    }

    // ==================== Vec assign scalar ====================
    {
        Vec3f a{1, 2, 3};
        a = 5.0f;
        check(Near(a[0], 5.0) && Near(a[1], 5.0) && Near(a[2], 5.0), "Vec assign scalar");
    }

    // ==================== Vec ^ cross operator ====================
    {
        Vec3f a{1, 0, 0};
        Vec3f b{0, 1, 0};
        auto c = a ^ b;
        check(Near(c[0], 0.0) && Near(c[1], 0.0) && Near(c[2], 1.0), "Vec ^ cross operator");

        Vec3f d{1, 0, 0};
        d ^= Vec3f{0, 1, 0};
        check(Near(d[2], 1.0), "Vec ^= cross operator");
    }

    // ==================== Vec 2D cross ====================
    {
        Vec2f a{1, 0};
        Vec2f b{0, 1};
        auto c = Cross(a, b);
        check(Near(c, 1.0), "Cross 2D");
    }

    // ==================== Length / Normalize ====================
    {
        Vec3f a{3, 4, 0};
        check(Near(Length(a), 5.0), "Length");

        Vec3f n = Normalize(a);
        check(Near(Length(n), 1.0), "Normalize length");
        check(Near(n[0], 0.6) && Near(n[1], 0.8), "Normalize values");

        Vec3f zero{0, 0, 0};
        auto nz = Normalize(zero);
        check(Near(nz[0], 0.0) && Near(nz[1], 0.0) && Near(nz[2], 0.0), "Normalize zero vector");
    }

    // ==================== Dot multi-vector ====================
    {
        Vec3f a{1, 0, 0};
        Vec3f b{0, 1, 0};
        Vec3f c{0, 0, 1};
        auto d = Dot(a, b, c);
        check(Near(d, 0.0), "Dot 3 vectors");

        Vec3f x{1, 1, 1};
        auto d2 = Dot(x, x, x);
        check(Near(d2, 3.0), "Dot 3 identical vectors");
    }

    // ==================== Hadamard multi-vector ====================
    {
        Vec3f a{1, 2, 3};
        Vec3f b{2, 3, 4};
        Vec3f c{3, 4, 5};
        auto h = Hadamard(a, b, c);
        check(Near(h[0], 6.0) && Near(h[1], 24.0) && Near(h[2], 60.0), "Hadamard 3 vectors");
    }

    // ==================== Cat multi-vector ====================
    {
        Vec2f a{1, 2};
        Vec2f b{3, 4};
        Vec2f c{5, 6};
        auto cat = Cat(a, b, c);
        check(cat.Size() == 6, "Cat 3 vectors size");
        check(Near(cat[0], 1.0) && Near(cat[5], 6.0), "Cat 3 vectors values");
    }

    // ==================== Lerp edge cases ====================
    {
        Vec3f a{0, 0, 0};
        Vec3f b{10, 10, 10};
        auto l0 = Lerp(a, b, 0.0);
        auto l1 = Lerp(a, b, 1.0);
        check(Near(l0[0], 0.0) && Near(l0[2], 0.0), "Lerp t=0");
        check(Near(l1[0], 10.0) && Near(l1[2], 10.0), "Lerp t=1");
    }

    // ==================== Radian/Degree edge cases ====================
    {
        Vec2f a{1, 0};
        auto rad0 = Radian(a, a);
        check(Near(rad0, 0.0), "Radian same vector");

        auto deg0 = Degree(a, a);
        check(Near(deg0, 0.0), "Degree same vector");

        Vec2f b{-1, 0};
        auto rad180 = Radian(a, b);
        check(Near(rad180, std::numbers::pi), "Radian opposite");

        auto deg180 = Degree(a, b);
        check(Near(deg180, 180.0), "Degree opposite");

        Vec2f z{0, 0};
        auto rad_zero = Radian(z, z);
        check(Near(rad_zero, 0.0), "Radian zero vector");
    }

    // ==================== VecView tests ====================
    {
        Vec4f v{1, 2, 3, 4};
        auto view = v[Range<0, 4, 2>{}];
        Vec2f vv = view;
        check(Near(vv[0], 1.0) && Near(vv[1], 3.0), "VecView conversion");

        view = Vec2f{9, 8};
        check(Near(v[0], 9.0) && Near(v[2], 8.0), "VecView assign Vec");

        view = {7, 6};
        check(Near(v[0], 7.0) && Near(v[2], 6.0), "VecView assign init_list");

        array<float, 2> arr = {5, 4};
        view = arr;
        check(Near(v[0], 5.0) && Near(v[2], 4.0), "VecView assign array");

        vector<float> vec = {3, 2};
        list<float> lst = {1, 0};
        view = vec;
        check(Near(v[0], 3.0) && Near(v[2], 2.0), "VecView assign vector");
        view = lst;
        check(Near(v[0], 1.0) && Near(v[2], 0.0), "VecView assign list");

        span<float, 2> sp(arr);
        view = sp;
        check(Near(v[0], 5.0) && Near(v[2], 4.0), "VecView assign span");

        check(view.Size() == 2, "VecView Size()");
    }

    // ==================== Mat constructors, conversions, and operations ====================
    {
        Mat2f m0;
        Mat2f m1(2.0f);
        Mat2f m2{1, 2, 3, 4};
        Mat2f m3{{1, 2}, {3, 4}};

        vector<float> vec = {1, 2, 3, 4};
        list<float> lst = {1, 2, 3, 4};
        array<float, 4> arr = {1, 2, 3, 4};
        span<const float, 4> sp(arr);
        Mat2f m4(vec);
        Mat2f m5(lst);
        Mat2f m6(sp);

        array<float, 4> out_arr = m2;
        vector<float> out_vec = m2;
        list<float> out_list = m2;
        span<float, 4> out_span = m2;

        auto add = m2 + m1;
        auto sub = m2 - m1;
        auto mul = m2 * m2;
        auto add_s = m2 + 1.0f;
        auto s_add = 1.0f + m2;
        auto neg = -m2;

        Vec2f v{1, 1};
        auto mv = m2 * v;
        auto vm = v * m2;

        auto mt = Transpose(m2);
        auto det = Det(m2);
        auto tr = Trace(m2);
        auto rk = Rank(m2);
        auto full = IsFullRank(m2);
        auto had = Hadamard(m2, m1);
        auto kron = Kronecker(m2, m2);
        auto inv = Inverse(m2);

        check(m0[0] == 0.0f && m1[1] == 2.0f, "Mat basic constructors");
        check(m2[0] == 1.0f && m3[3] == 4.0f, "Mat init constructors");
        check(m4[2] == 3.0f && m5[3] == 4.0f && m6[1] == 2.0f, "Mat list/vector/span constructors");
        check(out_arr[0] == 1.0f && out_vec[1] == 2.0f && *out_list.begin() == 1.0f, "Mat conversions");
        check(out_span[2] == 3.0f, "Mat span conversion");
        check(Near(add[0], 3.0) && Near(sub[3], 2.0), "Mat add/sub");
        check(Near(mul[0], 7.0) && Near(mul[3], 22.0), "Mat mul");
        check(Near(add_s[0], 2.0) && Near(s_add[3], 5.0), "Mat scalar add");
        check(Near(neg[2], -3.0), "Mat unary -");
        check(Near(mv[0], 3.0) && Near(vm[0], 4.0), "Mat-Vec mul");
        check(Near(mt[2], 2.0), "Transpose");
        check(Near(det, -2.0), "Det");
        check(Near(tr, 5.0), "Trace");
        check(rk == 2, "Rank");
        check(full == true, "IsFullRank");
        check(Near(had[0], 2.0) && Near(kron[0], 1.0), "Hadamard/Kronecker");
        check(Near(inv[0], -2.0) && Near(inv[3], -0.5), "Inverse");
    }

    // ==================== Mat copy/move ====================
    {
        Mat2f a{1, 2, 3, 4};
        Mat2f b(a);
        check(Near(b[0], 1.0) && Near(b[3], 4.0), "Mat copy constructor");

        Mat2f c(std::move(a));
        check(Near(c[1], 2.0), "Mat move constructor");

        Mat2f d;
        d = b;
        check(Near(d[0], 1.0), "Mat copy assignment");

        Mat2f e;
        e = std::move(c);
        check(Near(e[2], 3.0), "Mat move assignment");
    }

    // ==================== Mat type conversion ====================
    {
        Mat2f mf{1, 2, 3, 4};
        Mat2d md(mf);
        check(Near(md[0], 1.0) && Near(md[3], 4.0), "Mat float to double conversion");
    }

    // ==================== Mat static query methods ====================
    {
        check(Mat2f::Size() == 4, "Mat Size()");
        check(Mat2f::RowSize() == 2, "Mat RowSize()");
        check(Mat2f::ColSize() == 2, "Mat ColSize()");

        auto shape = Mat2f::Shape();
        check(std::get<0>(shape) == 2 && std::get<1>(shape) == 2, "Mat Shape()");
    }

    // ==================== MakeIdentity ====================
    {
        Mat3f id = Mat3f::MakeIdentity();
        check(Near(id[0, 0], 1.0) && Near(id[1, 1], 1.0) && Near(id[2, 2], 1.0), "MakeIdentity diagonal");
        check(Near(id[0, 1], 0.0) && Near(id[1, 0], 0.0) && Near(id[2, 0], 0.0), "MakeIdentity off-diagonal");

        Mat4f id4 = Mat4f::MakeIdentity();
        check(Near(id4[0, 0], 1.0) && Near(id4[3, 3], 1.0) && Near(id4[0, 1], 0.0), "MakeIdentity 4x4");
    }

    // ==================== Mat GetRow/GetCol ====================
    {
        Mat3f m{1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto row0 = m.GetRow(0);
        check(Near(row0[0], 1.0) && Near(row0[1], 2.0) && Near(row0[2], 3.0), "Mat GetRow");

        auto col1 = m.GetCol(1);
        check(Near(col1[0], 2.0) && Near(col1[1], 5.0) && Near(col1[2], 8.0), "Mat GetCol");
    }

    // ==================== Mat comparison operators ====================
    {
        Mat2f a{1, 2, 3, 4};
        Mat2f b{1, 2, 3, 4};
        Mat2f c{1, 2, 3, 5};

        check(a == b, "Mat equality");
        check(a != c, "Mat inequality");
    }

    // ==================== Mat compound assignment ====================
    {
        Mat2f a{1, 2, 3, 4};
        a += 1.0f;
        check(Near(a[0], 2.0) && Near(a[3], 5.0), "Mat += scalar");

        a -= 1.0f;
        check(Near(a[0], 1.0), "Mat -= scalar");

        Mat2f b{2, 2, 2, 2};
        a += b;
        check(Near(a[0], 3.0), "Mat += Mat");

        a -= b;
        check(Near(a[0], 1.0), "Mat -= Mat");

        a *= 2.0f;
        check(Near(a[0], 2.0), "Mat *= scalar");

        Mat2f sq{1, 0, 0, 1};
        sq *= Mat2f{2, 0, 0, 2};
        check(Near(sq[0, 0], 2.0) && Near(sq[1, 1], 2.0), "Mat *= Mat");
    }

    // ==================== Mat scalar sub/div ====================
    {
        Mat2f a{5, 5, 5, 5};
        auto sub_s = a - 2.0f;
        auto s_sub = 10.0f - a;
        check(Near(sub_s[0], 3.0), "Mat scalar sub");
        check(Near(s_sub[0], 5.0), "Mat scalar reverse sub");

        auto mul_s = a * 2.0f;
        auto s_mul = 3.0f * a;
        check(Near(mul_s[0], 10.0), "Mat scalar mul");
        check(Near(s_mul[0], 15.0), "Mat scalar reverse mul");
    }

    // ==================== Mat iterator ====================
    {
        Mat2f m{1, 2, 3, 4};
        float sum = 0;
        for (auto val : m) sum += val;
        check(Near(sum, 10.0), "Mat iterator");
    }

    // ==================== Mat pointer conversion ====================
    {
        Mat2f m{1, 2, 3, 4};
        float *p = static_cast<float *>(m);
        check(Near(p[0], 1.0) && Near(p[3], 4.0), "Mat pointer conversion");

        const Mat2f cm{1, 2, 3, 4};
        const float *cp = static_cast<const float *>(cm);
        check(Near(cp[1], 2.0), "Mat const pointer conversion");
    }

    // ==================== Mat assign scalar ====================
    {
        Mat2f m{1, 2, 3, 4};
        m = 5.0f;
        check(Near(m[0], 5.0) && Near(m[3], 5.0), "Mat assign scalar");
    }

    // ==================== Mat const span conversion ====================
    {
        const Mat2f m{1, 2, 3, 4};
        span<const float, 4> sp = m;
        check(Near(sp[0], 1.0) && Near(sp[3], 4.0), "Mat const span conversion");
    }

    // ==================== Transpose non-square ====================
    {
        Mat<float, 2, 3> m{1, 2, 3, 4, 5, 6};
        auto mt = Transpose(m);
        check(Near(mt[0, 0], 1.0) && Near(mt[0, 1], 4.0) && Near(mt[2, 1], 6.0), "Transpose non-square");
    }

    // ==================== Det 1x1 and 3x3 ====================
    {
        Mat<float, 1, 1> m1{5};
        check(Near(Det(m1), 5.0), "Det 1x1");

        Mat3f m3{1, 2, 3, 4, 5, 6, 7, 8, 10};
        check(Near(Det(m3), -3.0), "Det 3x3");
    }

    // ==================== Singular matrix Det ====================
    {
        Mat2f singular{1, 2, 2, 4};
        check(Near(Det(singular), 0.0), "Det singular matrix");
    }

    // ==================== Rank non-full ====================
    {
        Mat2f singular{1, 2, 2, 4};
        check(Rank(singular) == 1, "Rank singular matrix");

        Mat2f zero;
        check(Rank(zero) == 0, "Rank zero matrix");
    }

    // ==================== Cofactor / Adjoint ====================
    {
        Mat3f m{1, 2, 3, 0, 4, 5, 1, 0, 6};
        auto cof = Cofactor(m, 0, 0);
        check(Near(cof, 24.0), "Cofactor");

        auto adj = Adjoint(m);
        auto inv = Inverse(m);
        auto det = Det(m);
        check(Near(adj[0, 0] / det, inv[0, 0]), "Adjoint vs Inverse consistency");
    }

    // ==================== Inverse 3x3 ====================
    {
        Mat3f m{1, 2, 3, 0, 1, 4, 5, 6, 0};
        auto inv = Inverse(m);
        auto identity = m * inv;
        check(Near(identity[0, 0], 1.0) && Near(identity[1, 1], 1.0) && Near(identity[2, 2], 1.0), "Inverse 3x3 correctness");
        check(Near(identity[0, 1], 0.0) && Near(identity[1, 0], 0.0), "Inverse 3x3 off-diagonal");
    }

    // ==================== Kronecker multi-arg ====================
    {
        Mat2f a{1, 2, 3, 4};
        Mat2f b{0, 1, 1, 0};
        auto k = Kronecker(a, b);
        check(k.RowSize() == 4 && k.ColSize() == 4, "Kronecker size");
        check(Near(k[0, 0], 0.0) && Near(k[0, 1], 1.0), "Kronecker values");
    }

    // ==================== Hadamard multi-matrix ====================
    {
        Mat2f a{1, 2, 3, 4};
        Mat2f b{2, 3, 4, 5};
        Mat2f c{3, 4, 5, 6};
        auto h = Hadamard(a, b, c);
        check(Near(h[0], 6.0) && Near(h[1], 24.0) && Near(h[3], 120.0), "Hadamard 3 matrices");
    }

    // ==================== Vec *= Mat ====================
    {
        Vec3f v{1, 0, 0};
        Mat3f m{0, 1, 0, 1, 0, 0, 0, 0, 1};
        v *= m;
        check(Near(v[0], 0.0) && Near(v[1], 1.0), "Vec *= Mat");
    }

    // ==================== MatView tests ====================
    {
        Mat4f m = Mat4f::MakeIdentity();
        auto view = m[Range<0, 4, 2>{}, Range<0, 4, 2>{}];
        view = {1, 2, 3, 4};
        check(Near(m[0], 1.0) && Near(m[10], 4.0), "MatView assign init_list");

        Mat4f m2 = Mat4f::MakeIdentity();
        auto view2 = m2[Range<0, 2>{}, Range<0, 2>{}];
        Mat2f sub{5, 6, 7, 8};
        view2 = sub;
        check(Near(m2[0, 0], 5.0) && Near(m2[0, 1], 6.0) && Near(m2[1, 0], 7.0) && Near(m2[1, 1], 8.0), "MatView assign Mat");

        Mat2f extracted = view2;
        check(Near(extracted[0, 0], 5.0) && Near(extracted[1, 1], 8.0), "MatView to Mat conversion");
    }

    // ==================== SetViewMatrix 3D ====================
    {
        Mat4f view;
        Vec3f pos{0, 0, 5};
        Vec3f dir{0, 0, -1};
        Vec3f up{0, 1, 0};
        SetViewMatrix(view, pos, dir, up);
        check(Near(view[3, 2], -5.0), "SetViewMatrix 3D translation");
        check(Near(view[0, 0], 1.0) && Near(view[1, 1], 1.0), "SetViewMatrix 3D rotation");
    }

    // ==================== SetViewMatrix 2D ====================
    {
        Mat4f view;
        SetViewMatrix(view, Vec2f{0, 0}, 0.0f, 1.0f);
        check(Near(view[0, 0], 1.0) && Near(view[1, 1], 1.0), "SetViewMatrix 2D identity");

        Mat4f view2;
        SetViewMatrix(view2, Vec2f{1, 0}, 0.0f, 1.0f);
        check(Near(view2[0, 3], -1.0), "SetViewMatrix 2D translation");

        Mat4f view3;
        SetViewMatrix(view3, Vec2f{0, 0}, std::numbers::pi_v<float> / 2.0f, 1.0f);
        check(Near(view3[0, 0], 0.0, 1e-4) && Near(view3[0, 1], 1.0, 1e-4), "SetViewMatrix 2D rotation");
    }

    // ==================== SetProjectionMatrix perspective ====================
    {
        Mat4f proj;
        SetProjectionMatrix(proj, std::numbers::pi_v<float> / 2.0f, 1.0f, 0.1f, 100.0f);
        check(Near(proj[0, 0], 1.0) && proj[3, 2] == -1.0f, "SetProjectionMatrix perspective values");
    }

    // ==================== SetProjectionMatrix ortho (6 params) ====================
    {
        Mat4f ortho;
        SetProjectionMatrix(ortho, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        check(Near(ortho[0, 0], 1.0) && Near(ortho[1, 1], 1.0) && Near(ortho[2, 2], -1.0), "SetProjectionMatrix ortho 6-param");
        check(Near(ortho[3, 3], 1.0), "SetProjectionMatrix ortho bottom-right");
    }

    // ==================== SetProjectionMatrix ortho (2 params) ====================
    {
        Mat4f ortho;
        SetProjectionMatrix(ortho, 800.0f, 600.0f);
        float expected_00 = 2.0f / 800.0f;
        float expected_11 = 2.0f / 600.0f;
        check(Near(ortho[0, 0], expected_00), "SetProjectionMatrix ortho 2-param X scale");
        check(Near(std::abs(ortho[1, 1]), expected_11), "SetProjectionMatrix ortho 2-param Y scale");
        check(Near(ortho[3, 3], 1.0), "SetProjectionMatrix ortho 2-param bottom-right");
    }

    // ==================== Exception tests ====================
    {
        bool caught = false;
        try
        {
            Mat2f singular{1, 2, 2, 4};
            Inverse(singular);
        }
        catch (const std::runtime_error &)
        {
            caught = true;
        }
        check(caught, "Inverse singular matrix throws");

        caught = false;
        try
        {
            Vec3f v{1, 2, 3};
            Vec3f v2(std::initializer_list<float>{1, 2});
        }
        catch (const std::runtime_error &)
        {
            caught = true;
        }
        check(caught, "Vec initializer_list size mismatch throws");
    }

    // ==================== Output operator smoke tests ====================
    {
        Vec3f v{1, 2, 3};
        Mat2f m{1, 2, 3, 4};
        (void)v;
        (void)m;
    }

    cout << "\nTotal: " << total << ", Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}