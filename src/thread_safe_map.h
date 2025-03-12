#pragma once 

#include <unordered_map>
#include <optional>
#include <shared_mutex>

template<typename Key, typename Value>
class ThreadSafeMap {
public:
    using KeyType = Key;
    using KeyConstRef = const KeyType&;
    using ValueType = Value;
    using ValueConstRef = const ValueType&;

    std::optional<std::reference_wrapper<const ValueType>> Get(KeyConstRef key) const {
        std::shared_lock lock(mutex_);
        if(const auto it = map_.find(key); it != map_.cend()) {
            return std::cref(it->second);
        }

        return std::nullopt;
    }

    void Add(KeyConstRef key, ValueConstRef value) {
        Emplace(key, value);
    }

    template<typename... Args>
    void Emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        map_.emplace(std::forward<Args>(args)...);
    }

    bool Contains(KeyConstRef key) const {
        std::shared_lock lock(mutex_);
        return map_.count(key) != 0u;
    }

private:
    std::unordered_map<ValueType, KeyType> map_;
    mutable std::shared_mutex mutex_;
};