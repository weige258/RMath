#pragma once

#include "vec.hpp"
#include "mat.hpp"

// ===================== Vec 计算函数 =====================

// 模长
template <Detail::NumericVec T, std::size_t N>
T Length(const Vec<T, N> &v)
{
    if constexpr (Detail::VecUseSIMD<T, N> && !std::is_integral_v<T>)
    {
        constexpr std::size_t W = simd::SIMDWidth<T>;
        auto sum_vec = simd::zero<T>();
        std::size_t i = 0;
        for (; i + W <= N; i += W)
        {
            auto a = simd::loadu<T>(&v[i]);
            sum_vec = simd::fmadd<T>(a, a, sum_vec);
        }
        T sum = simd::hadd<T>(sum_vec);
        for (; i < N; ++i)
        {
            sum += v[i] * v[i];
        }
        return std::sqrt(sum);
    }
    else
    {
        T sum = 0;
        for (size_t i = 0; i < N; ++i)
        {
            sum += v[i] * v[i];
        }
        return std::sqrt(sum);
    }
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
    constexpr std::size_t N = (std::tuple_element_t<0, std::tuple<Vecs...>>::Size());
    static_assert(((vecs.Size() == N) && ...), "All vectors must have the same dimension N");
    using ResultType = std::common_type_t<typename Vecs::vec_type_alias...>;

    if constexpr (sizeof...(Vecs) == 2 && Detail::VecUseSIMD<ResultType, N>)
    {
        constexpr std::size_t W = simd::SIMDWidth<ResultType>;
        auto sum_vec = simd::zero<ResultType>();
        std::size_t i = 0;
        auto tuple_vecs = std::forward_as_tuple(vecs...);
        const auto &v1 = std::get<0>(tuple_vecs);
        const auto &v2 = std::get<1>(tuple_vecs);
        for (; i + W <= N; i += W)
        {
            auto va = simd::loadu<ResultType>(&v1[i]);
            auto vb = simd::loadu<ResultType>(&v2[i]);
            sum_vec = simd::fmadd<ResultType>(va, vb, sum_vec);
        }
        ResultType total_sum = simd::hadd<ResultType>(sum_vec);
        for (; i < N; ++i)
        {
            total_sum += static_cast<ResultType>(v1[i]) * static_cast<ResultType>(v2[i]);
        }
        return total_sum;
    }
    else
    {
        ResultType total_sum = 0;
        for (std::size_t i = 0; i < N; ++i)
        {
            total_sum += (static_cast<ResultType>(vecs[i]) * ...);
        }
        return total_sum;
    }
}

// 叉积
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
constexpr auto Cross(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    requires(N == 2 || N == 3 || N == 7)
{
    using ResultType = std::common_type_t<T, U>;

    if constexpr (N == 2) {
        return static_cast<ResultType>(lhs[0]) * rhs[1] - static_cast<ResultType>(lhs[1]) * rhs[0];
    }
    else if constexpr (N == 3) {
        return Vec<ResultType, 3>{
            static_cast<ResultType>(lhs[1]) * rhs[2] - static_cast<ResultType>(lhs[2]) * rhs[1],
            static_cast<ResultType>(lhs[2]) * rhs[0] - static_cast<ResultType>(lhs[0]) * rhs[2],
            static_cast<ResultType>(lhs[0]) * rhs[1] - static_cast<ResultType>(lhs[1]) * rhs[0]
        };
    }
    else if constexpr (N == 7) {
        Vec<ResultType, 7> res;
        res[0] = lhs[1]*rhs[3] - lhs[3]*rhs[1] + lhs[2]*rhs[6] - lhs[6]*rhs[2] + lhs[4]*rhs[5] - lhs[5]*rhs[4];
        res[1] = lhs[2]*rhs[4] - lhs[4]*rhs[2] + lhs[3]*rhs[0] - lhs[0]*rhs[3] + lhs[5]*rhs[6] - lhs[6]*rhs[5];
        res[2] = lhs[3]*rhs[5] - lhs[5]*rhs[3] + lhs[4]*rhs[1] - lhs[1]*rhs[4] + lhs[6]*rhs[0] - lhs[0]*rhs[6];
        res[3] = lhs[4]*rhs[6] - lhs[6]*rhs[4] + lhs[5]*rhs[2] - lhs[2]*rhs[5] + lhs[0]*rhs[1] - lhs[1]*rhs[0];
        res[4] = lhs[5]*rhs[0] - lhs[0]*rhs[5] + lhs[6]*rhs[3] - lhs[3]*rhs[6] + lhs[1]*rhs[2] - lhs[2]*rhs[1];
        res[5] = lhs[6]*rhs[1] - lhs[1]*rhs[6] + lhs[0]*rhs[4] - lhs[4]*rhs[0] + lhs[2]*rhs[3] - lhs[3]*rhs[2];
        res[6] = lhs[0]*rhs[2] - lhs[2]*rhs[0] + lhs[1]*rhs[5] - lhs[5]*rhs[1] + lhs[3]*rhs[4] - lhs[4]*rhs[3];
        return res;
    }
}

// 叉积运算符
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
constexpr auto operator^(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
    requires(N == 2 || N == 3 || N == 7)
{
    return Cross(lhs, rhs);
}

// 向量 Hadamard 积 (按元素相乘)
template <typename... Args>
    requires(sizeof...(Args) >= 2) &&
            (... && requires { typename std::remove_cvref_t<Args>::vec_type_alias; })
constexpr auto Hadamard(const Args &...args)
{

    using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;
    constexpr size_t N = std::remove_cvref_t<FirstArg>::Size();

    static_assert((... && (Args::Size() == N)),
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
    constexpr std::size_t TotalN = (Vecs::Size() + ...);
    using ResultT = std::common_type_t<typename Vecs::vec_type_alias...>;
    Vec<ResultT, TotalN> result;
    std::size_t offset = 0;
    ([&](const auto &v)
     {
        for (std::size_t i = 0; i < v.Size(); ++i) {
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

// 投影 Project
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
auto Project(const Vec<T, N> &a, const Vec<U, N> &b)
{
    using ResultT = std::common_type_t<T, U>;
    auto dot_val = Dot(a, b);
    auto b_mag_sq = Dot(b, b);
    return Vec<ResultT, N>(b) * (static_cast<ResultT>(dot_val) / static_cast<ResultT>(b_mag_sq));
}

// 反射 Reflect
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
auto Reflect(const Vec<T, N> &a, const Vec<U, N> &n)
{
    using ResultT = std::common_type_t<T, U>;
    auto dot_an = Dot(a, n);
    auto dot_nn = Dot(n, n);
    return Vec<ResultT, N>(a) - Vec<ResultT, N>(n) * (static_cast<ResultT>(2) * static_cast<ResultT>(dot_an) / static_cast<ResultT>(dot_nn));
}

// 计算两个向量之间的弧度
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
constexpr double Radian(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
{
    double dot = static_cast<double>(Dot(lhs , rhs));
    double len_product = Length(lhs) * Length(rhs);

    if (len_product < 1e-9)
        return 0.0;

    double cos_theta = dot / len_product;

    if (cos_theta > 1.0)
        cos_theta = 1.0;
    if (cos_theta < -1.0)
        cos_theta = -1.0;

    return std::acos(cos_theta);
}

// 计算两个向量之间的角度 
template <Detail::NumericVec T, Detail::NumericVec U, std::size_t N>
constexpr double Degree(const Vec<T, N> &lhs, const Vec<U, N> &rhs)
{

    double dot = 0.0;
    for (size_t i = 0; i < N; ++i) {
        dot += static_cast<double>(lhs[i]) * static_cast<double>(rhs[i]);
    }

    double len_lhs_sq = 0.0;
    double len_rhs_sq = 0.0;
    for (size_t i = 0; i < N; ++i) {
        len_lhs_sq += static_cast<double>(lhs[i]) * static_cast<double>(lhs[i]);
        len_rhs_sq += static_cast<double>(rhs[i]) * static_cast<double>(rhs[i]);
    }
    double len_product = std::sqrt(len_lhs_sq) * std::sqrt(len_rhs_sq);

    if (len_product < 1e-9)
        return 0.0;

    double cos_theta = dot / len_product;
    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;

    return std::acos(cos_theta) * (180.0 / std::numbers::pi);
}

// ===================== Mat 计算函数 =====================

// 向量与矩阵乘法
template <Detail::NumericMat T, Detail::NumericMat U, size_t N>
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

    constexpr size_t R = std::remove_cvref_t<FirstArg>::RowSize();
    constexpr size_t C = std::remove_cvref_t<FirstArg>::ColSize();

    static_assert((... && (Args::RowSize() == R && Args::ColSize() == C)),
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
        return mat[0];
    if constexpr (Size == 2)
        return mat[0, 0] * mat[1, 1] - mat[0, 1] * mat[1, 0];

    auto temp = mat;
    T det = 1;

    for (size_t i = 0; i < Size; ++i)
    {
        // 寻找主元
        size_t pivot = i;
        for (size_t j = i + 1; j < Size; ++j)
        {
            if (std::abs(temp[j, i]) > std::abs(temp[pivot, i]))
                pivot = j;
        }

        if (Detail::IsNearZero(temp[pivot, i]))
            return static_cast<T>(0);

        if (pivot != i)
        {
            // 交换行，行列式变号
            for (size_t k = i; k < Size; ++k)
                std::swap(temp[i, k], temp[pivot, k]);
            det *= -1;
        }

        det *= temp[i, i];

        for (size_t j = i + 1; j < Size; ++j)
        {
            T factor = temp[j, i] / temp[i, i];
            for (size_t k = i + 1; k < Size; ++k)
            {
                temp[j, k] -= factor * temp[i, k];
            }
        }
    }
    return det;
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

    if (Detail::IsNearZero(det))
    {
        throw std::runtime_error("Matrix is singular and cannot be inverted.");
    }
    return Adjoint(mat) * (static_cast<T>(1) / det);
}

// 矩阵的迹
template <Detail::NumericMat T, size_t Size>
constexpr auto Trace(const Mat<T, Size, Size> &mat)
{
    T trace = 0;
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
            if (!row_used[j] && !Detail::IsNearZero(temp[j, i]))
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
    return Rank(mat) == Size;
}

// 设置视图矩阵 (4x4)
template <Detail::NumericMat T>
constexpr void SetViewMatrix(Mat<T, 4, 4> &mat, const Vec<T, 3> &camera_pos, const Vec<T, 3> &camera_direction, const Vec<T, 3> &camera_up)
{
    Mat<T, 4, 4> rotation_matrix = Mat<T, 4, 4>::MakeIdentity();
    Mat<T, 4, 4> translation_matrix = Mat<T, 4, 4>::MakeIdentity();

    Vec<T, 3> front = Normalize(camera_direction);
    Vec<T, 3> right = Normalize(camera_direction ^ camera_up);
    Vec<T, 3> up = Normalize(right ^ camera_direction);

    rotation_matrix[0, 0] = right[0];
    rotation_matrix[0, 1] = right[1];
    rotation_matrix[0, 2] = right[2];
    rotation_matrix[1, 0] = up[0];
    rotation_matrix[1, 1] = up[1];
    rotation_matrix[1, 2] = up[2];
    rotation_matrix[2, 0] = front[0];
    rotation_matrix[2, 1] = front[1];
    rotation_matrix[2, 2] = front[2];

    translation_matrix[3, 0] = -camera_pos[0];
    translation_matrix[3, 1] = -camera_pos[1];
    translation_matrix[3, 2] = -camera_pos[2];

    mat = rotation_matrix * translation_matrix;
}

// 设置2D视图矩阵 (4x4)
template <Detail::NumericMat T>
constexpr void SetViewMatrix(Mat<T, 4, 4> &mat, const Vec<T, 2> &pos, T rotation_radians, T zoom)
{
    mat = Mat<T, 4, 4>{};

    T cos_r = std::cos(rotation_radians);
    T sin_r = std::sin(rotation_radians);

    mat[0, 0] = cos_r * zoom;
    mat[0, 1] = sin_r * zoom;
    mat[0, 3] = -(pos[0] * cos_r + pos[1] * sin_r) * zoom;

    mat[1, 0] = -sin_r * zoom;
    mat[1, 1] = cos_r * zoom;
    mat[1, 3] = (pos[0] * sin_r - pos[1] * cos_r) * zoom;

    mat[2, 2] = 1;

    mat[3, 3] = 1;
}

// 设置透视投影矩阵 (4x4)
template <Detail::NumericMat T>
constexpr void SetProjectionMatrix(Mat<T, 4, 4> &mat, T fov_radians, T aspect, T near, T far)
{
    mat = Mat<T, 4, 4>{};

    T tanHalfFov = std::tan(fov_radians / static_cast<T>(2));

    mat[0, 0] = static_cast<T>(1) / (aspect * tanHalfFov);
    mat[1, 1] = static_cast<T>(1) / tanHalfFov;
    mat[2, 2] = -(far + near) / (far - near);
    mat[2, 3] = -(static_cast<T>(2) * far * near) / (far - near);
    mat[3, 2] = static_cast<T>(-1);
}

// 设置正交投影矩阵 (4x4)
template <Detail::NumericMat T>
constexpr void SetProjectionMatrix(Mat<T, 4, 4> &mat, T left, T right, T bottom, T top, T near, T far)
{
    mat = Mat<T, 4, 4>{};

    mat[0, 0] = static_cast<T>(2) / (right - left);
    mat[1, 1] = static_cast<T>(2) / (top - bottom);
    mat[2, 2] = -static_cast<T>(2) / (far - near);

    mat[0, 3] = -(right + left) / (right - left);
    mat[1, 3] = -(top + bottom) / (top - bottom);
    mat[2, 3] = -(far + near) / (far - near);
    mat[3, 3] = static_cast<T>(1);
}

// 设置简单正交投影矩阵 (4x4)
template <Detail::NumericMat T>
constexpr void SetProjectionMatrix(Mat<T, 4, 4> &mat, T width, T height)
{
    T half_w = width / static_cast<T>(2);
    T half_h = height / static_cast<T>(2);

    SetProjectionMatrix(mat, -half_w, half_w, half_h, -half_h, static_cast<T>(-1), static_cast<T>(1));
}

// ===================== 输出运算符 =====================

// Vec 输出运算符
template <Detail::NumericVec T, std::size_t N>
std::ostream &operator<<(std::ostream &os, const Vec<T, N> &v)
{
    os << "[";
    for (size_t i = 0; i < N; ++i)
    {
        os << v[i];
        if (i + 1 < N)
            os << ", ";
    }
    os << "]";
    return os;
}

// VecView 输出运算符
template <Detail::NumericVec T, std::size_t N, int start, int end, int step>
constexpr std::ostream &operator<<(std::ostream &os, const VecView<T, N, start, end, step> &view)
{
    os << Vec<T, Range<start, end, step>::Size()>(view);
    return os;
}

// Mat 输出运算符
template <Detail::NumericMat T, size_t Row, size_t Col>
std::ostream &operator<<(std::ostream &os, const Mat<T, Row, Col> &mat)
{
    for (size_t r = 0; r < Row; ++r)
    {
        os << "[";
        for (size_t c = 0; c < Col; ++c)
        {
            if (mat[r, c] >= 0)
            {
                os << " ";
            }
            os << mat[r, c];
            if (c + 1 < Col)
            {
                os << ", ";
            }
        }
        os << "]";
        if (r + 1 < Row)
            os << "\n";
    }

    return os;
}

// MatView 输出运算符
template <Detail::NumericMat T, size_t Row, size_t Col, typename RowRange, typename ColRange>
std::ostream &operator<<(std::ostream &os, const MatView<T, Row, Col, RowRange, ColRange> &mat_view)
{
    os << Mat<T, RowRange::Size(), ColRange::Size()>(mat_view);
    return os;
}