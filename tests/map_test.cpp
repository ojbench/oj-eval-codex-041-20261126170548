#include "map.hpp"
#include <cassert>
#include <map>
#include <random>
#include <vector>

template<class A, class B> void equal(const A &a, const B &b) {
    assert(a.size() == b.size());
    auto x = a.begin();
    for (auto &v : b) {
        assert(x != a.end());
        assert(x->first == v.first && x->second == v.second);
        ++x;
    }
    assert(x == a.end());
    auto y = b.end();
    while (y != b.begin()) {
        --y; --x;
        assert(x->first == y->first && x->second == y->second);
    }
    assert(x == a.begin());
}
struct NonDefault {
    int x;
    explicit NonDefault(int n) : x(n) {}
    bool operator<(const NonDefault &b) const { return x < b.x; }
};
int main() {
    sjtu::map<NonDefault, NonDefault> special;
    special.insert({NonDefault(1), NonDefault(2)});
    assert(special.begin()->second.x == 2);
    auto special_copy = special;
    special_copy.erase(special_copy.begin());
    assert(special_copy.empty());
    sjtu::map<int, int> a;
    std::map<int, int> b;
    std::mt19937 rng(123456);
    auto end = a.end();
    for (int i = 0; i < 250000; ++i) {
        int k = rng() % 2000, v = rng();
        switch (rng() % 6) {
        case 0: {
            auto x = a.insert({k, v});
            auto y = b.insert({k, v});
            assert(x.second == y.second && x.first->second == y.first->second);
            break;
        }
        case 1:
            if (b.count(k)) { a.erase(a.find(k)); b.erase(k); }
            break;
        case 2: a[k] = v; b[k] = v; break;
        case 3: assert(a.count(k) == b.count(k)); break;
        case 4:
            if (b.count(k)) assert(a.at(k) == b.at(k));
            break;
        default:
            if (i % 1000 == 0) {
                sjtu::map<int, int> copy(a);
                equal(copy, b);
                copy = copy;
                equal(copy, b);
                copy.clear();
                copy = a;
                equal(copy, b);
            }
        }
        assert(end == a.end());
        if (i % 100 == 0) equal(a, b);
    }
    equal(a, b);
    std::vector<sjtu::map<int, int>::iterator> iterators;
    std::vector<const void *> addresses;
    for (int k = 0; k < 2000; ++k) {
        auto it = a.insert({k, k}).first;
        iterators.push_back(it);
        addresses.push_back(&*it);
    }
    for (int k = 0; k < 2000; k += 2) a.erase(iterators[k]);
    for (int k = 1; k < 2000; k += 2) {
        assert(iterators[k]->first == k);
        assert(&*iterators[k] == addresses[k]);
        assert(a.find(k) == iterators[k]);
    }
    sjtu::map<int, int>::const_iterator ci = a.begin();
    assert(ci == a.begin() && a.begin() == ci);
    sjtu::map<int, int, std::greater<int>> reverse;
    std::map<int, int, std::greater<int>> reference;
    for (int i = 0; i < 100000; ++i) { reverse[i] = i; reference[i] = i; }
    equal(reverse, reference);
    for (int i = 0; i < 100000; ++i) reverse.erase(reverse.begin());
    assert(reverse.empty());
}
