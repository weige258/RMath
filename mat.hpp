#ifndef MAT_HPP
#define MAT_HPP

#include <tuple>
#include <concepts>
#include <array>
#include <cmath>
#include <variant>
#include "vec.hpp"
#include "range.hpp"

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
};

template <Detail::NumericMat T, size_t Row, size_t Col>
struct Mat
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

    constexpr Mat(const T *arr)
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

    template<Detail::NumericVec U>
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

    template<Detail::NumericVec U>
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


    //查询方法
    static constexpr size_t size()  {return Row * Col;};

    static constexpr std::tuple<size_t, size_t> shape()  {return std::make_tuple(Row, Col);};

    static constexpr size_t row_size()  {return Row;};

    static constexpr size_t col_size()  {return Col;};

};

// 矩阵 Hadamard 积 
template <typename... Args>
    requires(sizeof...(Args) >= 2)
constexpr auto Hadamard(const Args&... args) {
    using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;
    
    constexpr size_t R = std::remove_cvref_t<FirstArg>::row_size();
    constexpr size_t C = std::remove_cvref_t<FirstArg>::col_size();

    static_assert((... && (Args::row_size() == R && Args::col_size() == C)), 
                  "All matrices must have the same dimensions.");

    using ResultScalar = std::common_type_t<typename std::remove_cvref_t<Args>::mat_type_alias...>;

    Mat<ResultScalar, R, C> result;

    for (size_t i = 0; i < R * C; ++i) {
        result[i] = (static_cast<ResultScalar>(args[i]) * ...);
    }

    return result;
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



#endif // MAT_HPP
