#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP
#include "utility.hpp"
#include "exceptions.hpp"
#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>

namespace sjtu {
template<class Key, class T, class Compare = std::less<Key>>
class map {
public:
    using value_type = pair<const Key, T>;
private:
    // The sentinel contains no value, so empty maps require no default constructors.
    struct link {
        link *left = nullptr, *right = nullptr, *parent = nullptr;
        link *prev = nullptr, *next = nullptr;
        int height = 1;
    };
    struct node : link {
        value_type value;
        explicit node(const value_type &v) : value(v) {}
    };
    link head_;
    link *root_ = nullptr;
    std::size_t size_ = 0;
    Compare compare_;

    static value_type &value(link *p) { return static_cast<node *>(p)->value; }
    static int height(link *p) { return p ? p->height : 0; }
    static void update(link *p) {
        int a = height(p->left), b = height(p->right);
        p->height = 1 + (a > b ? a : b);
    }
    void replace(link *old, link *p) {
        if (!old->parent) root_ = p;
        else if (old->parent->left == old) old->parent->left = p;
        else old->parent->right = p;
        if (p) p->parent = old->parent;
    }
    link *rotate_left(link *p) {
        link *q = p->right;
        replace(p, q);
        p->right = q->left;
        if (p->right) p->right->parent = p;
        q->left = p;
        p->parent = q;
        update(p);
        update(q);
        return q;
    }
    link *rotate_right(link *p) {
        link *q = p->left;
        replace(p, q);
        p->left = q->right;
        if (p->left) p->left->parent = p;
        q->right = p;
        p->parent = q;
        update(p);
        update(q);
        return q;
    }
    void rebalance(link *p) {
        while (p) {
            update(p);
            int balance = height(p->left) - height(p->right);
            if (balance > 1) {
                if (height(p->left->left) < height(p->left->right)) rotate_left(p->left);
                p = rotate_right(p);
            } else if (balance < -1) {
                if (height(p->right->right) < height(p->right->left)) rotate_right(p->right);
                p = rotate_left(p);
            }
            p = p->parent;
        }
    }
    link *lookup(const Key &key) const {
        link *p = root_;
        while (p) {
            if (compare_(key, value(p).first)) p = p->left;
            else if (compare_(value(p).first, key)) p = p->right;
            else return p;
        }
        return nullptr;
    }
    link *clone(link *source, link *parent, link *&last) {
        if (!source) return nullptr;
        node *p = new node(value(source));
        p->parent = parent;
        p->height = source->height;
        p->left = clone(source->left, p, last);
        p->prev = last;
        last->next = p;
        last = p;
        p->right = clone(source->right, p, last);
        return p;
    }
public:
    template<bool Const> class basic_iterator {
        friend class map;
        template<bool> friend class basic_iterator;
        link *p_ = nullptr;
        explicit basic_iterator(link *p) : p_(p) {}
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename map::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::conditional<Const, const value_type *, value_type *>::type;
        using reference = typename std::conditional<Const, const value_type &, value_type &>::type;
        basic_iterator() = default;
        template<bool C, typename std::enable_if<Const && !C, int>::type = 0>
        basic_iterator(const basic_iterator<C> &other) : p_(other.p_) {}
        reference operator*() const { return value(p_); }
        pointer operator->() const { return &value(p_); }
        basic_iterator &operator++() { p_ = p_->next; return *this; }
        basic_iterator operator++(int) { auto old = *this; ++*this; return old; }
        basic_iterator &operator--() { p_ = p_->prev; return *this; }
        basic_iterator operator--(int) { auto old = *this; --*this; return old; }
        template<bool C> bool operator==(const basic_iterator<C> &other) const { return p_ == other.p_; }
        template<bool C> bool operator!=(const basic_iterator<C> &other) const { return p_ != other.p_; }
    };
    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;
    map() { head_.next = head_.prev = &head_; }
    explicit map(const Compare &comp) : compare_(comp) { head_.next = head_.prev = &head_; }
    map(const map &other) : compare_(other.compare_) {
        head_.next = head_.prev = &head_;
        link *last = &head_;
        root_ = clone(other.root_, nullptr, last);
        last->next = &head_;
        head_.prev = last;
        size_ = other.size_;
    }
    map &operator=(const map &other) {
        if (this == &other) return *this;
        clear();
        compare_ = other.compare_;
        link *last = &head_;
        root_ = clone(other.root_, nullptr, last);
        last->next = &head_;
        head_.prev = last;
        size_ = other.size_;
        return *this;
    }
    ~map() { clear(); }
    iterator begin() { return iterator(head_.next); }
    const_iterator begin() const { return const_iterator(head_.next); }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return iterator(&head_); }
    const_iterator end() const { return const_iterator(const_cast<link *>(&head_)); }
    const_iterator cend() const { return end(); }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }
    void clear() {
        link *p = head_.next;
        while (p != &head_) {
            link *next = p->next;
            delete static_cast<node *>(p);
            p = next;
        }
        root_ = nullptr;
        size_ = 0;
        head_.next = head_.prev = &head_;
    }
    pair<iterator, bool> insert(const value_type &v) {
        link *p = root_, *parent = nullptr;
        bool left = false;
        while (p) {
            parent = p;
            if (compare_(v.first, value(p).first)) { left = true; p = p->left; }
            else if (compare_(value(p).first, v.first)) { left = false; p = p->right; }
            else return {iterator(p), false};
        }
        node *fresh = new node(v);
        fresh->parent = parent;
        if (!parent) {
            root_ = fresh;
            fresh->prev = fresh->next = &head_;
        } else if (left) {
            parent->left = fresh;
            fresh->prev = parent->prev;
            fresh->next = parent;
        } else {
            parent->right = fresh;
            fresh->prev = parent;
            fresh->next = parent->next;
        }
        fresh->prev->next = fresh;
        fresh->next->prev = fresh;
        ++size_;
        rebalance(parent);
        return {iterator(fresh), true};
    }
    void erase(iterator position) {
        link *p = position.p_, *start;
        if (!p || p == &head_) throw invalid_iterator();
        p->prev->next = p->next;
        p->next->prev = p->prev;
        if (!p->left || !p->right) {
            start = p->parent;
            replace(p, p->left ? p->left : p->right);
        } else {
            // Transplant the successor node itself to preserve all other iterators.
            link *s = p->next;
            if (s->parent == p) start = s;
            else {
                start = s->parent;
                replace(s, s->right);
                s->right = p->right;
                s->right->parent = s;
            }
            replace(p, s);
            s->left = p->left;
            s->left->parent = s;
        }
        delete static_cast<node *>(p);
        --size_;
        rebalance(start);
    }
    iterator find(const Key &key) { link *p = lookup(key); return p ? iterator(p) : end(); }
    const_iterator find(const Key &key) const { link *p = lookup(key); return p ? const_iterator(p) : end(); }
    std::size_t count(const Key &key) const { return lookup(key) != nullptr; }
    T &at(const Key &key) {
        link *p = lookup(key);
        if (!p) throw index_out_of_bound();
        return value(p).second;
    }
    const T &at(const Key &key) const {
        link *p = lookup(key);
        if (!p) throw index_out_of_bound();
        return value(p).second;
    }
    T &operator[](const Key &key) {
        link *p = lookup(key);
        if (p) return value(p).second;
        return insert(value_type(key, T())).first->second;
    }
    const T &operator[](const Key &key) const { return at(key); }
};
}
#endif
