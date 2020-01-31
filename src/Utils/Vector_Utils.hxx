#pragma once

#include <vector>

namespace tablator {

//=====================================================================

// const version
template <typename S, typename T>
inline typename std::vector<std::pair<S, T> >::const_iterator pair_vec_find_if(
        const std::vector<std::pair<S, T> > &pair_vec, const S &key) {
    auto iter = std::find_if(pair_vec.begin(), pair_vec.end(),
                             [&](const std::pair<S, T> &p) { return p.first == key; });
    return iter;
}

// non-const version
template <typename S, typename T>
inline typename std::vector<std::pair<S, T> >::iterator pair_vec_find_if(
        std::vector<std::pair<S, T> > &pair_vec, const S &key) {
    auto iter = std::find_if(pair_vec.begin(), pair_vec.end(),
                             [&](const std::pair<S, T> &p) { return p.first == key; });
    return iter;
}

//=====================================================================

template <typename S, typename T>
inline typename std::vector<std::pair<S, T> >::iterator pair_vec_find_or_create(
        std::vector<std::pair<S, T> > &pair_vec, const S &key) {
    auto iter = std::find_if(pair_vec.begin(), pair_vec.end(),
                             [&](const std::pair<S, T> &p) { return p.first == key; });
    if (iter == pair_vec.end()) {
        T value;
        pair_vec.emplace_back(std::make_pair(key, value));
        iter = pair_vec.begin();
        std::advance(iter, pair_vec.size() - 1);
    }
    return iter;
}

//=====================================================================

// const version
template <typename T>
inline size_t vec_find(const std::vector<T> &vec, const T &val) {
    auto iter = std::find(vec.begin(), vec.end(), val);
    return distance(vec.begin(), iter);
}

//=====================================================================

// const version
template <typename T>
inline size_t vec_find_or_bust(const std::vector<T> &vec, const T &val) {
    auto iter = std::find(vec.begin(), vec.end(), val);
    if (iter == vec.end()) {
        throw std::runtime_error("Unknown value " + val);
    }
    return distance(vec.begin(), iter);
}

}  // namespace tablator
