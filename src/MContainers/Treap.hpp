#ifndef MGKTL_MCONTAINERS_TREAP_HPP
#define MGKTL_MCONTAINERS_TREAP_HPP

#include <concepts>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <MUtils/utils.hpp>
namespace mgk {

    template<typename T>
    using Ptr = T*;

    template<class T, template<typename> class Pointer = Ptr, class Updater = void(*)()>
    class Treap
    {
    public:
    
    Treap() = default;
    Treap(Updater upd) : updatef_(upd) {}

    ~Treap() {
        if constexpr (std::is_same<Pointer<Node>, Node*>::value) {
            delete root_;
        }
    }
        struct Node
        {
            T key; 
            Pointer<Node> left  = nullptr;
            Pointer<Node> right = nullptr;

            std::size_t priority_ = std::rand();
            std::size_t size_ = 1;

            ~Node() {
                if constexpr (std::is_same<Pointer<Node>, Node*>::value){
                    delete left;
                    delete right;
                }
            }
        };
    
    [[nodiscard]]
    std::pair<Pointer<Node>, Pointer<Node>> splitKey(Pointer<Node> node, const T& key)
    {
        if(!node) {return {nullptr, nullptr};}

        if(node->key <= key) {
            auto [left, right] = splitKey(node->right, key);
            node->right = left;
            update(node);
            return {node, right};
        }
        auto [left, right] = splitKey(node->left, key);
        node->left = right;
        update(node);
        return {left, node};
    }

    [[nodiscard]]
    std::pair<Pointer<Node>, Pointer<Node>> splitSize(Pointer<Node> node, size_t size)
    {
        if(!node && size == 0) {return {nullptr, nullptr};}
        assert(node);
        assert(size <= node->size_);

        if(size == 0) { return {nullptr, node}; }

        if(node->size_ == size) { return {nullptr, node}; }

        if(getNodeSize(node->left) == size) {
            Pointer<Node> left = node->left;
            node->left = nullptr;
            update(node);
            return {left, node};
        }

        if(getNodeSize(node->left) + 1 == size) {
            Pointer<Node> right = node->left;
            node->left = nullptr;
            update(node);
            return {node, right};
        }

        if(size < getNodeSize(node->left)) {
            auto [left, right] = splitSize(node->left, size);
            node->left = right;
            update(node);
            return {left, node};
        }

        auto [left, right] = splitSize(node->right, size - getNodeSize(node->left) - 1);
        node->right = left;
        update(node);
        return {node, right};
    }

    [[nodiscard]]
    Pointer<Node> merge(Pointer<Node> left, Pointer<Node> right)
    {
        if(!left)  return right;
        if(!right) return left;
        if(left->priority_ > right->priority_){
            left->right = merge(left->right, right);
            update(left);
            return left;
        }
        right->left = merge(left, right->left);
        update(right);
        return right;
    }

    [[nodiscard]]
    Pointer<Node> createNode(T key) {
        return Pointer<Node>(new Node(mgk::move(key)));
    }

    void deleteNode(Pointer<Node> node) {
        if constexpr (std::is_same<Pointer<Node>, Node*>::value) {
            delete node;
        }
    }

    T& operator[](size_t i) const {
        Pointer<Node> node = root_;
        assert(node);
        assert(node->size_ > i);
        while(1) {
            if(getNodeSize(node->left) == i) {
                return node->key;
            }
            if(i < getNodeSize(node->left)) {
                node = node->left;
            } else {
                i -= getNodeSize(node->left) + 1;
                node = node->right;
            }
        }
    }

    Pointer<Node> getRoot() { return root_;}
    void setRoot(Pointer<Node> root) { root_ = root;}

    size_t getNodeSize(Pointer<Node> node) const {return node ? node->size_ : 0;}

    private:
        void update(Pointer<Node> node) {
            if(!node) return;
            node->size_ =  1 + (node->right ? node->right->size_ : 0) +
                               (node->left  ? node->left ->size_ : 0);
            if constexpr (!std::is_same<Updater, void(*)()>::value) {
                if(updatef_) updatef_(&*node);
            }
        }


    private:
        Pointer<Node> root_ = nullptr;
        Updater updatef_;
    };
    
}

#endif /* MGKTL_MCONTAINERS_TREAP_HPP */
