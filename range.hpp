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

// 范围类
template <Detail::NumericRange T>
struct Range final {
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
        size_t index = 0;
        for (auto value : *this) {
            arr[index++] = static_cast<U>(value);
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

// 静态范围类
template <int Start, int End, int Step = 1>
struct StaticRange final {
    static_assert(Step != 0, "Step cannot be zero");

    // 编译期计算元素个数
    static constexpr int size = []() {
        if constexpr (Step > 0) {
            return (End > Start) ? (End - Start + Step - 1) / Step : 0;
        } else {
            return (End < Start) ? (Start - End - Step - 1) / (-Step) : 0;
        }
    }();

    struct Iterator {
        int current;
        constexpr int operator*() const { return current; }
        constexpr Iterator& operator++() { 
            current += Step; 
            return *this; 
        }
        
        constexpr bool operator!=(const Iterator& other) const { 
            if constexpr (Step > 0) return current < other.current;
            else return current > other.current;
        }
    };

    constexpr Iterator begin() const { return Iterator{Start}; }
    constexpr Iterator end() const { return Iterator{End}; }
};

template<typename T> Range(T, T) -> Range<T>;
template<typename T> Range(T, T, T) -> Range<T>;

#endif