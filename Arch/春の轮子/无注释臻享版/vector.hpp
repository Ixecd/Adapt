#include <cstddef>  // for size_t nullptr_t ptrdiff_t ...
#include <cstdint>  // for uint64_t int8_t 
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility> // for std::as_const
namespace qc {

template <class T, class Alloc = std::allocator<T>>
class vector {
public:
    using allocator = std::allocator<T>;
    using value_type = Alloc;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<T const*>;

public:
    vector(Alloc const& alloc = allocator()) noexcept : m_alloc(alloc) {
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    explicit vector(size_t n, allocator const& alloc = Alloc())
        : m_alloc(alloc) {
        m_data = m_alloc.allocate(n);
        m_size = m_capacity = n;
        for (size_t i = 0; i < m_size; ++i) std::construct_at(&m_data[i]);
    }

    vector(size_t n, T const& val, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        m_data = m_alloc.allocate(n);
        m_size = m_capacity = n;
        for (size_t i = 0; i < m_size; ++i)
            std::construct_at(&m_data[i], std::move(val));
    }

    template <std::random_access_iterator InputIt>
    vector(InputIt first, InputIt last, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        size_t n = last - first;
        m_data = m_alloc.allocate(n);
        for (size_t i = 0; i < n; ++i) std::construct_at(&m_data[i], *first++);
        m_capacity = m_size = n;
    }

    vector(std::initializer_list<T> ilist, allocator const& alloc = allocator())
        : vector(ilist.begin(), ilist.end(), alloc) {}

    vector(const vector& v, allocator const& alloc = allocator())
        : m_alloc(alloc) {
        m_capacity = m_size = v.m_size;
        if (m_size) {
            m_data = m_alloc.allocate(m_size);
            for (size_t i = 0; i < m_size; ++i) {
                std::construct_at(&m_data[i], std::as_const(v.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    vector(vector&& v, allocator const& alloc = allocator()) noexcept
        : m_alloc(alloc) {
        m_data = v.m_data;
        m_size = v.m_size;
        m_capacity = v.m_capacity;
        v.m_data = nullptr;
        v.m_size = 0;
        v.m_capacity = 0;
    }

    vector& operator=(vector&& v) noexcept {
        for (size_t i = 0; i < m_size; ++i) 
            std::destroy(m_data[i]);
        if (m_capacity) [[likely]]
            m_alloc.deallocate(m_data, m_capacity);
        m_data = v.m_data;
        m_size = v.m_size;
        m_capacity = v.m_capacity;
        v.m_data = nullptr;
        v.m_size = 0;
        v.m_capacity = 0;
        return *this;
    }

    vector& operator=(vector const& v) {
        if (&v == this) return *this;
        reserve(v.m_size);
        m_capacity = m_size = v.m_size;
        if (m_size) {
            for (size_t i = 0; i < m_size; ++i)
                std::construct_at(&m_data[i], std::as_const(v.m_data[i]));
        }
        return *this;
    }

    Alloc get_allocator() const noexcept { return m_alloc; }

    bool operator==(vector const& that) noexcept {
        return std::equal(begin(), end(), that.begin(), that.end());
    }

    template <std::random_access_iterator InputIt>
    void assgin(InputIt first, InputIt last) {
        size_t diff = last - first;
        reserve(diff);
        for (size_t i = 0; i < diff; ++i) {
            std::construct_at(&m_data[i], std::as_const(*first++));
        }
    }

    void assgin(size_t n, T const& val) {
        clear();
        reserve(n);
        m_size = n;
        for (size_t i = 0; i < n; ++i) {
            std::construct_at(&m_data[i], val);
        }
    }

    void assign(std::initializer_list<T> ilist) noexcept {
        assign(ilist.begin(), ilist.end());
    }

    ~vector() noexcept {
        for (size_t i = 0; i < m_size; ++i) std::destroy_at(&m_data[i]);
        if (m_capacity) [[likely]]
            m_alloc.deallocate(m_data, m_capacity);
    }

public:
    void clear() noexcept {
        for (size_t i = 0; i < m_size; ++i) std::destroy_at(&m_data[i]);
        m_size = 0;
    }

    void _recapacity(size_t n) {
        {
            if (m_capacity) {
                if (0 == n) {
                    allocator{}.deallocate(m_data, m_capacity);
                    m_data = nullptr;
                    m_capacity = 0;
                    return;
                }
                T* tmp = allocator{}.allocate(n);
                for (size_t i = 0; i < std::min(m_capacity, n); ++i)
                    std::construct_at(&tmp[i], std::as_const(m_data[i]));
                allocator{}.deallocate(m_data, m_capacity);
                m_data = tmp;
                m_capacity = n;
            } else {
                if (0 == n) return;
                m_data = allocator{}.allocate(n);
                m_capacity = n;
            }
        }
    }

    void shrink_to_fit() {
        T* old_data = m_data;
        size_t old_capacity = m_capacity;
        m_capacity = m_size;
        if (m_size == 0) {
            m_data = nullptr;
        } else {
            m_data = m_alloc.allocate(m_size);
            for (size_t i = 0; i < m_size; ++i)
                std::construct_at(&m_data[i], std::as_const(old_data[i]));
        }
        if (old_capacity) m_alloc.deallocate(old_data, old_capacity);
    }
    
    void _grow_capacity_until(size_t n) {
        if (n <= m_capacity) return;
        T* old_data = m_data;
        size_t old_capacity = m_capacity;
        n = std::max(n, m_capacity * 2);
        if (n == 0) {
            m_data = nullptr;
            m_capacity = 0;
        } else {
            m_data = m_alloc.allocate(n);
            m_capacity = n;
        }
        if (old_capacity) {
            if (m_size)
                for (size_t i = 0; i < m_size; ++i)
                    std::construct_at(&m_data[i], std::as_const(old_data[i]));
            m_alloc.deallocate(old_data, old_capacity);
        }
    }

    void resize(size_t n) {
        _grow_capacity_until(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; ++i) std::construct_at(&m_data[i]);
        }
        m_size = n;
    }

    void resize(size_t n, T const& val) {
        _grow_capacity_until(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; ++i)
                std::construct_at(&m_data[i], std::move(val));
        }
        m_size = n;
    }

    void reserve(size_t n) { _grow_capacity_until(n); }

public:
    T& operator[](size_t i) noexcept { return m_data[i]; }

    const T& operator[](size_t i) const noexcept { return m_data[i]; }

    T& at(size_t i) {
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at");
        return m_data[i];
    }

    T const& at(size_t i) const {
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at");
        return m_data[i];
    }

    T& back() noexcept { return m_data[m_size - 1]; }

    T const& back() const noexcept { return m_data[m_size - 1]; }

    T& front() noexcept { return m_data[0]; }

    T const& front() const noexcept { return m_data[0]; }

    size_t size() const noexcept { return m_size; }

    bool empty() const noexcept { return m_size == 0; }

    size_t capacity() const noexcept { return m_capacity; }

public:
    const iterator begin() const noexcept { return m_data; }

    const iterator end() const noexcept { return m_data + m_size; }

    const iterator cbegin() const noexcept { return m_data; }

    const iterator cend() const noexcept { return m_data + m_size; }

    std::reverse_iterator<iterator> rbegin() noexcept {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<iterator> rend() noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<const iterator> rcbegin() const noexcept {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<const iterator> rcend() const noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }

    iterator begin() noexcept { return m_data; }

    iterator end() noexcept { return m_data + m_size; }

    iterator data() noexcept { return m_data; }

    const iterator data() const noexcept { return m_data; }

    const iterator cdata() const noexcept { return m_data; }

public:
    void push_back(T const& val) {
        reserve(size() + 1);
        std::construct_at(&m_data[m_size], std::as_const(val));
        m_size += 1;
    }

    void push_back(T&& val) {
        reserve(size() + 1);
        std::construct_at(&m_data[m_size], std::move(val));
        m_size += 1;
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        reserve(size() + 1);
        T* p = &m_data[m_size];
        std::construct_at(p, std::forward<Args>(args)...);
        m_size += 1;
        return *p;
    }
    T* erase(size_t index) noexcept {
        for (size_t j = index + 1; j < m_size; ++j) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(size() - 1);
        std::destroy_at(&m_data[m_size]);
        return const_cast<T*>(&m_data[index]);
    }

    T* erase(size_t ibeg, size_t iend) noexcept {
        size_t diff = iend - ibeg;
        for (size_t j = iend; j < m_size; ++j) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        m_size -= diff;
        for (size_t i = m_size; i < m_size + diff; ++i)
            std::destroy_at(&m_data[i]);

        return const_cast<T*>(&m_data[ibeg]);
    }

    T* erase(iterator const it) noexcept(std::is_nothrow_move_assignable_v<T>) {
        size_t s = it - m_data;
        for (size_t i = s + 1; i != m_size; ++i)
            m_data[i - 1] = std::move_if_noexcept(m_data[i]);
        m_size -= 1;
        std::destroy_at(&m_data[m_size]);
        return const_cast<T*>(it + 1);
    }

    T* erase(iterator const first, iterator const last) noexcept {
        size_t start = first - m_data;
        size_t end = last - m_data;
        return erase(start, end);
    }

    T* insert(iterator const it, T&& val) {
        size_t j = it - m_data;
        reserve(size() + 1);
        m_size += 1;
        for (int i = j; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }

        for (size_t i = j; i < j + 1; ++i)
            std::construct_at(&m_data[i], std::move(val));
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, T const& val) {
        size_t j = it - m_data;
        reserve(size() + 1);
        m_size += 1;
        for (int i = j; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        for (size_t i = j; i < j + 1; ++i) std::construct_at(&m_data[i], val);
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, size_t n, T const& val) {
        size_t j = it - m_data;
        if (n == 0) [[unlikely]]
            return const_cast<T*>(it);
        reserve(size() + n);
        m_size += n;
        for (int i = j + n - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        for (size_t i = j; i < j + n; ++i) std::construct_at(&m_data[i], val);
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, size_t n) {
        size_t j = it - m_data;
        size_t l = end() - it;
        if (n == 0) [[unlikely]]
            return it;
        reserve(size() + n);
        m_size += n;
        for (int i = j + n - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        for (size_t i = j; i < j + n; ++i) std::construct_at(&m_data[i]);
        return const_cast<T*>(it);
    }

    template <std::random_access_iterator InputIt>
    T* insert(iterator const it, InputIt first, InputIt last) {
        size_t n = last - first;
        size_t j = it - m_data;
        size_t l = end() - it;
        if (n == 0) [[unlikely]]
            return it;
        reserve(size() + n);
        m_size += n;
        for (int i = j + l - 1; i >= (int)j; --i) {
            std::construct_at(&m_data[i + 1], std::move(m_data[i]));
            std::destroy_at(&m_data[i]);
        }
        for (size_t i = j; i < j + n; ++i) {
            std::construct_at(&m_data[i], std::move(*first++));
        }
        return const_cast<T*>(it);
    }

    T* insert(iterator const it, std::initializer_list<T> ilist) {
        return insert(it, ilist.begin(), ilist.end());
    }

private:
    T* m_data;
    size_t m_size;
    size_t m_capacity;
    [[no_unique_address]] allocator m_alloc;
};

}  // namespace qc
