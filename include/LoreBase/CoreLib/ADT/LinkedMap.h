#ifndef LORE_BASE_CORELIB_LINKEDMAP_H
#define LORE_BASE_CORELIB_LINKEDMAP_H

#include <list>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstddef>

namespace lore {

    template <class K, class V, class ListType = std::list<std::pair<const K, V>>,
              class MapType = std::unordered_map<K, V>>
    class LinkedMap {
    private:
        ListType _list;
        MapType _map;

    public:
        using key_type = K;
        using mapped_type = V;
        using value_type = typename ListType::value_type;
        using size_type = size_t;
        using difference_type = typename ListType::difference_type;
        using allocator_type = typename MapType::allocator_type;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = typename ListType::pointer;
        using const_pointer = typename ListType::const_pointer;

        LinkedMap() = default;

        LinkedMap(const LinkedMap &RHS) {
            for (const auto &item : RHS._list) {
                append(item.first, item.second);
            }
        }

        LinkedMap(LinkedMap &&RHS) noexcept {
            _list = std::move(RHS._list);
            _map = std::move(RHS._map);
        }

        LinkedMap &operator=(const LinkedMap &RHS) {
            clear();
            for (const auto &item : RHS._list) {
                append(item.first, item.second);
            }
            return *this;
        }

        LinkedMap &operator=(LinkedMap &&RHS) noexcept {
            _list = std::move(RHS._list);
            _map = std::move(RHS._map);
            return *this;
        }

        LinkedMap(std::initializer_list<std::pair<K, V>> list)
            : LinkedMap(list.begin(), list.end()) {
        }

        template <typename InputIterator>
        LinkedMap(InputIterator f, InputIterator l) {
            for (; f != l; ++f)
                append(f.key(), f.value());
        }

        inline bool operator==(const LinkedMap &RHS) const {
            return _list == RHS._list;
        }

        inline bool operator!=(const LinkedMap &RHS) const {
            return _list != RHS._list;
        }

        void swap(LinkedMap &RHS) noexcept {
            std::swap(_list, RHS._list);
            std::move(_map, RHS._map);
        }

        // clang-format off
        class iterator {
        public:
            iterator() = default;

            using iterator_category = typename ListType::iterator::iterator_category;
            using value_type        = typename ListType::value_type;
            using difference_type   = typename ListType::difference_type;
            using pointer           = typename ListType::pointer;
            using reference         = value_type &;

            inline std::pair<const K, V> &operator*() const { return *i; }
            inline std::pair<const K, V> *operator->() const { return &(*i); }
            inline bool operator==(const iterator &o) const { return i == o.i; }
            inline bool operator!=(const iterator &o) const { return i != o.i; }
            inline iterator &operator++() { i++; return *this; }
            inline iterator operator++(int) { iterator r = *this; i++; return r; }
            inline iterator &operator--() { i--; return *this; }
            inline iterator operator--(int) { iterator r = *this; i--; return r; }

            inline const K &key() const { return i->first; }
            inline V &value() const { return i->second; }

        private:
            explicit iterator(const typename ListType::iterator &i) : i(i) {
            }
            typename ListType::iterator i;
            friend class LinkedMap;
            friend class const_iterator;
        };

        class const_iterator {
        public:
            using iterator_category = typename ListType::const_iterator::iterator_category;
            using value_type        = typename ListType::value_type;
            using difference_type   = typename ListType::difference_type;
            using pointer           = typename ListType::const_pointer;
            using reference         = const value_type &;

            const_iterator() = default;
            const_iterator(const iterator &it) : i(it.i) {
            }
            inline const std::pair<const K, V> &operator*() const { return *i; }
            inline const std::pair<const K, V> *operator->() const { return &(*i); }
            inline bool operator==(const const_iterator &o) const { return i == o.i; }
            inline bool operator!=(const const_iterator &o) const { return i != o.i; }
            inline const_iterator &operator++() { i++; return *this; }
            inline const_iterator operator++(int) { iterator r = *this; i++; return r; }
            inline const_iterator &operator--() { i--; return *this; }
            inline const_iterator operator--(int) { iterator r = *this; i--; return r; }

            inline const K &key() const { return i->first; }
            inline const V &value() const { return i->second; }

        private:
            explicit const_iterator(const typename ListType::const_iterator &i) : i(i) {
            }
            typename ListType::const_iterator i;
            friend class LinkedMap;
        };
        // clang-format on

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        std::pair<iterator, bool> append(const K &key, const V &val) {
            return insert_impl(_list.end(), key, val);
        }

        std::pair<iterator, bool> append(const K &key, V &&val) {
            return insert_impl(_list.end(), key, std::forward<V>(val));
        }

        std::pair<iterator, bool> prepend(const K &key, const V &val) {
            return insert_impl(_list.begin(), key, val);
        }

        std::pair<iterator, bool> prepend(const K &key, V &&val) {
            return insert_impl(_list.begin(), key, std::forward<V>(val));
        }

        std::pair<iterator, bool> insert(const const_iterator &it, const K &key, const V &val) {
            return insert_impl(it.i, key, val);
        }

        std::pair<iterator, bool> insert(const const_iterator &it, const K &key, V &&val) {
            return insert_impl(it.i, key, std::forward<V>(val));
        }

        V &operator[](const K &key) {
            return append(key, V{}).first->second;
        }

        bool remove(const K &key) {
            auto it = _map.find(key);
            if (it == _map.end()) {
                return false;
            }
            _list.erase(it->second);
            _map.erase(it);
            return true;
        }

        size_t erase(const K &key) {
            return remove(key) ? 1 : 0;
        }

        iterator erase(const iterator &it) {
            return erase(const_iterator(it));
        }

        iterator erase(const const_iterator &it) {
            auto it2 = _map.find(it.key());
            if (it2 == _map.end()) {
                return iterator();
            }
            auto res = std::next(it2->second);
            _list.erase(it2->second);
            _map.erase(it2);
            return iterator(res);
        }

        iterator find(const K &key) {
            auto it = _map.find(key);
            if (it != _map.end()) {
                return iterator(it->second);
            }
            return end();
        }

        const_iterator find(const K &key) const {
            auto it = _map.find(key);
            if (it != _map.cend()) {
                return const_iterator(it->second);
            }
            return cend();
        }

        V value(const K &key, const V &defaultValue = V{}) const {
            auto it = _map.find(key);
            if (it != _map.end()) {
                return (*it->second).second;
            }
            return defaultValue;
        }

        // clang-format off
        inline iterator begin() { return iterator(_list.begin()); }
        inline const_iterator begin() const { return const_iterator(_list.cbegin()); }
        inline const_iterator cbegin() const { return const_iterator(_list.cbegin()); }
        inline iterator end() { return iterator(_list.end()); }
        inline const_iterator end() const { return const_iterator(_list.cend()); }
        inline const_iterator cend() const { return const_iterator(_list.cend()); }
        reverse_iterator rbegin() { return reverse_iterator(end()); }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
        const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
        inline bool contains(const K &key) const { return _map.find(key) != _map.end(); }
        inline size_t size() const { return _list.size(); }
        inline bool empty() const { return _list.empty(); }
        inline void clear() { _list.clear(); _map.clear(); }
        // clang-format on

        template <class Container = std::vector<K>>
        Container keys() const {
            Container res;
            res.reserve(_list.size());
            for (const auto &item : std::as_const(_list)) {
                res.push_back(item.first);
            }
            return res;
        }
        template <class Container = std::vector<V>>
        Container values() const {
            Container res;
            res.reserve(_list.size());
            for (const auto &item : std::as_const(_list)) {
                res.push_back(item.second);
            }
            return res;
        }
        size_t capacity() const {
            return _map.capacity();
        }
        void reserve(size_t size) {
            _map.reserve(size);
        }

    private:
        std::pair<iterator, bool> insert_impl(typename ListType::iterator it, const K &key,
                                              const V &val) {
            auto res = _map.insert(std::make_pair(key, _list.end()));
            auto &org_it = res.first->second;
            if (res.second) {
                // key doesn't exist
                auto new_it = _list.emplace(it, key, val);
                org_it = new_it;
                return std::make_pair(iterator(new_it), true);
            }
            return std::make_pair(iterator(org_it), false);
        }

        std::pair<iterator, bool> insert_impl(typename ListType::iterator it, const K &key,
                                              V &&val) {
            auto res = _map.insert(std::make_pair(key, _list.end()));
            auto &org_it = res.first->second;
            if (res.second) {
                // key doesn't exist
                auto new_it = _list.emplace(it, key, std::forward<V>(val));
                org_it = new_it;
                return std::make_pair(iterator(new_it), true);
            }
            return std::make_pair(iterator(org_it), false);
        }
    };

}

#endif // LORE_BASE_CORELIB_LINKEDMAP_H
