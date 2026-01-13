#include <concepts>
#include <array>
#include <cmath>
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
    // 构造
    constexpr Mat() { _data.fill(0); }

    constexpr Mat(T num) { _data.fill(num); }

    constexpr Mat(const Mat &other) = default;

    constexpr Mat(Mat &&other) = default;

    template <Detail::NumericMat U>
    constexpr Mat(std::initializer_list<U> list)
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

    constexpr Mat(std::initializer_list<Detail::RowData<T, Col>> list)
    {
        if (list.size() != Row)
            throw std::runtime_error("Row count mismatch");
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

    template <typename U, size_t OtherRow, size_t OtherCol>
    constexpr Mat(Mat<U, OtherRow, OtherCol> &&other)
    {
        static_assert(OtherRow == Row && OtherCol == Col, "Matrix dimension mismatch: Row and Col must be equal for conversion.");
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

    template <typename U>
    constexpr Mat(const std::list<U> &list)
        requires std::convertible_to<U, T>
    {
        if (list.size() != Row * Col)
        {
            throw std::runtime_error("Size mismatch");
        }

        std::copy(list.begin(), list.end(), _data.begin());
    }

    template <typename U>
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

    // 析构
    ~Mat() = default;

    // 数据转化
    template <typename U>
    constexpr Mat(const Mat<U, Row, Col> &other)
    {
        for (size_t i = 0; i < Row * Col; ++i)
        {
            _data[i] = static_cast<T>(other[i]);
        }
    }

    template <typename U>
    constexpr operator std::array<U, Row *Col>() const
        requires std::convertible_to<T, U>
    {
        return std::array<U, Row * Col>(_data.begin(), _data.end());
    }

    template <typename U>
    constexpr operator std::list<U>() const
        requires std::convertible_to<T, U>
    {
        return std::list<U>(_data.begin(), _data.end());
    }

    template <typename U>
    constexpr operator std::vector<U>() const
        requires std::convertible_to<T, U>
    {
        return std::vector<U>(_data.begin(), _data.end());
    }

    template <typename U>
    constexpr operator std::span<U, Row *Col>() const
        requires std::convertible_to<T, U>
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

    //赋值
    constexpr Mat& operator=(const Mat& other) = default;
    
    constexpr Mat& operator=(Mat&& other) = default;
};

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
