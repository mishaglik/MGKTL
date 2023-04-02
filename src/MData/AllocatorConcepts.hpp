#ifndef MGKTL_MDATA_ALLOCATORCONCEPTS_HPP
#define MGKTL_MDATA_ALLOCATORCONCEPTS_HPP

#include <type_traits>
#include <concepts>
namespace mgk {

template<typename Allocator>
struct AllocatorTraits
{
    using value_type = typename Allocator::value_type;
    using pointer_type = value_type*;
};


template<typename T, template<typename> class Allocator>
concept SingularAllocator = requires(Allocator<T> allocator, typename AllocatorTraits<Allocator<T>>::pointer_type ptr)
{
    {allocator.allocate()} -> std::convertible_to<typename AllocatorTraits<Allocator<T>>::pointer_type>;
    allocator.deallocate(ptr);
};

template<typename T, template<typename> class Alloc>
concept Allocator = SingularAllocator<T, Alloc> &&
requires(Alloc<T> allocator, typename AllocatorTraits<Alloc<T>>::pointer_type ptr, std::size_t sz)
{
    {allocator.allocate(sz)} -> std::convertible_to<typename AllocatorTraits<Alloc<T>>::pointer_type>;
    allocator.deallocate(ptr, sz);
};

}
#endif /* MGKTL_MDATA_ALLOCATORCONCEPTS_HPP */
