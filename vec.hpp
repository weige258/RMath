#include <array>
#include <list>
#include <vector>
#include <span>
#include <concepts>
#include <iostream>
#include <cmath>
#include "range.hpp"

#ifndef VEC_HPP
#define VEC_HPP

namespace Detail
{
    // 概念：Numeric 表示数值类型（整数或浮点数）
    template <typename T>
    concept NumericVec = std::is_arithmetic_v<T>;

    // 结构体：ComplieTimeIndexCheck 用于编译期检查索引是否超出范围
    template <std::size_t Limit>
    struct ComplieTimeIndexCheckVec
    {
        std::size_t value;

        consteval ComplieTimeIndexCheckVec(std::size_t i) : value(i)
        {
            static_assert(i >= Limit, "Vec index out of bounds!");
        }
    };
}

// Vec 对象
template <Detail::NumericVec T, std::size_t N>
struct Vec
{
private:
    // 数据
    std::array<T, N> _data;

public:
    using vec_type_alias = T;
    template <Detail::NumericVec U, std::size_t M>
    friend struct Vec;

public:
    // 构造
    constexpr Vec() { _data.fill(0); }

    constexpr Vec(T num) { _data.fill(num); }

    constexpr Vec(const Vec &other) = default;

    constexpr Vec(Vec &&other) = default;

    constexpr Vec(std::initializer_list<T> list)
    {
        if (list.size() != N)
        {
            throw std::runtime_error("Initializer list size mismatch");
        }
        std::copy(list.begin(), list.end(), _data.begin());
    }

    template <typename... Args>
    constexpr Vec(const Args &...args)
        requires(sizeof...(args) == N && (std::convertible_to<Args, T> && ...))
    {
        _data = std::array<T, N>{static_cast<T>(args)...};
    };

    constexpr Vec(const T *arr)
    {
        for (size_t i = 0; i < N; ++i)
            _data[i] = arr[i];
    }

    constexpr Vec(const std::array<T, N> &array) : _data(array) {}

    template <typename U>
    constexpr Vec(const std::list<U> &list)
        requires std::convertible_to<U, T>
    {
        if (list.size() != N)
        {
            throw std::runtime_error("List size does not match vector dimension");
        }
        std::copy(list.begin(), list.end(), _data.begin());
    }

    template <typename U>
    constexpr Vec(const std::vector<U> &vector)
        requires std::convertible_to<U, T>
    {
        if (vector.size() != N)
        {
            throw std::runtime_error("Vector size does not match vector dimension");
        }
        std::copy(vector.begin(), vector.end(), _data.begin());
    }

    constexpr Vec(std::span<const T, N> s)
    {
        std::copy(s.begin(), s.end(), _data.begin());
    }

    // 析构
    ~Vec() = default;

    // 类型转换
    template <typename U>
    constexpr Vec(const Vec<U, N> &other)
        requires std::convertible_to<U, T>
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] = static_cast<T>(other._data[i]);
        }
    }

    template <typename U>
        requires Detail::NumericVec<U>
    operator std::array<U, N>() const
    {
        std::array<U, N> result;
        for (int i = 0; i < N; ++i)
        {
            result[i] = static_cast<U>(_data[i]);
        }
        return result;
    }

    template <typename U>
        requires Detail::NumericVec<U>
    operator std::vector<U>() const
    {
        std::vector<U> result;
        result.reserve(N);
        for (const auto &val : _data)
        {
            result.push_back(static_cast<U>(val));
        }
        return result;
    }

    template <typename U>
        requires Detail::NumericVec<U>
    operator std::list<U>() const
    {
        std::list<U> result;
        for (const auto &val : _data)
        {
            result.push_back(static_cast<U>(val));
        }
        return result;
    }

    operator std::span<const T, N>() const
    {
        return std::span<const T, N>(_data);
    }

    // 指针转换
    explicit operator T *() { return _data._data(); }
    explicit operator const T *() const { return _data._data(); }

    // 访问
    constexpr T &operator[](std::size_t index)
    {
        return _data[index];
    }

    constexpr const T &operator[](std::size_t index) const
    {
        return _data[index];
    }

    constexpr T &operator[](Detail::ComplieTimeIndexCheckVec<N> index)
    {
        return _data[index.value];
    }

    template <int start, int end, int step>
    constexpr auto operator[](StaticRange<start, end, step> range) const
    {
        constexpr size_t outDim = decltype(range)::size;

        if constexpr (outDim > 0)
        {
            constexpr int lastIdx = start + (static_cast<int>(outDim) - 1) * step;
            static_assert(start >= 0 && start < (int)N, "Vec slice start out of bounds!");
            static_assert(lastIdx >= 0 && lastIdx < (int)N, "Vec slice end out of bounds!");
        }
        Vec<T, outDim> result;
        int i = 0;
        for (auto idx : range)
        {
            result[i++] = _data[idx];
        }
        return result;
    }

    constexpr T &x()
        requires(0 < N)
    {
        return _data[0];
    }

    constexpr T &y()
        requires(1 < N)
    {
        return _data[1];
    }

    constexpr T &z()
        requires(2 < N)
    {
        return _data[2];
    }

    constexpr T &w()
        requires(3 < N)
    {
        return _data[3];
    }

    // 赋值
    constexpr Vec &operator=(const Vec &other) = default;

    constexpr Vec &operator=(Vec &&other) = default;

    constexpr Vec &operator=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] = value;
        }
        return *this;
    }

    // 运算
    template <Detail::NumericVec U>
    constexpr friend auto operator+(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result._data[i] = static_cast<ResultType>(lhs._data[i]) + static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Vec<T, N>> && Detail::NumericVec<RType>) ||
            (Detail::NumericVec<LType> && std::same_as<RType, Vec<T, N>>))
    constexpr friend auto operator+(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Vec<ResultType, N> result;

        if constexpr (std::same_as<LType, Vec<T, N>>)
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs._data[i]) + static_cast<ResultType>(rhs);
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs) + static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <Detail::NumericVec U>
    constexpr friend auto operator-(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result._data[i] = static_cast<ResultType>(lhs._data[i]) - static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Vec<T, N>> && Detail::NumericVec<RType>) ||
            (Detail::NumericVec<LType> && std::same_as<RType, Vec<T, N>>))
    constexpr friend auto operator-(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Vec<ResultType, N> result;

        if constexpr (std::same_as<LType, Vec<T, N>>)
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs._data[i]) - static_cast<ResultType>(rhs);
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs) - static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <Detail::NumericVec U>
    constexpr friend auto operator*(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result._data[i] = static_cast<ResultType>(lhs._data[i]) * static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Vec<T, N>> && Detail::NumericVec<RType>) ||
            (Detail::NumericVec<LType> && std::same_as<RType, Vec<T, N>>))
    constexpr friend auto operator*(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Vec<ResultType, N> result;

        if constexpr (std::same_as<LType, Vec<T, N>>)
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs._data[i]) * static_cast<ResultType>(rhs);
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs) * static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <Detail::NumericVec U>
    constexpr friend auto operator/(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result._data[i] = static_cast<ResultType>(lhs._data[i]) / static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Vec<T, N>> && Detail::NumericVec<RType>) ||
            (Detail::NumericVec<LType> && std::same_as<RType, Vec<T, N>>))
    constexpr friend auto operator/(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Vec<ResultType, N> result;

        if constexpr (std::same_as<LType, Vec<T, N>>)
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs._data[i]) / static_cast<ResultType>(rhs);
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
                result._data[i] = static_cast<ResultType>(lhs) / static_cast<ResultType>(rhs._data[i]);
        }
        return result;
    }

    constexpr Vec operator/(const Vec &other) const
    {
        Vec result{};
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = _data[i] / other._data[i];
        }
        return result;
    }

    constexpr Vec operator^(const Vec &other) const
        requires(N == 3)
    {
        return Vec(
            _data[1] * other._data[2] - _data[2] * other._data[1],
            _data[2] * other._data[0] - _data[0] * other._data[2],
            _data[0] * other._data[1] - _data[1] * other._data[0]);
    }

    constexpr Vec operator-()
    {
        Vec result{};
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = -_data[i];
        }
        return result;
    }

    // 复合赋值操作符
    constexpr Vec &operator+=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] += other._data[i];
        }
        return *this;
    }

    constexpr Vec &operator+=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] += value;
        }
        return *this;
    }

    constexpr Vec &operator-=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] -= other._data[i];
        }
        return *this;
    }

    constexpr Vec &operator-=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] -= value;
        }
        return *this;
    }

    constexpr Vec &operator*=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] *= other._data[i];
        }
        return *this;
    }

    constexpr Vec &operator*=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] *= value;
        }
        return *this;
    }

    constexpr Vec &operator/=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] /= other._data[i];
        }
        return *this;
    }

    constexpr Vec &operator/=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] /= value;
        }
        return *this;
    }

    constexpr Vec &operator^=(const Vec &other)
        requires(N == 3)
    {
        T x = _data[1] * other._data[2] - _data[2] * other._data[1];
        T y = _data[2] * other._data[0] - _data[0] * other._data[2];
        T z = _data[0] * other._data[1] - _data[1] * other._data[0];
        _data[0] = x;
        _data[1] = y;
        _data[2] = z;
        return *this;
    };

    // 迭代器支持
    auto begin() noexcept { return _data.begin(); }
    auto end() noexcept { return _data.end(); }
    auto begin() const noexcept { return _data.begin(); }
    auto end() const noexcept { return _data.end(); }

    // 比较操作符 (C++20)
    auto operator<=>(const Vec &other) const = default;

    // 查询方法
    static constexpr size_t size() noexcept { return N; }

    static constexpr size_t size_in_bytes() noexcept { return N * sizeof(T); }

    static const std::type_info &type() noexcept { return typeid(Vec<T, N>); }

    static const std::type_info &value_type() noexcept { return typeid(T); }
};

// 模长
template <Detail::NumericVec T, std::size_t N>
T Length(const Vec<T, N> &v)
{
    T sum = 0;
    for (size_t i = 0; i < N; ++i)
    {
        sum += v[i] * v[i];
    }
    return sqrt(sum);
}

// 归一化
template <Detail::NumericVec T, std::size_t N>
Vec<T, N> Normalize(const Vec<T, N> &v)
{
    T len = Length(v);
    if (len > 0)
    {
        return (v) / len;
    }
    return Vec<T, N>{};
}

// 点积
template <typename... Vecs>
    requires(sizeof...(Vecs) >= 2)
auto Dot(const Vecs &...vecs)
{
    constexpr std::size_t N = (std::tuple_element_t<0, std::tuple<Vecs...>>::size());
    static_assert(((vecs.size() == N) && ...), "All vectors must have the same dimension N");
    using ResultType = std::common_type_t<typename Vecs::vec_type_alias...>;
    ResultType total_sum = 0;
    for (std::size_t i = 0; i < N; ++i)
    {
        total_sum += (static_cast<ResultType>(vecs[i]) * ...);
    }

    return total_sum;
}

// 向量 Hadamard 积 (按元素相乘)
template <typename... Args>
    requires(sizeof...(Args) >= 2) &&
            (... && requires { typename std::remove_cvref_t<Args>::vec_type_alias; })
constexpr auto Hadamard(const Args &...args)
{

    using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;
    constexpr size_t N = std::remove_cvref_t<FirstArg>::size();

    static_assert((... && (Args::size() == N)),
                  "All vectors must have the same dimension for Hadamard product.");

    using ResultScalar = std::common_type_t<typename std::remove_cvref_t<Args>::vec_type_alias...>;

    Vec<ResultScalar, N> result;

    for (size_t i = 0; i < N; ++i)
    {
        result[i] = (static_cast<ResultScalar>(args[i]) * ...);
    }

    return result;
}

// 拼接
template <typename... Vecs>
    requires(sizeof...(Vecs) >= 1)
auto Cat(const Vecs &...vecs)
{
    constexpr std::size_t TotalN = (Vecs::size() + ...);
    using ResultT = std::common_type_t<typename Vecs::vec_type_alias...>;
    Vec<ResultT, TotalN> result;
    std::size_t offset = 0;
    ([&](const auto &v)
     {
        for (std::size_t i = 0; i < v.size(); ++i) {
            result[offset++] = static_cast<ResultT>(v[i]);
        } }(vecs), ...);

    return result;
}

// 计算距离
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
auto Distance(const Vec<T, N> &a, const Vec<U, N> &b)
{
    using CalcT = std::common_type_t<T, U>;
    Vec<CalcT, N> diff;
    for (size_t i = 0; i < N; ++i)
    {
        diff[i] = static_cast<CalcT>(a[i]) - static_cast<CalcT>(b[i]);
    }

    return std::sqrt(Dot(diff, diff));
}

// 线性插值
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N, typename V>
auto Lerp(const Vec<T, N> &a, const Vec<U, N> &b, V t)
{
    using ResultT = std::common_type_t<T, U, V>;
    return Vec<ResultT, N>(a) * (static_cast<ResultT>(1.0) - static_cast<ResultT>(t)) + Vec<ResultT, N>(b) * static_cast<ResultT>(t);
}

// 2. 投影 Project
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
auto Project(const Vec<T, N> &a, const Vec<U, N> &b)
{
    using ResultT = std::common_type_t<T, U>;
    auto dot_val = Dot(a, b);
    auto b_mag_sq = Dot(b, b);
    return Vec<ResultT, N>(b) * (static_cast<ResultT>(dot_val) / static_cast<ResultT>(b_mag_sq));
}

// 3. 反射 Reflect
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
auto Reflect(const Vec<T, N> &a, const Vec<U, N> &n)
{
    using ResultT = std::common_type_t<T, U>;
    return Vec<ResultT, N>(a) - Vec<ResultT, N>(n) * (static_cast<ResultT>(2) * Dot(a, n));
}

// 输出运算符
template <Detail::NumericVec T, std::size_t N>
std::ostream &operator<<(std::ostream &os, const Vec<T, N> &vec)
{
    os << "(";
    for (size_t i = 0; i < N; ++i)
    {
        os << vec[i];
        if (i < N - 1)
            os << ", ";
    }
    os << ")";
    return os;
}

// 类型推导申明
template <Detail::NumericVec... Args>
Vec(Args...) -> Vec<std::common_type_t<Args...>, sizeof...(Args)>;

template <typename T, std::size_t N>
Vec(std::array<T, N>) -> Vec<T, N>;

template <typename T, std::size_t N>
Vec(std::span<T, N>) -> Vec<T, N>;

// 常用向量类型
using Vec2i = Vec<int, 2>;
using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;
using Vec2l = Vec<long, 2>;
using Vec3i = Vec<int, 3>;
using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;
using Vec3l = Vec<long, 3>;
using Vec4i = Vec<int, 4>;
using Vec4f = Vec<float, 4>;
using Vec4d = Vec<double, 4>;
using Vec4l = Vec<long, 4>;

#endif // VEC_HPP
