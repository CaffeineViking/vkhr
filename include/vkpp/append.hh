#ifndef VKPP_APPEND_HH
#define VKPP_APPEND_HH

#include <vector>

namespace vkpp {
    template<typename T> // Super simple way to append values to vector
    void append(const std::vector<T>& values, std::vector<T>& target) {
        target.insert(target.end(), values.begin(), values.end());
    }
}

#endif
