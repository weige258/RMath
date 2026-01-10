#include <array>
#include <list>
#include <vector>
#include <span>
#include <concepts>
#include <iostream>

using namespace std;

namespace Detail
{
    // 概念：Numeric 表示数值类型（整数或浮点数）
    template <typename T>
    concept Numeric = std::is_arithmetic_v<T>;

    // 结构体：ComplieTimeIndexCheck 用于编译期检查索引是否超出范围
    template <std::size_t Limit>
    struct ComplieTimeIndexCheck
    {
        std::size_t value;

        consteval ComplieTimeIndexCheck(std::size_t i) : value(i)
        {
            if (i >= Limit)
            {
                throw std::out_of_range("Vec index out of bounds!");
            }
        }
    };
}

// Vec 对象
template <Detail::Numeric T, std::size_t N>
struct Vec
{
protected:
    // 数据
    std::array<T, N> data;

public:
    using value_type_alias = T;
    template <Detail::Numeric U, std::size_t M>
    friend struct Vec;

public:
    // 构造
    Vec() { data.fill(0); }

    template <typename... Args>
    Vec(const Args &...args)
        requires(sizeof...(args) == N && (std::convertible_to<Args, T> && ...))
    {
        data = std::array<T, N>{static_cast<T>(args)...};
    };

    Vec(const Vec &other) = default;

    Vec(Vec &&other) = default;

    Vec(T *arr, size_t n)
        requires(n == N)
    {
        for (size_t i = 0; i < n; ++i)
        {
            data[i] = arr[i];
        }
    }

    Vec(const std::array<T, N> &array) : data(array) {}

    template <typename U>
    Vec(const std::list<U> &list)
        requires std::convertible_to<U, T>
    {
        if (list.size() != N)
        {
            throw std::runtime_error("List size does not match vector dimension");
        }
        std::copy(list.begin(), list.end(), data.begin());
    }

    template <typename U>
    Vec(const std::vector<U> &vector)
        requires std::convertible_to<U, T>
    {
        if (vector.size() != N)
        {
            throw std::runtime_error("Vector size does not match vector dimension");
        }
        std::copy(vector.begin(), vector.end(), data.begin());
    }

    Vec(std::span<const T, N> s)
    {
        std::copy(s.begin(), s.end(), data.begin());
    }

    // 析构
    ~Vec() = default;

    // 类型转换构造函数
    template <typename U>
    Vec(const Vec<U, N> &other)
        requires std::convertible_to<U, T>
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] = static_cast<T>(other.data[i]);
        }
    }

    // 标准库类型转换
    operator std::array<T, N>() const
    {
        return data;
    }

    operator std::vector<T>() const
    {
        return std::vector<T>(data.begin(), data.end());
    }

    operator std::list<T>() const
    {
        return std::list<T>(data.begin(), data.end());
    }

    operator std::span<const T, N>() const
    {
        return std::span<const T, N>(data);
    }

    // 指针转换
    operator T *() { return data.data(); }
    operator const T *() const { return data.data(); }

    // 访问
    T &operator[](Detail::ComplieTimeIndexCheck<N> index)
    {
        return data[index.value];
    }

    T &x()
        requires(0 < N)
    {
        return data[0];
    }

    T &y()
        requires(1 < N)
    {
        return data[1];
    }

    T &z()
        requires(2 < N)
    {
        return data[2];
    }

    T &w()
        requires(3 < N)
    {
        return data[3];
    }

    // 赋值
    Vec &operator=(const Vec &other) = default;

    Vec &operator=(Vec &&other) = default;

    // 加法
    template <Detail::Numeric U>
    friend auto operator+(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] = static_cast<ResultType>(lhs.data[i]) + static_cast<ResultType>(rhs.data[i]);
        }
        return result;
    }

    Vec operator+(const T &value)
    {
        Vec result = *this;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] += value;
        }
        return result;
    }

    // 减法
    template <Detail::Numeric U>
    friend auto operator-(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] = static_cast<ResultType>(lhs.data[i]) - static_cast<ResultType>(rhs.data[i]);
        }
        return result;
    }

    Vec operator-(const T &value)
    {
        Vec result = *this;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] -= value;
        }
        return result;
    }

    // 乘法
    template <Detail::Numeric U>
    friend auto operator*(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] = static_cast<ResultType>(lhs.data[i]) * static_cast<ResultType>(rhs.data[i]);
        }
        return result;
    }

    Vec operator*(const T &value)
    {
        Vec result = *this;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] *= value;
        }
        return result;
    }

    // 除法
    template <Detail::Numeric U>
    friend auto operator/(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] = static_cast<ResultType>(lhs.data[i]) / static_cast<ResultType>(rhs.data[i]);
        }
        return result;
    }

    Vec operator/(const T &value)
    {
        Vec result = *this;
        for (size_t i = 0; i < N; ++i)
        {
            result.data[i] /= value;
        }
        return result;
    }

    // 叉乘
    Vec operator^(const Vec &other) const
        requires(N == 3)
    {
        return Vec(
            data[1] * other.data[2] - data[2] * other.data[1],
            data[2] * other.data[0] - data[0] * other.data[2],
            data[0] * other.data[1] - data[1] * other.data[0]);
    }

    // 复合赋值操作符
    Vec &operator+=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] += other.data[i];
        }
        return *this;
    }

    Vec &operator+=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] += value;
        }
        return *this;
    }

    Vec &operator-=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] -= other.data[i];
        }
        return *this;
    }

    Vec &operator-=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] -= value;
        }
        return *this;
    }

    Vec &operator*=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] *= other.data[i];
        }
        return *this;
    }

    Vec &operator*=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] *= value;
        }
        return *this;
    }

    Vec &operator/=(const Vec &other)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] /= other.data[i];
        }
        return *this;
    }

    Vec &operator/=(const T &value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            data[i] /= value;
        }
        return *this;
    }

    // 迭代器支持
    auto begin() noexcept { return data.begin(); }
    auto end() noexcept { return data.end(); }
    auto begin() const noexcept { return data.begin(); }
    auto end() const noexcept { return data.end(); }

    // 比较操作符 (C++20)
    auto operator<=>(const Vec &other) const = default;

    // 查询方法
    static constexpr size_t size() noexcept { return N; }

    static constexpr size_t length() noexcept { return N; }

    static constexpr size_t bytes() noexcept { return N * sizeof(T); }

    static const type_info &type() noexcept { return typeid(Vec<T, N>); }

    static const type_info &value_type() noexcept { return typeid(T); }
};

// 点积
template <typename... Vecs>
    requires(sizeof...(Vecs) >= 2)
auto Dot(const Vecs &...vecs)
{
    // 1. 获取第一个向量的维度 N，并强制后续所有向量维度必须与之匹配
    constexpr std::size_t N = (std::tuple_element_t<0, std::tuple<Vecs...>>::size());
    static_assert(((vecs.size() == N) && ...), "All vectors must have the same dimension N");

    // 2. 推导所有向量元素相乘后的公共返回类型
    using ResultType = std::common_type_t<typename Vecs::value_type_alias...>;

    ResultType total_sum = 0;

    // 3. 遍历维度进行累加
    for (std::size_t i = 0; i < N; ++i)
    {
        // 利用折叠表达式 (Fold Expression) 实现：vec1[i] * vec2[i] * ...
        total_sum += (static_cast<ResultType>(vecs[i]) * ...);
    }

    return total_sum;
}

// 类型推导申明
template <Detail::Numeric... Args>
Vec(Args...) -> Vec<std::common_type_t<Args...>, sizeof...(Args)>;

template <typename T, std::size_t N>
Vec(std::array<T, N>) -> Vec<T, N>;

template <typename T, std::size_t N>
Vec(std::span<T, N>) -> Vec<T, N>;