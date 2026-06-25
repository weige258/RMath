#pragma once

#include <tuple>
#include <concepts>
#include <array>
#include <cmath>
#include <cassert>
#include <variant>
#include "vec.hpp"
#include "range.hpp"

namespace Detail
{
    template <typename T>
    concept NumericMat = std::is_arithmetic_v<T>;

    template <NumericMat T>
    constexpr bool IsNearZero(T val)
    {
        if constexpr (std::is_floating_point_v<T>)
            return std::abs(val) < static_cast<T>(1e-6);
        else
            return val == 0;
    }

    // 多维initlist构造行数据
    template <Detail::NumericMat T, size_t Col>
    struct RowData
    {
        std::array<T, Col> row_values;

        template <typename... Args>
        constexpr RowData(Args &&...args)
            : row_values{static_cast<T>(args)...} {}
    };

    template <typename T>
    struct NullEmptyPtr
    {
        const T *ptr;
        consteval NullEmptyPtr(std::nullptr_t)
        {
            static_assert(sizeof(T) == 0, "Mat cannot be initialized with nullptr!");
        }
        consteval NullEmptyPtr(const T *p) : ptr(p) {}

        constexpr T operator[](size_t i) const { return ptr[i]; }
    };

    template <typename T, size_t Row, size_t Col>
    inline constexpr bool MatUseSIMD = simd::SupportsSIMD<T> && (Row * Col >= simd::SIMDWidth<T>);
}

// 矩阵视图类型声明
template <Detail::NumericMat T, size_t Row, size_t Col, typename RowRange, typename ColRange>
struct MatView;

// 矩阵类型
template <Detail::NumericMat T, size_t Row, size_t Col>
struct Mat final
{
private:
    std::array<T, Row * Col> m_data;

public:
    using mat_type_alias = T;

    // 标量混合运算符的友元声明
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator+(const Mat<U, R, C> &lhs, V rhs);
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator+(V lhs, const Mat<U, R, C> &rhs);
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator-(const Mat<U, R, C> &lhs, V rhs);
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator-(V lhs, const Mat<U, R, C> &rhs);
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator*(const Mat<U, R, C> &lhs, V rhs);
    template <Detail::NumericMat U, size_t R, size_t C, Detail::NumericMat V>
    friend constexpr auto operator*(V lhs, const Mat<U, R, C> &rhs);

public:
    // 构造
    constexpr Mat() { m_data.fill(0); }

    constexpr Mat(T num) { m_data.fill(num); }

    constexpr Mat(const Mat &other) = default;

    constexpr Mat(Mat &&other) noexcept = default;

    template <Detail::NumericMat U>
    constexpr Mat(const std::initializer_list<U> &list)
    {
        if (list.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }
        size_t i = 0;
        for (const auto &val : list)
        {
            m_data[i++] = static_cast<T>(val);
        }
    }

    constexpr Mat(const std::initializer_list<Detail::RowData<T, Col>> &list)
    {
        if (list.size() != Row)
        {
            throw std::runtime_error("Row count mismatch");
        }
        size_t r = 0;
        for (const auto &row : list)
        {
            for (size_t c = 0; c < Col; ++c)
            {
                m_data[r * Col + c] = row.row_values[c];
            }
            r++;
        }
    }

    template <typename... Args>
    constexpr Mat(const Args &...args)
        requires(sizeof...(args) == Row * Col && (std::convertible_to<Args, T> && ...))
    {
        m_data = std::array<T, Row * Col>{static_cast<T>(args)...};
    };

    template <typename U, size_t OtherRow, size_t OtherCol>
    constexpr Mat(const Mat<U, OtherRow, OtherCol> &other)
    {
        static_assert(OtherRow == Row && OtherCol == Col, "Matrix dimension mismatch: Row and Col must be equal for conversion.");
        for (size_t i = 0; i < Row * Col; ++i)
        {
            m_data[i] = static_cast<T>(other[i]);
        }
    }

    template <Detail::NumericMat U>
    constexpr Mat(Mat<U, Row, Col> &&other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            m_data[i] = static_cast<T>(other[i]);
        }
    }

    constexpr Mat(const Detail::NullEmptyPtr<T> arr)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            m_data[i] = arr[i];
        }
    }

    constexpr Mat(const std::array<T, Row * Col> &arr) : m_data(arr) {}

    template <Detail::NumericMat U>
    constexpr Mat(const std::list<U> &list)
        requires std::convertible_to<U, T>
    {
        if (list.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }

        std::copy(list.begin(), list.end(), m_data.begin());
    }

    template <Detail::NumericMat U>
    constexpr Mat(const std::vector<U> &vec)
        requires std::convertible_to<U, T>
    {
        if (vec.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }

        std::copy(vec.begin(), vec.end(), m_data.begin());
    }

    constexpr Mat(const std::span<const T, Row * Col> &span)
    {
        std::copy(span.begin(), span.end(), m_data.begin());
    }

    static constexpr Mat<T, Row, Col> MakeIdentity()
    {
        static_assert(Row == Col, "Identity matrix must be square.");
        Mat<T, Row, Col> res;
        for (size_t i = 0; i < Row; ++i)
            res[i, i] = 1;
        return res;
    }

    // 析构
    ~Mat() = default;

    // 数据转化
    template <Detail::NumericMat U>
    constexpr operator std::array<U, Row *Col>() const
    {
        std::array<U, Row * Col> result{};
        for (size_t i = 0; i < Row * Col; ++i)
        {
            result[i] = static_cast<U>(m_data[i]);
        }
        return result;
    }

    template <Detail::NumericMat U>
    constexpr operator std::list<U>() const
    {
        return std::list<U>(m_data.begin(), m_data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::vector<U>() const
    {
        return std::vector<U>(m_data.begin(), m_data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::span<U, Row *Col>() 
    {
        return std::span<U, Row * Col>(m_data.begin(), m_data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::span<const U, Row *Col>() const
    {
        return std::span<const U, Row * Col>(m_data.begin(), m_data.end());
    }

    // 指针转换
    explicit operator T *() { return m_data.data(); }
    explicit operator const T *() const { return m_data.data(); }

    // 访问
    constexpr T &operator[](size_t index)
    {
        assert(index < Row * Col);
        return m_data[index];
    }

    constexpr const T &operator[](size_t index) const
    {
        assert(index < Row * Col);
        return m_data[index];
    }

    constexpr T &operator[](size_t row, size_t col)
    {
        assert(row < Row && col < Col);
        return m_data[row * Col + col];
    }

    constexpr const T &operator[](size_t row, size_t col) const
    {
        assert(row < Row && col < Col);
        return m_data[row * Col + col];
    }

    template <int RStart, int REnd, int RStep,
              int CStart, int CEnd, int CStep>
    constexpr auto operator[](Range<RStart, REnd, RStep> rr,
                              Range<CStart, CEnd, CStep> rc)
    {
        return MatView<T, Row, Col, decltype(rr), decltype(rc)>(*this, rr, rc);
    }

    constexpr Mat<T, 1, Col> GetRow(size_t row) const
    {
        Mat<T, 1, Col> result;

        size_t j = 0;
        for (size_t i = row * Col; i < row * Col + Col; ++i)
        {
            result[j] = (m_data[i]);
            j++;
        }

        return result;
    }

    constexpr Mat<T, Row, 1> GetCol(size_t col) const
    {
        Mat<T, Row, 1> result;

        for (size_t i = 0; i < Row; ++i)
        {
            result[i] = (m_data[i * Col + col]);
        }

        return result;
    }

    // 赋值
    constexpr Mat &operator=(const Mat &other) = default;

    constexpr Mat &operator=(Mat &&other) noexcept = default;

    constexpr Mat &operator=(const T &value)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            auto v = simd::set1<T>(value);
            constexpr std::size_t W = simd::SIMDWidth<T>;
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                simd::storeu<T>(&m_data[i], v);
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] = value;
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] = value;
            }
        }
        return *this;
    }

    // 运算

    template <Detail::NumericMat U>
    constexpr friend auto operator+(const Mat<T, Row, Col> &lhs, const Mat<U, Row, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Mat<ResultType, Row, Col> result;
        if constexpr (Detail::CanUseSIMD<T, U> && Detail::MatUseSIMD<ResultType, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<ResultType>;
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<ResultType>(&lhs.m_data[i]);
                auto b = simd::loadu<ResultType>(&rhs.m_data[i]);
                simd::storeu<ResultType>(&result.m_data[i], simd::add<ResultType>(a, b));
            }
            for (; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs[i]);
            }
        }
        return result;
    }

    template <Detail::NumericMat U>
    constexpr friend auto operator-(const Mat<T, Row, Col> &lhs, const Mat<U, Row, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Mat<ResultType, Row, Col> result;
        if constexpr (Detail::CanUseSIMD<T, U> && Detail::MatUseSIMD<ResultType, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<ResultType>;
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<ResultType>(&lhs.m_data[i]);
                auto b = simd::loadu<ResultType>(&rhs.m_data[i]);
                simd::storeu<ResultType>(&result.m_data[i], simd::sub<ResultType>(a, b));
            }
            for (; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs[i]);
            }
        }
        return result;
    }

    template <Detail::NumericMat U, size_t OtherCol>
    constexpr friend auto operator*(const Mat<T, Row, Col> &lhs, const Mat<U, Col, OtherCol> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Mat<ResultType, Row, OtherCol> result;
        for (size_t r = 0; r < Row; ++r)
        {
            for (size_t c = 0; c < OtherCol; ++c)
            {
                ResultType sum = 0;
                for (size_t k = 0; k < Col; ++k)
                {
                    sum += static_cast<ResultType>(lhs[r * Col + k]) * static_cast<ResultType>(rhs[k * OtherCol + c]);
                }
                result[r * OtherCol + c] = sum;
            }
        }
        return result;
    }

    template <Detail::NumericVec U>
    constexpr friend auto operator*(const Mat<T, Row, Col> &lhs, const Vec<U, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, Row> result;
        for (size_t r = 0; r < Row; ++r)
        {
            ResultType sum = 0;
            for (size_t c = 0; c < Col; ++c)
            {
                sum += static_cast<ResultType>(lhs[r * Col + c]) * static_cast<ResultType>(rhs[c]);
            }
            result[r] = sum;
        }
        return result;
    }

    template <Detail::NumericVec U>
    constexpr friend auto operator*(const Vec<T, Row> &lhs, const Mat<U, Row, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Vec<ResultType, Col> result;
        for (size_t c = 0; c < Col; ++c)
        {
            ResultType sum = 0;
            for (size_t r = 0; r < Row; ++r)
            {
                sum += static_cast<ResultType>(lhs[r]) * static_cast<ResultType>(rhs[r * Col + c]);
            }
            result[c] = sum;
        }
        return result;
    }

    constexpr Mat<T, Row, Col> operator-() const
    {
        Mat<T, Row, Col> result;
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            auto z = simd::zero<T>();
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                simd::storeu<T>(&result.m_data[i], simd::sub<T>(z, a));
            }
            for (; i < Row * Col; ++i)
            {
                result[i] = -m_data[i];
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = -m_data[i];
            }
        }
        return result;
    }

    // 复合赋值运算符

    constexpr Mat<T, Row, Col> &operator+=(const Mat<T, Row, Col> &other)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                auto b = simd::loadu<T>(&other.m_data[i]);
                simd::storeu<T>(&m_data[i], simd::add<T>(a, b));
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] += other.m_data[i];
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] += other.m_data[i];
            }
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator+=(const T &value)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            auto sv = simd::set1<T>(value);
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                simd::storeu<T>(&m_data[i], simd::add<T>(a, sv));
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] += value;
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] += value;
            }
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator-=(const Mat<T, Row, Col> &other)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                auto b = simd::loadu<T>(&other.m_data[i]);
                simd::storeu<T>(&m_data[i], simd::sub<T>(a, b));
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] -= other.m_data[i];
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] -= other.m_data[i];
            }
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator-=(const T &value)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            auto sv = simd::set1<T>(value);
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                simd::storeu<T>(&m_data[i], simd::sub<T>(a, sv));
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] -= value;
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] -= value;
            }
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr auto &operator*=(const Mat<U, Col, Col> &rhs)
    {
        static_assert(Row == Col, "operator*= is only supported for square matrices to maintain dimensions.");

        Mat<T, Row, Col> temp;
        for (size_t r = 0; r < Row; ++r)
        {
            for (size_t c = 0; c < Col; ++c)
            {
                T sum = 0;
                for (size_t k = 0; k < Col; ++k)
                {
                    sum += (*this)[r, k] * static_cast<T>(rhs[k, c]);
                }
                temp[r, c] = sum;
            }
        }
        *this = temp;
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr Mat &operator*=(const U &scalar)
    {
        if constexpr (Detail::MatUseSIMD<T, Row, Col>)
        {
            constexpr std::size_t W = simd::SIMDWidth<T>;
            auto sv = simd::set1<T>(static_cast<T>(scalar));
            std::size_t i = 0;
            for (; i + W <= Row * Col; i += W)
            {
                auto a = simd::loadu<T>(&m_data[i]);
                simd::storeu<T>(&m_data[i], simd::mul<T>(a, sv));
            }
            for (; i < Row * Col; ++i)
            {
                m_data[i] = static_cast<T>(m_data[i] * scalar);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                m_data[i] = static_cast<T>(m_data[i] * scalar);
            }
        }
        return *this;
    }

    // 迭代器支持
    auto begin() noexcept { return m_data.begin(); }
    auto end() noexcept { return m_data.end(); }
    auto begin() const noexcept { return m_data.begin(); }
    auto end() const noexcept { return m_data.end(); }

    // 比较操作符 (C++20)
    auto operator<=>(const Mat &other) const = default;

    // 查询方法
    static constexpr size_t Size() { return Row * Col; };

    static constexpr size_t RowSize() { return Row; };

    static constexpr size_t ColSize() { return Col; };

    static constexpr std::tuple<size_t, size_t> Shape() { return std::make_tuple(Row, Col); };

    static const std::type_info &Type() noexcept { return typeid(Mat<T, Row, Col>); }

    static const std::type_info &ValueType() noexcept { return typeid(T); }
};

// 标量与矩阵的混合运算（非友元，避免模板重定义冲突）
template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator+(const Mat<T, Row, Col> &lhs, U rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(rhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto a = simd::loadu<ResultType>(&lhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::add<ResultType>(a, sv));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs);
    }
    return result;
}

template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator+(U lhs, const Mat<T, Row, Col> &rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(lhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto b = simd::loadu<ResultType>(&rhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::add<ResultType>(sv, b));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs) + static_cast<ResultType>(rhs[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs) + static_cast<ResultType>(rhs[i]);
    }
    return result;
}

template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator-(const Mat<T, Row, Col> &lhs, U rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(rhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto a = simd::loadu<ResultType>(&lhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::sub<ResultType>(a, sv));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs);
    }
    return result;
}

template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator-(U lhs, const Mat<T, Row, Col> &rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(lhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto b = simd::loadu<ResultType>(&rhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::sub<ResultType>(sv, b));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs) - static_cast<ResultType>(rhs[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs) - static_cast<ResultType>(rhs[i]);
    }
    return result;
}

template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator*(const Mat<T, Row, Col> &lhs, U rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(rhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto a = simd::loadu<ResultType>(&lhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::mul<ResultType>(a, sv));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs[i]) * static_cast<ResultType>(rhs);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs[i]) * static_cast<ResultType>(rhs);
    }
    return result;
}

template <Detail::NumericMat T, size_t Row, size_t Col, Detail::NumericMat U>
constexpr auto operator*(U lhs, const Mat<T, Row, Col> &rhs)
{
    using ResultType = std::common_type_t<T, U>;
    Mat<ResultType, Row, Col> result;
    if constexpr (Detail::MatUseSIMD<ResultType, Row, Col>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sv = simd::set1<ResultType>(static_cast<ResultType>(lhs));
        std::size_t i = 0;
        for (; i + W <= Row * Col; i += W)
        {
            auto b = simd::loadu<ResultType>(&rhs.m_data[i]);
            simd::storeu<ResultType>(&result.m_data[i], simd::mul<ResultType>(sv, b));
        }
        for (; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs) * static_cast<ResultType>(rhs[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < Row * Col; ++i)
            result[i] = static_cast<ResultType>(lhs) * static_cast<ResultType>(rhs[i]);
    }
    return result;
}

// 常用矩阵类型
using Mat2i = Mat<int, 2, 2>;
using Mat2f = Mat<float, 2, 2>;
using Mat2d = Mat<double, 2, 2>;
using Mat2l = Mat<long, 2, 2>;
using Mat3i = Mat<int, 3, 3>;
using Mat3f = Mat<float, 3, 3>;
using Mat3d = Mat<double, 3, 3>;
using Mat3l = Mat<long, 3, 3>;
using Mat4i = Mat<int, 4, 4>;
using Mat4f = Mat<float, 4, 4>;
using Mat4d = Mat<double, 4, 4>;
using Mat4l = Mat<long, 4, 4>;

// 矩阵视图
template <Detail::NumericMat T, size_t Row, size_t Col, typename RowRange, typename ColRange>
struct MatView final
{
private:
    Mat<T, Row, Col> &_original_mat;
    static constexpr auto _row_indices = RowRange::Values();
    static constexpr auto _col_indices = ColRange::Values();

public:
    // 构造函数
    MatView(Mat<T, Row, Col> &mat, RowRange rr, ColRange rc) : _original_mat(mat) {}

    // 赋值
    template <Detail::NumericMat U>
    constexpr MatView &operator=(const Mat<U, RowRange::Size(), ColRange::Size()> &other)
    {
        for (size_t r = 0; r < RowRange::Size(); ++r)
        {
            for (size_t c = 0; c < ColRange::Size(); ++c)
            {
                _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(other[r, c]);
            }
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr MatView &operator=(const std::initializer_list<U> &ilist)
    {
        if (ilist.size() != RowRange::Size() * ColRange::Size())
        {
            throw std::runtime_error("Size mismatch");
        }
        size_t index = 0;
        for (const auto &value : ilist)
        {
            size_t r = index / ColRange::Size();
            size_t c = index % ColRange::Size();
            _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(value);
            index++;
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr MatView &operator=(const std::array<U, RowRange::Size() * ColRange::Size()> &arr)
    {
        size_t index = 0;
        for (size_t r = 0; r < RowRange::Size(); ++r)
        {
            for (size_t c = 0; c < ColRange::Size(); ++c)
            {
                _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(arr[index++]);
            }
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr MatView &operator=(const std::vector<U> &vec)
    {
        if (vec.Size() != RowRange::Size() * ColRange::Size())
        {
            throw std::runtime_error("Size mismatch");
        }
        size_t index = 0;
        for (size_t r = 0; r < RowRange::Size(); ++r)
        {
            for (size_t c = 0; c < ColRange::Size(); ++c)
            {
                _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(vec[index++]);
            }
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr MatView &operator=(const std::list<U> &list)
    {
        if (list.Size() != RowRange::Size() * ColRange::Size())
        {
            throw std::runtime_error("Size mismatch");
        }
        size_t index = 0;
        for (const auto &value : list)
        {
            size_t r = index / ColRange::Size();
            size_t c = index % ColRange::Size();
            _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(value);
            index++;
        }
        return *this;
    }

    template <Detail::NumericMat U>
    constexpr MatView &operator=(const std::span<U, RowRange::Size() * ColRange::Size()> &span)
    {
        size_t index = 0;
        for (size_t r = 0; r < RowRange::Size(); ++r)
        {
            for (size_t c = 0; c < ColRange::Size(); ++c)
            {
                _original_mat[_row_indices[r], _col_indices[c]] = static_cast<T>(span[index++]);
            }
        }
        return *this;
    }

    //// 隐式转换回Mat
    template <Detail::NumericMat U>
    constexpr operator Mat<U, RowRange::Size(), ColRange::Size()>() const
    {
        Mat<U, RowRange::Size(), ColRange::Size()> result;
        for (size_t r = 0; r < RowRange::Size(); ++r)
        {
            for (size_t c = 0; c < ColRange::Size(); ++c)
            {
                result[r, c] = static_cast<U>(_original_mat[_row_indices[r], _col_indices[c]]);
            }
        }
        return result;
    }
};