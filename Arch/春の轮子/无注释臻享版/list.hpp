#include <cstddef>  // for size_t nullptr_t ptrdiff_t ...
#include <cstdint>  // for uint64_t int8_t 
#include <initializer_list>
#include <iterator>
#include <numeric> // for max

namespace qc {
#ifdef NDEBUG
#define DEBUG_INIT_DEADBEAF(T)
#else
#define DEBUG_INIT_DEADBEAF(T) \
    { (T *)0xdeadbead }
#endif

template <class T, class Alloc = std::allocator<T>>
class list {
public:
    struct ListNode {
        ListNode *m_next DEBUG_INIT_DEADBEAF(ListNode);
        ListNode *m_prev DEBUG_INIT_DEADBEAF(ListNode);
        inline T &value() { return static_cast<ListValueNode &>(*this).m_val; }
        inline T const &value() const {
            return static_cast<ListValueNode const &>(*this).m_val;
        }
    };

    struct ListValueNode : public ListNode {
        union {
            T m_val;
        };
    };

public:
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;
    using AllocNode = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ListValueNode>;

public:
    list(Alloc const &alloc = Alloc()) noexcept {
        m_dummy.m_next = &m_dummy;
        m_dummy.m_prev = &m_dummy;
        m_alloc = alloc;
        m_size = 0;
    }

    explicit list(size_t n, Alloc const &alloc = Alloc()) : m_alloc(alloc) {
        ListNode *prev = &m_dummy;
        for (size_t i = 0; i < n; ++i) {
            ListNode *node = newNode();
            prev->m_next = node;
            node->m_prev = prev;
            std::construct_at(&node->value());
            prev = node;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
        m_size = n;
    }

    explicit list(size_t n, T const &val, Alloc const &alloc = Alloc())
        : m_alloc(alloc) {
        ListNode *prev = &m_dummy;
        for (size_t i = 0; i < n; ++i) {
            ListNode *node = newNode();
            prev->m_next = node;
            node->m_prev = prev;
            std::construct_at(&node->value(), val);
            prev = node;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
        m_size = n;
    }

    template <std::input_iterator InputIt>
    explicit list(InputIt first, InputIt last, Alloc const &alloc = Alloc()) : m_alloc(alloc) {
        ListNode *prev = &m_dummy;
        while (first != last) {
            ListNode *node = newNode();
            node->m_prev = prev;
            prev->m_next = node;
            std::construct_at(&node->value(), *first++);
            prev = node;
            ++m_size;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
    }

    list(std::initializer_list<T> ilist, Alloc const &alloc = Alloc())
        : m_alloc(alloc) {
        _uninit_assign(ilist.begin(), ilist.end());
    }

    list(list const &lst, Alloc const &alloc = Alloc()) : m_alloc(alloc) {
        ListNode *prev = &m_dummy;
        for (typename list<T>::iterator it = lst.begin(); it != lst.end();
             ++it) {
            ListNode *node = newNode();
            prev->m_next = node;
            node->m_prev = prev;
            std::construct_at(&node->value(), *it);
            prev = node;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
        m_size = lst.m_size;
    }

    list(list &&that) : m_alloc(std::move(that.m_alloc)) {
        m_dummy = that.m_dummy;
        m_dummy.m_next->m_prev = &m_dummy;
        m_dummy.m_prev->m_next = &m_dummy;
        m_size = that.m_size;
        that.m_dummy.m_next = that.m_dummy.m_prev = &that.m_dummy;
        that.m_size = 0;
    }

    list(list &&that, Alloc const &alloc) : m_alloc(alloc) {
        m_dummy = that.m_dummy;
        m_dummy.m_next->m_prev = &m_dummy;
        m_dummy.m_prev->m_next = &m_dummy;
        m_size = that.m_size;
        that.m_dummy.m_next = that.m_dummy.m_prev = &that.m_dummy;
        that.m_size = 0;
    }

    list &operator=(list &&that) {
        m_alloc = std::move(that.m_alloc);
        clear();
        m_dummy = that.m_dummy;
        m_dummy.m_next->m_prev = &m_dummy;
        m_dummy.m_prev->m_next = &m_dummy;
        m_size = that.m_size;
        that.m_dummy.m_next = that.m_dummy.m_prev = &that.m_dummy;
        that.m_size = 0;
    }

    list &operator=(std::initializer_list<T> ilist) { assign(ilist); }

    ~list() noexcept {
        ListNode *curr = m_dummy.m_next;
        while (curr != &m_dummy) {
            std::destroy_at(&curr->value());
            auto nxt = curr->m_next;
            deleteNode(curr);
            curr = nxt;
        }
        m_size = 0;
    }

private:
    template <std::input_iterator InputIt>
    void _uninit_assign(InputIt first, InputIt last) {
        ListNode *prev = &m_dummy;
        while (first != last) {
            ListNode *node = newNode();
            node->m_prev = prev;
            prev->m_next = node;
            std::construct_at(&node->value(), *first++);
            prev = node;
            ++m_size;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
    }

    void _uninit_assign(size_t n, T const &val) {
        ListNode *prev = &m_dummy;
        for (size_t i = 0; i < n; ++i) {
            ListNode *node = newNode();
            node->m_prev = prev;
            prev->m_next = node;
            std::construct_at(&node->value(), val);
            prev = node;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
        m_size = n;
    }

    void _uninit_assign(size_t n) {
        ListNode *prev = &m_dummy;
        for (size_t i = 0; i < n; ++i) {
            ListNode *node = newNode();
            node->m_prev = prev;
            prev->m_next = node;
            std::construct_at(&node->value());
            prev = node;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
        m_size = n;
    }

public:
    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        ListNode *prev = &m_dummy;
        while (first != last) {
            ListNode *node = newNode();
            node->m_prev = prev;
            prev->m_next = node;
            std::construct_at(&node->value(), *first++);
            prev = node;
            ++m_size;
        }
        prev->m_next = &m_dummy;
        m_dummy.m_prev = prev;
    }

    void assign(std::initializer_list<T> ilist) {
        clear();
        _uninit_assign(ilist.begin(), ilist.end());
    }

    void assign(size_t n, T const &val) {
        clear();
        _uninit_assign(n, val);
    }

    void assign(size_t n) {
        clear();
        _uninit_assign(n);
    }

public:
    void clear() noexcept {
        ListNode *curr = m_dummy.m_next;
        while (curr != &m_dummy) {
            std::destroy_at(&curr->value());
            auto nxt = curr->m_next;
            deleteNode(curr);
            curr = nxt;
        }
        m_dummy.m_next = m_dummy.m_prev = &m_dummy;
        m_size = 0;
    }

public:
    bool empty() const noexcept { return m_dummy.m_prev == m_dummy.m_next; }

    T &front() noexcept { return m_dummy.m_next->value(); }

    const T &front() const noexcept { return m_dummy.m_next->value(); }

    T &back() noexcept { return m_dummy.m_prev->value(); }

    T const &back() const noexcept { return m_dummy.m_prev->value(); }

    template <class Visitor>
    void foreach (Visitor visit) noexcept {
        ListNode *curr = m_dummy.m_next;
        while (curr != &m_dummy) {
            visit(curr->value());
            curr = curr->m_next;
        }
    }

public:
    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        friend list;

    private:
        ListNode *m_curr;

        explicit iterator(ListNode *curr) noexcept : m_curr(curr) {}

    public:
        iterator() = default;

        iterator &operator++() noexcept {
            m_curr = m_curr->m_next;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        iterator &operator--() noexcept {
            m_curr = m_curr->m_prev;
            return *this;
        }
        iterator operator--(int) noexcept {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        T &operator*() const noexcept { return m_curr->value(); }

        bool operator!=(iterator const &that) const noexcept {
            return m_curr != that.m_curr;
        }

        bool operator==(iterator const &that) const noexcept {
            return m_curr == that.m_curr;
        }
    };

    struct const_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T const;
        using difference_type = ptrdiff_t;
        using pointer = T const *;
        using reference = T const &;

        friend list;

    private:
        ListNode const *m_curr;

        explicit const_iterator(ListNode const *curr) noexcept : m_curr(curr) {}

        explicit operator iterator() noexcept {
            return iterator{const_cast<ListNode *>(m_curr)};
        }

    public:
        const_iterator() = default;

        const_iterator(iterator const &that) noexcept : m_curr(that.m_curr) {}

        const_iterator &operator++() noexcept {
            m_curr = m_curr->m_next;
            return *this;
        }

        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        const_iterator &operator--() noexcept {
            m_curr = m_curr->m_prev;
            return *this;
        }

        const_iterator operator--(int) noexcept {
            const_iterator tmp = *this;
            --*this;
            return tmp;
        }

        T const &operator*() const noexcept { return m_curr->value(); }

        bool operator!=(const_iterator const &that) const noexcept {
            return m_curr != that.m_curr;
        }

        bool operator==(const_iterator const &that) const noexcept {
            return m_curr == that.m_curr;
        }
    };

public:
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    iterator begin() noexcept { return iterator{m_dummy.m_next}; }

    iterator end() noexcept { return iterator{&m_dummy}; }

    const_iterator cbegin() const noexcept {
        return const_iterator{m_dummy.m_next};
    }

    const_iterator cend() const noexcept { return const_iterator{&m_dummy}; }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(iterator{&m_dummy});
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(iterator{m_dummy.m_next});
    }

    reverse_const_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(const_iterator{&m_dummy});
    }

    reverse_const_iterator crend() const noexcept {
        return std::make_reverse_iterator(const_iterator{m_dummy.m_next});
    }

    reverse_const_iterator rbegin() const noexcept { return crbegin(); }

    reverse_const_iterator rend() const noexcept { return crend(); }

public:
    template <class... Args>
    T &emplace_back(Args &&...args) {
        ListNode *node = newNode();
        std::construct_at(&node->value(), std::forward<Args>(args)...);
        ListNode *prev = m_dummy.m_prev;
        node->m_next = &m_dummy;
        node->m_prev = prev;
        prev->m_next = node;
        m_dummy.m_prev = node;
        ++m_size;
        return node->value();
    }

    void push_back(T const &val) {
        ListNode *node = newNode();
        std::construct_at(&node->value(), std::move(val));
        ListNode *prev = m_dummy.m_prev;
        node->m_next = &m_dummy;
        node->m_prev = prev;
        prev->m_next = node;
        m_dummy.m_prev = node;
        ++m_size;
    }

    void push_back(T &&val) { emplace_back(std::move(val)); }

    void push_front(T const &val) {
        ListNode *node = newNode();
        std::construct_at(&node->value(), val);
        node->m_next = m_dummy.m_next;
        node->m_prev = &m_dummy;
        m_dummy.m_next->m_prev = node;
        m_dummy->m_next = node;
        ++m_size;
    }

    void push_front(T &&val) { emplace_front(std::move(val)); }

    template <class... Args>
    T &emplace_front(Args &&...args) {
        ListNode *node = newNode();
        std::construct_at(&node->value(), std::forward<Args>(args)...);
        node->m_next = m_dummy.m_next;
        node->m_prev = &m_dummy;
        m_dummy.m_next->m_prev = node;
        m_dummy.m_next = node;
        ++m_size;
        return node->value();
    }

    iterator erase(const_iterator pos) noexcept {
        ListNode *node = const_cast<ListNode *>(pos.m_curr);
        ListNode *next = node->m_next;
        ListNode *prev = node->m_prev;
        prev->m_next = next;
        next->m_prev = prev;
        std::destroy_at(&node->value());
        deleteNode(node);
        --m_size;
        return iterator{next};
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        while (first != last) {
            first = erase(first);
            --m_size;
        }
        return iterator(first);
    }
    
    size_t remove(T const &val) noexcept {
        auto first = begin();
        auto last = end();
        size_t count = 0;
        while (first != last) {
            if (*first == val)
                first = erase(first), ++count, --m_size;
            else
                ++first;
        }
        return count;
    }

    template <std::invocable<T &> Pred>
    size_t remove_if(Pred &&pred) noexcept {
        auto first = begin();
        auto last = end();
        size_t count = 0;
        while (first != last) {
            if (pred(*first))
                first = erase(first), ++count, --m_size;
            else
                ++first;
        }
        return count;
    }

    void pop_front() noexcept { erase(begin()); }

    void pop_back() noexcept {
        erase(std::prev(end()));
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args &&...args) {
        ListNode *node = newNode();
        ListNode *curr = const_cast<ListNode *>(pos.m_curr);
        ListNode *prev = curr->m_prev;
        node->m_next = curr;
        node->m_prev = prev;
        prev->m_next = node;
        curr->m_prev = node;
        std::construct_at(&node->value(), std::forward<Args>(args)...);
        ++m_size;
        return iterator{node};
    }

    iterator insert(const_iterator pos, const T &val) {
        return emplace(pos, val);
    }

    iterator insert(const_iterator pos, T &&val) {
        return emplace(pos, std::move(val));
    }

    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        const_iterator orig_first;
        bool had_orig = false;
        while (first != last) {
            pos = emplace(pos, *first);
            if (!had_orig) {
                had_orig = true;
                orig_first = pos;
            }
            ++pos;
            ++first;
            ++m_size;
        }
        return orig_first;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    iterator insert(const_iterator pos, size_t n, T const &val) {
        const_iterator orig_first;
        bool had_orig = false;
        while (n) {
            pos = emplace(pos, val);
            if (!had_orig) {
                had_orig = true;
                orig_first = pos;
            }
            ++pos;
            --n;
        }
        m_size += n;
        return orig_first;
    }

    void splice(const_iterator pos, list<T> &&that) {
        insert(pos, std ::make_move_iterator(that.begin()),
               std::make_move_iterator(that.end()));
    }

    size_t size() const noexcept { return m_size; }

    static constexpr size_t max_size() noexcept {
        return std::numeric_limits<size_t>::max();
    }

    Alloc get_allocator() const { return m_alloc; }

public:
    ListNode *get_head() const noexcept { return m_dummy.m_next; }

    ListNode const *get_dummy() const noexcept { return &m_dummy; }

    ListNode *get_dummy() noexcept { return &m_dummy; }

private:
    ListNode m_dummy;
    size_t m_size = 0;
    [[no_unique_address]] Alloc m_alloc;

    ListNode *newNode() {
        return AllocNode{m_alloc}.allocate(sizeof(ListValueNode));
    }

    void deleteNode(ListNode *node) noexcept {
        AllocNode{m_alloc}.deallocate(static_cast<ListValueNode *>(node), 1);
    }
};

}  // namespace qc
