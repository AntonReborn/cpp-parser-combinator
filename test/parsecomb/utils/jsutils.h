#pragma once

#include "pc/parsecomb/traits.h"
#include <cstdint>
#include <map>
#include <string>
#include <variant>

namespace js {

template<typename T>
    requires pc::Eq<T>
bool equal(const T &a, const T &b) {
    return a == b;
}

template<typename K, typename V>
bool equal(const std::map<K, V> &a, const std::map<K, V> &b) {
    auto it_a = a.begin();
    auto it_b = b.begin();

    for (; it_a != a.end() && it_b != b.end(); ++it_a, ++it_b) {
        if (!equal(it_a->first, it_b->first))
            return false;

        if (!equal(it_a->second, it_b->second))
            return false;
    }

    return it_a == a.end() && it_b == b.end();
}

template<typename T, typename... VArgs>
bool equal(const T &a, const std::variant<VArgs...> &b) {
    if (!std::holds_alternative<T>(b))
        return false;

    return equal(a, std::get<T>(b));
}

template<typename T, typename... VArgs>
bool equal(const std::variant<VArgs...> &a, const T &b) {
    return equal(b, a);
}
}// namespace js