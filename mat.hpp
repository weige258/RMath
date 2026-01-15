#include <tuple>
#include <concepts>
#include <array>
#include <cmath>
#include <variant>
#include "vec.hpp"
#include "range.hpp"

#ifndef MAT_HPP
#define MAT_HPP

namespace Detail 
{
    template <typename T>
    concept NumericMat = std::is_arithmetic_v<T>;

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
        consteval NullEmptyPtr(const T *p) : ptr(p)
        {
            if (p == nullptr)
            {
            }
        }
    };
};

template <Detail::NumericMat T, size_t Row, size_t Col>
struct Mat final
{

private:
    std::array<T, Row * Col> _data;

public:
    using mat_type_alias = T;

public:
    // 构造
    constexpr Mat() { _data.fill(0); }

    constexpr Mat(T num) { _data.fill(num); }

    constexpr Mat(const Mat &other) = default;

    constexpr Mat(Mat &&other) = default;

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
            _data[i++] = static_cast<T>(val);
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
                _data[r * Col + c] = row.row_values[c];
            }
            r++;
        }
    }

    template <typename... Args>
    constexpr Mat(const Args &...args)
        requires(sizeof...(args) == Row * Col && (std::convertible_to<Args, T> && ...))
    {
        _data = std::array<T, Row * Col>{static_cast<T>(args)...};
    };

    template <typename U, size_t OtherRow, size_t OtherCol>
    constexpr Mat(const Mat<U, OtherRow, OtherCol> &other)
    {
        static_assert(OtherRow == Row && OtherCol == Col, "Matrix dimension mismatch: Row and Col must be equal for conversion.");
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = static_cast<T>(other[i]);
        }
    }

    template <Detail::NumericMat U>
    constexpr Mat(Mat<U, Row, Col> &&other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = static_cast<T>(other[i]);
        }
    }

    constexpr Mat(const Detail::NullEmptyPtr<T> arr)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = arr[i];
        }
    }

    constexpr Mat(const std::array<T, Row * Col> &arr) : _data(arr) {}

    template <Detail::NumericMat U>
    constexpr Mat(const std::list<U> &list)
        requires std::convertible_to<U, T>
    {
        if (list.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }

        std::copy(list.begin(), list.end(), _data.begin());
    }

    template <Detail::NumericMat U>
    constexpr Mat(const std::vector<U> &vec)
        requires std::convertible_to<U, T>
    {
        if (vec.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }

        std::copy(vec.begin(), vec.end(), _data.begin());
    }

    constexpr Mat(const std::span<T, Row * Col> &span)
    {
        std::copy(span.begin(), span.end(), _data.begin());
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
    constexpr Mat(const Mat<U, Row, Col> &other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = static_cast<T>(other[i]);
        }
    }

    template <Detail::NumericMat U>
    constexpr operator std::array<U, Row *Col>() const
    {
        return std::array<U, Row * Col>(_data.begin(), _data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::list<U>() const
    {
        return std::list<U>(_data.begin(), _data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::vector<U>() const
    {
        return std::vector<U>(_data.begin(), _data.end());
    }

    template <Detail::NumericMat U>
    constexpr operator std::span<U, Row *Col>() const
    {
        return std::span<U, Row * Col>(_data.begin(), _data.end());
    }

    // 指针转换
    explicit operator T *() { return _data._data(); }
    explicit operator const T *() const { return _data._data(); }

    // 访问
    constexpr T &operator[](size_t index)
    {
        return _data[index];
    }

    constexpr const T &operator[](size_t index) const
    {
        return _data[index];
    }

    constexpr T &operator[](size_t row, size_t col)
    {
        return _data[row * Col + col];
    }

    constexpr const T &operator[](size_t row, size_t col) const
    {
        return _data[row * Col + col];
    }

    template <int RStart, int REnd, int RStep,
              int CStart, int CEnd, int CStep>
    constexpr auto operator[](StaticRange<RStart, REnd, RStep> rr,
                              StaticRange<CStart, CEnd, CStep> rc) const
    {
        constexpr size_t OutRow = static_cast<size_t>(decltype(rr)::size);
        constexpr size_t OutCol = static_cast<size_t>(decltype(rc)::size);
        Mat<T, OutRow, OutCol> result;

        if constexpr (OutRow > 0)
        {
            constexpr int last_r = RStart + (static_cast<int>(OutRow) - 1) * RStep;
            static_assert(RStart >= 0 && RStart < (int)Row, "StaticRange Row Start out of bounds.");
            static_assert(last_r >= 0 && last_r < (int)Row, "StaticRange Row sequence exceeds Matrix dimensions.");
        }

        if constexpr (OutCol > 0)
        {
            constexpr int last_c = CStart + (static_cast<int>(OutCol) - 1) * CStep;
            static_assert(CStart >= 0 && CStart < (int)Col, "StaticRange Col Start out of bounds.");
            static_assert(last_c >= 0 && last_c < (int)Col, "StaticRange Col sequence exceeds Matrix dimensions.");
        }

        size_t out_r = 0;
        for (auto r_idx : rr)
        {
            size_t out_c = 0;
            for (auto c_idx : rc)
            {
                result[out_r, out_c] = (*this)[r_idx, c_idx];
                out_c++;
            }
            out_r++;
        }

        return result;
    }

    constexpr Mat<T, 1, Col> GetRow(size_t row) const
    {
        Mat<T, 1, Col> result;

        size_t j = 0;
        for (size_t i = row * Col; i < row * Col + Col; ++i)
        {
            result[j] = (_data[i]);
            j++;
        }

        return result;
    }

    constexpr Mat<T, Row, 1> GetCol(size_t col) const
    {
        Mat<T, Row, 1> result;

        for (size_t i = 0; i < Row; ++i)
        {
            result[i] = (_data[i * Col + col]);
        }

        return result;
    }

    // 赋值
    constexpr Mat &operator=(const Mat &other) = default;

    constexpr Mat &operator=(Mat &&other) = default;

    constexpr Mat &operator=(const T &value)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = value;
        }
        return *this;
    }

    // 运算

    template <Detail::NumericMat U>
    constexpr friend auto operator+(const Mat<T, Row, Col> &lhs, const Mat<U, Row, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Mat<ResultType, Row, Col> result;
        for (size_t i = 0; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Mat<T, Row, Col>> && Detail::NumericMat<RType>) ||
            (Detail::NumericMat<LType> && std::same_as<RType, Mat<T, Row, Col>>))
    constexpr friend auto operator+(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Mat<ResultType, Row, Col> result;

        if constexpr (std::same_as<LType, Mat<T, Row, Col>>)
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) + static_cast<ResultType>(rhs);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs) + static_cast<ResultType>(rhs[i]);
            }
        }
        return result;
    }

    template <Detail::NumericMat U>
    constexpr friend auto operator-(const Mat<T, Row, Col> &lhs, const Mat<U, Row, Col> &rhs)
    {
        using ResultType = std::common_type_t<T, U>;
        Mat<ResultType, Row, Col> result;
        for (size_t i = 0; i < Row * Col; ++i)
        {
            result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs[i]);
        }
        return result;
    }

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Mat<T, Row, Col>> && Detail::NumericMat<RType>) ||
            (Detail::NumericMat<LType> && std::same_as<RType, Mat<T, Row, Col>>))
    constexpr friend auto operator-(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Mat<ResultType, Row, Col> result;

        if constexpr (std::same_as<LType, Mat<T, Row, Col>>)
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) - static_cast<ResultType>(rhs);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs) - static_cast<ResultType>(rhs[i]);
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

    template <typename LType, typename RType>
        requires(
            (std::same_as<LType, Mat<T, Row, Col>> && Detail::NumericMat<RType>) ||
            (Detail::NumericMat<LType> && std::same_as<RType, Mat<T, Row, Col>>))
    constexpr friend auto operator*(const LType &lhs, const RType &rhs)
    {
        using ScalarType = std::conditional_t<Detail::NumericVec<LType>, LType, RType>;
        using ResultType = std::common_type_t<T, ScalarType>;

        Mat<ResultType, Row, Col> result;

        if constexpr (std::same_as<LType, Mat<T, Row, Col>>)
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs[i]) * static_cast<ResultType>(rhs);
            }
        }
        else
        {
            for (size_t i = 0; i < Row * Col; ++i)
            {
                result[i] = static_cast<ResultType>(lhs) * static_cast<ResultType>(rhs[i]);
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
        for (size_t i = 0; i < Row * Col; ++i)
        {
            result[i] = -_data[i];
        }
        return result;
    }

    // 复合赋值运算符

    constexpr Mat<T, Row, Col> &operator+=(const Mat<T, Row, Col> &other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] += other._data[i];
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator+=(const T &value)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] += value;
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator-=(const Mat<T, Row, Col> &other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] -= other._data[i];
        }
        return *this;
    }

    constexpr Mat<T, Row, Col> &operator-=(const T &value)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] -= value;
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
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = static_cast<T>(_data[i] * scalar);
        }
        return *this;
    }

    // 迭代器支持
    auto begin() noexcept { return _data.begin(); }
    auto end() noexcept { return _data.end(); }
    auto begin() const noexcept { return _data.begin(); }
    auto end() const noexcept { return _data.end(); }

    // 比较操作符 (C++20)
    auto operator<=>(const Mat &other) const = default;

    // 查询方法
    static constexpr size_t size() { return Row * Col; };

    static constexpr size_t row_size() { return Row; };

    static constexpr size_t col_size() { return Col; };

    static constexpr std::tuple<size_t, size_t> shape() { return std::make_tuple(Row, Col); };

    static const std::type_info &type() noexcept { return typeid(Mat<T, Row, Col>); }

    static const std::type_info &value_type() noexcept { return typeid(T); }
};

// 向量与矩阵乘法
template <Detail::NumericMat T,Detail::NumericMat U, size_t N>
    constexpr auto &operator*=(Vec<U, N> &lhs, const Mat<T, N, N> &rhs)
    {
        Vec<T, N> temp;
        for (size_t c = 0; c < N; ++c)
        {
            T sum = 0;
            for (size_t r = 0; r < N; ++r)
            {
                sum += (lhs)[r] * static_cast<T>(rhs[r, c]);
            }
            temp[c] = sum;
        }
        lhs = temp;
        return lhs;
    }

// 矩阵 Hadamard 积
template <typename... Args>
    requires(sizeof...(Args) >= 2) &&
            (... && requires { typename std::remove_cvref_t<Args>::mat_type_alias; })
constexpr auto Hadamard(const Args &...args)
{
    using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;

    constexpr size_t R = std::remove_cvref_t<FirstArg>::row_size();
    constexpr size_t C = std::remove_cvref_t<FirstArg>::col_size();

    static_assert((... && (Args::row_size() == R && Args::col_size() == C)),
                  "All matrices must have the same dimensions.");

    using ResultScalar = std::common_type_t<typename std::remove_cvref_t<Args>::mat_type_alias...>;

    Mat<ResultScalar, R, C> result;

    for (size_t i = 0; i < R * C; ++i)
    {
        result[i] = (static_cast<ResultScalar>(args[i]) * ...);
    }

    return result;
}

// 矩阵克罗内积

template <Detail::NumericMat T, Detail::NumericMat U,
          size_t Row1, size_t Col1, size_t Row2, size_t Col2>
constexpr auto KroneckerProduct(const Mat<T, Row1, Col1> &lhs,
                                const Mat<U, Row2, Col2> &rhs)
{
    using ResultType = std::common_type_t<T, U>;
    constexpr size_t ResultRow = Row1 * Row2;
    constexpr size_t ResultCol = Col1 * Col2;

    Mat<ResultType, ResultRow, ResultCol> result;

    for (size_t i = 0; i < Row1; ++i)
    {
        for (size_t j = 0; j < Col1; ++j)
        {
            T scalar = lhs[i, j];
            for (size_t k = 0; k < Row2; ++k)
            {
                for (size_t l = 0; l < Col2; ++l)
                {
                    result[i * Row2 + k, j * Col2 + l] =
                        static_cast<ResultType>(scalar) *
                        static_cast<ResultType>(rhs[k, l]);
                }
            }
        }
    }
    return result;
}

template <typename T, typename... Args>
constexpr auto Kronecker(const T &first, const Args &...rest)
{
    if constexpr (sizeof...(rest) == 0)
    {
        return first;
    }
    else
    {
        return KroneckerProduct(first, Kronecker(rest...));
    }
}

// 转置
template <Detail::NumericMat T, size_t Row, size_t Col>
constexpr auto Transpose(const Mat<T, Row, Col> &mat)
{
    Mat<T, Col, Row> result;
    for (size_t r = 0; r < Row; ++r)
    {
        for (size_t c = 0; c < Col; ++c)
        {
            result[c, r] = mat[r, c];
        }
    }
    return result;
}

// 辅助函数：获取子矩阵 (用于计算余子式)
template <Detail::NumericMat T, size_t Row, size_t Col>
constexpr auto MinorMatrix(const Mat<T, Row, Col> &mat, size_t omitRow, size_t omitCol)
{
    static_assert(Row > 1 && Col > 1, "Cannot get minor of a 1x1 matrix.");
    Mat<T, Row - 1, Col - 1> result;
    size_t rr = 0;
    for (size_t r = 0; r < Row; ++r)
    {
        if (r == omitRow)
            continue;
        size_t cc = 0;
        for (size_t c = 0; c < Col; ++c)
        {
            if (c == omitCol)
                continue;
            result[rr, cc] = mat[r, c];
            cc++;
        }
        rr++;
    }
    return result;
}

// 行列式取值
template <Detail::NumericMat T, size_t Size>
constexpr auto Det(const Mat<T, Size, Size> &mat)
{
    if constexpr (Size == 1)
    {
        return mat[0];
    }
    else if constexpr (Size == 2)
    {
        return mat[0, 0] * mat[1, 1] - mat[0, 1] * mat[1, 0];
    }
    else
    {
        auto det = static_cast<T>(0);
        int sign = 1;
        // 沿第一行展开
        for (size_t j = 0; j < Size; ++j)
        {
            det += sign * mat[0, j] * Det(MinorMatrix(mat, 0, j));
            sign = -sign;
        }
        return det;
    }
}

// --- 代数余子式 (Cofactor) ---
template <Detail::NumericMat T, size_t Size>
constexpr auto Cofactor(const Mat<T, Size, Size> &mat, size_t row, size_t col)
{
    auto minorDet = Det(MinorMatrix(mat, row, col));
    return ((row + col) % 2 == 0) ? minorDet : -minorDet;
}

// --- 伴随矩阵 (Adjoint) ---
template <Detail::NumericMat T, size_t Size>
constexpr auto Adjoint(const Mat<T, Size, Size> &mat)
{
    if constexpr (Size == 1)
    {
        return Mat<T, 1, 1>{1};
    }
    Mat<T, Size, Size> adj;
    for (size_t r = 0; r < Size; ++r)
    {
        for (size_t c = 0; c < Size; ++c)
        {
            adj[c, r] = Cofactor(mat, r, c);
        }
    }
    return adj;
}

// 逆矩阵
template <Detail::NumericMat T, size_t Size>
constexpr auto Inverse(const Mat<T, Size, Size> &mat)
{
    auto det = Det(mat);
    
    if (std::abs(det) < 1e-9)
    {
        throw std::runtime_error("Matrix is singular and cannot be inverted.");
    }
    return Adjoint(mat) * (static_cast<T>(1) / det);
}

// 矩阵的迹
template <Detail::NumericMat T, size_t Size>
constexpr auto Trace(const Mat<T, Size, Size> &mat)
{
    auto trace = 0;
    for (size_t i = 0; i < Size; ++i)
    {
        trace += mat[i, i];
    }
    return trace;
}

// 矩阵的秩
template <Detail::NumericMat T, size_t Row, size_t Col>
constexpr size_t Rank(const Mat<T, Row, Col> &mat)
{
    auto temp = mat;
    size_t rank = 0;
    std::vector<bool> row_used(Row, false);

    for (size_t i = 0; i < Col && rank < Row; ++i)
    {
        size_t pivot = Row;
        for (size_t j = 0; j < Row; ++j)
        {
            if (!row_used[j] && std::abs(temp[j, i]) > 1e-9)
            {
                pivot = j;
                break;
            }
        }

        if (pivot != Row)
        {
            row_used[pivot] = true;
            rank++;
            for (size_t j = 0; j < Row; ++j)
            {
                if (!row_used[j])
                {
                    T factor = temp[j, i] / temp[pivot, i];
                    for (size_t k = i; k < Col; ++k)
                    {
                        temp[j, k] -= factor * temp[pivot, k];
                    }
                }
            }
        }
    }
    return rank;
}

// 满秩判断
template <Detail::NumericMat T, size_t Size>
constexpr bool IsFullRank(const Mat<T, Size, Size> &mat)
{
    if (Det(mat) != 0 && Rank(mat) == Size)
        return true;
    else
        return false;
}

// 输出运算符
template <Detail::NumericMat T, size_t Row, size_t Col>
std::ostream &operator<<(std::ostream &os, const Mat<T, Row, Col> &mat)
{
    os << "[";
    for (size_t r = 0; r < Row; ++r)
    {
        if (r > 0)
            os << " ";
        for (size_t c = 0; c < Col; ++c)
        {
            os << mat[r * Col + c];
            if (c < Col - 1)
            {
                os << ", ";
            }
        }
        if (r < Row - 1)
        {
            os << ",\n";
        }
    }
    os << "] \n";
    return os;
}

//常用矩阵类型
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

#endif // MAT_HPP