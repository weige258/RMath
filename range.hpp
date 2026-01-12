#pragma once

#include <cstddef>   // size_t
#include <vector>    // std::vector
#include <cstdlib>   // std::abs (int)
#include <stdexcept> // throw



//向量 矩阵范围访问制造器
template <size_t RowOrColSize>
struct Range
{

private:
    size_t start;
    size_t end;
    int step = 1;

public:
    consteval Range(const int &s, const int &e, const int &st = 1)
    {
        if (std::abs(s) > (int)RowOrColSize || std::abs(e) > (int)RowOrColSize)
        {
            throw "Index out of range";
        }
        start = Resolve(s);
        end = Resolve(e);
        step = st;
    }

private:
    static consteval size_t Resolve(int i)
    {
        if (i >= 0)
        {
            return static_cast<size_t>(i);
        };
        return static_cast<size_t>(RowOrColSize + i);
    }

public:
    auto MakeAccessIndices() const
    {
        std::vector<size_t> indices;

        if (step == 0)
        {
            return indices;
        }

        if (step > 0)
        {
            for (size_t i = start; i < end; i += step)
            {
                indices.push_back(i);
            }
        }
        else
        {
            long long current = static_cast<long long>(start);
            long long target = static_cast<long long>(end);
            while (current > target)
            {
                indices.push_back(static_cast<size_t>(current));
                current += step;
            }
        }
        return indices;
    }
};