#ifndef RANGE_HPP
#define RANGE_HPP

#include <concepts>
#include <iterator>
#include <vector>
#include <type_traits>

namespace Detail {
    template <typename T>
    concept NumericRange = std::is_arithmetic_v<T>; 
}

template <Detail::NumericRange T>
struct Range {
private:
//数据
    T _start, _end, _step; 

public:
    //构造
    constexpr Range(T s, T e, T st) : _start(s), _end(e), _step(st) {} 
    constexpr Range(T s, T e) : _start(s), _end(e), _step(static_cast<T>(1)) {} 
    
    // 迭代器
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        T current_value, end_value, step_value;

        constexpr T operator*() const { return current_value; }
        constexpr Iterator& operator++() { current_value += step_value; return *this; }
        
        constexpr bool operator==(const Iterator& other) const {
            if (step_value > 0) return current_value >= end_value; // 正向越界判定
            return current_value <= end_value; // 负向越界判定
        }
        constexpr bool operator!=(const Iterator& other) const { return !(*this == other); }
    };

    constexpr Iterator begin() const { return Iterator{_start, _end, _step}; }
    constexpr Iterator end() const { return Iterator{_end, _end, _step}; }
    
    //数据转换
    template<Detail::NumericRange U,size_t N>
    operator std::array<U,N>() const {
        std::array<U, N> arr;
        for (auto i : *this) {
            arr.push_back(static_cast<U>(i));
        }
        return arr;
    }

    template<Detail::NumericRange U>
    operator std::vector<U>() const {
        std::vector<U> vec;
        for (auto i : *this) {
            vec.push_back(static_cast<U>(i));
        }
        return vec;
    }

    template<Detail::NumericRange U>
    operator std::list<U>() const {
        std::list<U> list;
        for (auto i : *this) {
            list.push_back(static_cast<U>(i));
        }
        return list;
    }

    //查询方法
    constexpr const size_t size() const {
        if (_step == 0) return 0; //
        if (_step > 0 && _start >= _end) return 0; //
        if (_step < 0 && _start <= _end) return 0; //

        auto diff = (_step > 0) ? (_end - _start) : (_start - _end); //
        auto abs_step = (_step > 0) ? _step : -_step; //
        return static_cast<size_t>((diff + abs_step - static_cast<T>(1)) / abs_step); //
    }

    constexpr const size_t size_in_bytes() const {
        return sizeof(T) * size();
    }

    static const std::type_info &type() noexcept { return typeid(Range<T>); }

    static const  std::type_info &value_type() noexcept { return typeid(T); }
};

template<typename T> Range(T, T) -> Range<T>;
template<typename T> Range(T, T, T) -> Range<T>;

#endif