#ifndef RANGE_HPP
#define RANGE_HPP

#include <concepts>
#include <iterator>
#include <vector>
#include <type_traits>

// 静态范围类
template <int Start, int End, int Step = 1>
struct Range final
{
    static_assert(Step != 0, "Step cannot be zero");

    // 编译期计算元素个数
    static constexpr size_t Size()
    {
        if constexpr (Step > 0)
        {
            return (End > Start) ? (End - Start + Step - 1) / Step : 0;
        }
        else
        {
            return (End < Start) ? (Start - End - Step - 1) / (-Step) : 0;
        }
    };
    
    // 迭代器支持
    struct Iterator
    {
        int current;
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;

        constexpr int operator*() const { return current; }
        
        constexpr Iterator &operator++()
        {
            current += Step;
            return *this;
        }

        constexpr bool operator!=(const Iterator &other) const
        {
            if constexpr (Step > 0)
                return current < other.current;
            else
                return current > other.current;
        }
    };
    
    constexpr Iterator begin() const { return Iterator{Start}; }
    constexpr Iterator end() const { return Iterator{End}; }

    // 索引访问
    constexpr int operator[](const size_t &index) const
    {
        if (index >= static_cast<std::size_t>(Size()))
        {
            throw std::out_of_range("Range index out of bounds!");
        }
        return Start + static_cast<int>(index) * Step;
    }
    
    //值访问
    static constexpr std::array<int, Size()> Values() {
        return []<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::array<int, Size()>{ (Start + static_cast<int>(Is) * Step)... };
        }(std::make_index_sequence<Size()>{});
    }

    constexpr operator std::vector<int>() const {
        return []<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::vector<int>{ (Start + static_cast<int>(Is) * Step)... };
        }(std::make_index_sequence<Size()>{});
    }

    constexpr operator std::list<int>() const {
        std::list<int> result;
        for (int val : *this) {
            result.push_back(val);
        }
        return result;
    }

    operator std::array<int, Size()>(){
        return []<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::array<int, Size()>{ (Start + static_cast<int>(Is) * Step)... };
        }(std::make_index_sequence<Size()>{});
    }

    
};

#endif