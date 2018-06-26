//
// Created by artem on 24.06.18.
//

#ifndef CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_CIRCULAR_BUFFER_H

#include <iterator>
#include <cassert>
#include <cstring>

template<typename T>
class circular_buffer {
private:
    size_t _size;
    size_t capacity;
    size_t beg;
    T *data;

    template<typename U>
    struct buffer_iterator;
public:
    using iterator = buffer_iterator<T>;
    using const_iterator = buffer_iterator<T const>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    template<typename U>
    class buffer_iterator : public std::iterator<std::random_access_iterator_tag, U> {
    public:
        friend class circular_buffer;

        buffer_iterator() = delete;

        buffer_iterator(buffer_iterator const &other) = default;

        template<typename V>
        buffer_iterator(buffer_iterator<V> const &other, typename std::enable_if<
                std::is_same<V const, U>::value && std::is_const<U>::value>::type * = nullptr) :
                pos(other.pos), cap(other.cap), p(other.p), beg(other.beg) {}


        buffer_iterator &operator++() {
            return *this += 1;
        }

        buffer_iterator operator++(int) {
            buffer_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        buffer_iterator &operator--() {
            return *this -= 1;
        }

        buffer_iterator operator--(int) {
            buffer_iterator tmp = *this;
            --*this;
            return tmp;
        }

        buffer_iterator &operator+=(ptrdiff_t const &b) {
            if (b < 0) {
                return *this -= (-b);
            }
            p += (b % cap);
            if (p >= beg + cap) {
                p -= cap;
            }
            pos += b;
            return *this;
        }

        buffer_iterator &operator-=(ptrdiff_t const &b) {
            if (b < 0) {
                return *this += (-b);
            }
            p -= (b % cap);
            if (p < beg) {
                p += cap;
            }
            pos -= b;
            return *this;
        }

        friend ptrdiff_t operator-(buffer_iterator const &a, buffer_iterator const &b) {
            return a.pos - b.pos;
        }

        friend bool operator==(buffer_iterator const &a, buffer_iterator const &b) {
            return a.p == b.p;
        }

        friend bool operator!=(buffer_iterator const &a, buffer_iterator const &b) {
            return a.p != b.p;
        }

        friend bool operator<(buffer_iterator const &a, buffer_iterator const &b) {
            return a.pos < b.pos;
        }

        friend bool operator>(buffer_iterator const &a, buffer_iterator const &b) {
            return a.pos > b.pos;
        }

        friend bool operator<=(buffer_iterator const &a, buffer_iterator const &b) {
            return a.pos <= b.pos;
        }

        friend bool operator>=(buffer_iterator const &a, buffer_iterator const &b) {
            return a.pos >= b.pos;
        }

        U &operator*() const {
            return *p;
        }

        U *operator->() const {
            return p;
        }

        friend buffer_iterator operator+(buffer_iterator a, ptrdiff_t const &b) {
            return a += b;
        }

        friend buffer_iterator operator-(buffer_iterator a, ptrdiff_t const &b) {
            return a -= b;
        }

    private:
        buffer_iterator(U *ptr, size_t pos, size_t cap, U *beg) : pos(pos), cap(cap), p(ptr), beg(beg) {}

        size_t pos, cap;
        U *p, *beg;
    };

    void move(const_iterator pos, T *buffer) {
        std::copy(buffer, buffer + 1, pos);
    }

public:

    circular_buffer() : _size(0), capacity(10), beg(0) {
        data = static_cast<T *>(malloc(10 * sizeof(T)));
    }

    circular_buffer(size_t n) : _size(0), capacity(n), beg(0) {
        data = static_cast<T *>(malloc(n * sizeof(T)));
    }

    circular_buffer(circular_buffer const &other) : circular_buffer(other.capacity) {
        try {
            for (const_iterator it = other.begin(); it != other.end(); it++) {
                push_back(*it);
            }
        } catch (...) {
            clear();
            throw;
        }
    };

    ~circular_buffer() noexcept {
        clear();
        free(data);
    }

    T &operator[](const int &pos) {
        return data[(beg + pos) % capacity];
    }

    T const &operator[](const int &pos) const {
        return data[(beg + pos) % capacity];
    }

    circular_buffer &operator=(circular_buffer const &other) {
        circular_buffer tmp(other);
        swap(tmp, *this);
        return *this;
    }

    iterator begin() {
        return iterator(data + beg, beg, capacity, data);
    }

    const_iterator cbegin() const {
        return const_iterator(data + beg, beg, capacity, data);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return begin() + _size;
    }

    const_iterator cend() const {
        return begin() + _size;
    }

    const_iterator end() const {
        return cend();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    void push_back(T const &value) {
        if (_size + 1 >= capacity) {
            circular_buffer tmp = circular_buffer(capacity * 2);
            for (auto &el:*this) {
                tmp.push_back(el);
            }
            swap(*this, tmp);
        }
        new(end().p) T(value);
        _size++;
    }

    void pop_back() noexcept {
        _size--;
        (end())->~T();
    }


    void push_front(T const &value) {
        if (_size + 1 >= capacity) {
            circular_buffer tmp = circular_buffer(capacity * 2);
            for (auto &el:*this) {
                tmp.push_back(el);
            }
            swap(*this, tmp);
        }
        new((begin() - 1).p) T(value);
        beg = (capacity + beg - 1) % capacity;
        _size++;
    }

    void pop_front() noexcept {
        (begin()++)->~T();
        _size--;
        beg = (beg + 1) % capacity;
    }


    iterator insert(const_iterator pos, T const &value) {
        int sz = pos.pos - begin().pos;
        push_back(value);
        for (iterator it = end() - 1; it > pos; it--) {
            std::swap(*it, *(it - 1));
        }
        return buffer_iterator((begin() + sz).p, sz + beg, capacity, data);
    }

    iterator erase(const_iterator pos) {
        iterator tmppos = begin() + (pos.pos - beg + capacity) % capacity;
        for (iterator it = tmppos + 1; it != end(); it++) {
            std::swap(*it, *(it - 1));
        }
        pop_back();
        return buffer_iterator<T>(data + (beg + pos.pos) % capacity, (pos.pos + capacity) % capacity, capacity, data);
    }

    T &front() {
        return data[beg];
    }

    T &back() {
        return data[(beg + _size - 1) % capacity];
    }

    T const &front() const {
        return data[beg];
    }

    T const &back() const {
        return data[(beg + _size - 1) % capacity];
    }

    bool empty() const {
        return _size == 0;
    }

    size_t size() const {
        return _size;
    }

    void clear() {
        while (_size) {
            pop_back();
        }
    }

    friend void swap(circular_buffer<T> &a, circular_buffer<T> &b) {
        std::swap(a.data, b.data);
        std::swap(a.beg, b.beg);
        std::swap(a._size, b._size);
        std::swap(a.capacity, b.capacity);
    }
};

#endif //CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
