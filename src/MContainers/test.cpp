#include "MData/Pointers.hpp"
#include "MIo/stream.hpp"
#include "Treap.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>

using Treap = mgk::Treap<size_t, mgk::CringePtr>;

static void shift(Treap& treap, size_t k) {
    auto root = treap.getRoot();
    auto [l,r] = treap.splitSize(root, k);
    treap.setRoot(treap.merge(r, l));
    assert(treap.getNodeSize(treap.getRoot()) == 1000);
}

int main() {
    Treap treap;

    for(size_t i = 0; i < 1000; ++i) {
        mgk::out.flush();
        treap.setRoot(treap.merge(treap.getRoot(), treap.createNode(i)));
        assert(treap.getNodeSize(treap.getRoot()) == i + 1);
    }

    for(size_t i = 0; i < 1'000'000; ++i) {
        shift(treap, rand() % 1000);
    }
    mgk::out << "Calced: ";
    mgk::out << treap[0] << '\n';
}

// Raw ptrs:    27s
// Cringe ptrs: 40s