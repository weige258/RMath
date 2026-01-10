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

// Vec 对象：元素类型为 T,表示一个 N 维向量
template <Detail::Numeric T, std::size_t N>
struct Vec
{
protected:
    // 数据
    std::array<T, N> data;

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

    Vec(std::span<const T, N> s) {
    std::copy(s.begin(), s.end(), data.begin());
}

    // 析构
    ~Vec() = default;

    // 结构化访问
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

    T &operator[](Detail::ComplieTimeIndexCheck<N> index)
    {
        return data[index.value];
    }
};

// 类型推导申明
template <typename... Args>
Vec(Args...) -> Vec<std::common_type_t<Args...>, sizeof...(Args)>;
