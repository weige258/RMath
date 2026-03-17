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

    // Range tests
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
    }

    // Vec constructors and conversions
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

    // Vec operations
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
        check(cross[2] == 1.0f, "Cross");
        check(had[1] == 6.0f, "Hadamard");
        check(cat.Size() == 5 && cat[4] == 5.0f, "Cat");
        check(Near(dist, 5.0), "Distance");
        check(lerp[0] == 5.0f, "Lerp");
        check(proj[0] == 2.0f && proj[1] == 0.0f, "Project");
        check(refl[1] == 1.0f, "Reflect");
        check(Near(rad, std::numbers::pi / 2.0), "Radian");
        check(Near(deg, 90.0), "Degree");
    }

    // VecView tests
    {
        Vec4f v{1, 2, 3, 4};
        auto view = v[Range<0, 4, 2>{}];
        Vec2f vv = view;
        check(vv[0] == 1.0f && vv[1] == 3.0f, "VecView conversion");

        view = Vec2f{9, 8};
        check(v[0] == 9.0f && v[2] == 8.0f, "VecView assign Vec");

        view = {7, 6};
        check(v[0] == 7.0f && v[2] == 6.0f, "VecView assign init_list");

        array<float, 2> arr = {5, 4};
        view = arr;
        check(v[0] == 5.0f && v[2] == 4.0f, "VecView assign array");

        vector<float> vec = {3, 2};
        list<float> lst = {1, 0};
        view = vec;
        check(v[0] == 3.0f && v[2] == 2.0f, "VecView assign vector");
        view = lst;
        check(v[0] == 1.0f && v[2] == 0.0f, "VecView assign list");

        span<float, 2> sp(arr);
        view = sp;
        check(v[0] == 5.0f && v[2] == 4.0f, "VecView assign span");
    }

    // Mat constructors, conversions, and operations
    {
        Mat2f m0;
        Mat2f m1(2.0f);
        Mat2f m2{1, 2, 3, 4};
        Mat2f m3{{1, 2}, {3, 4}};

        vector<float> vec = {1, 2, 3, 4};
        list<float> lst = {1, 2, 3, 4};
        array<float, 4> arr = {1, 2, 3, 4};
        span<float, 4> sp(arr);
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
        check(add[0] == 3.0f && sub[3] == 2.0f, "Mat add/sub");
        check(mul[0] == 7.0f && mul[3] == 22.0f, "Mat mul");
        check(add_s[0] == 2.0f && s_add[3] == 5.0f, "Mat scalar add");
        check(neg[2] == -3.0f, "Mat unary -");
        check(mv[0] == 3.0f && vm[0] == 4.0f, "Mat-Vec mul");
        check(mt[2] == 2.0f, "Transpose");
        check(Near(det, -2.0), "Det");
        check(tr == 5.0f, "Trace");
        check(rk == 2, "Rank");
        check(full == true, "IsFullRank");
        check(had[0] == 2.0f && kron[0] == 1.0f, "Hadamard/Kronecker");
        check(Near(inv[0], -2.0) && Near(inv[3], -0.5), "Inverse");
    }

    // MatView and projection/view matrix tests
    {
        Mat4f m = Mat4f::MakeIdentity();
        auto view = m[Range<0, 4, 2>{}, Range<0, 4, 2>{}];
        view = {1, 2, 3, 4};
        check(m[0] == 1.0f && m[10] == 4.0f, "MatView assign init_list");

        Mat4f view2d;
        view2d.SetViewMatrix(Vec2f{1, 2}, 0.0f, 1.0f);
        check(view2d[0] == 1.0f, "SetViewMatrix 2D");

        Mat4f proj;
        proj.SetProjectionMatrix(std::numbers::pi_v<float> / 2.0f, 1.0f, 0.1f, 100.0f);
        check(proj[0] != 0.0f && proj[10] < 0.0f, "SetProjectionMatrix perspective");
    }

    cout << "\nTotal: " << total << ", Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
