#include "Allocator.hpp"
#include "BitArray.hpp"
#include <bits/iterator_concepts.h>
#include <iostream>
#include "MData/Pointers.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <list>
#include <memory>

template<std::random_access_iterator Iter>
void check(Iter)
{
    return;
}

template<class T>
void kek(T t){
    t();
}

int main()
{
    mgk::BitArray v(10, 0);
    v[0] = true;
    v[1] = true;
    v[3] |= 1;
    if(v[4])
    {
        check(mgk::RAIterator<int>());
    }   

    for(auto x : v)
    {
        std::cout << (x ? "1" : "0");
    }
    std::unique_ptr<int> ptr = std::make_unique<int>(1);

    kek([ptr = std::move(ptr)]{});

    std::cout << '\n' << (std::find(v.begin(), v.end(), false) - v.begin()) << '\n';

    auto kk = mgk::make_unique<int>(0);
}
