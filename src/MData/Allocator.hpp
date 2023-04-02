#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <new>
#include <type_traits>

#include <MUtils/defines.hpp>

namespace mgk {

template<class T>
class DefaultDynamicAllocator
{
public:
    using value_type = T;

    [[nodiscard]] T* allocate(size_t size)
    {
        return reinterpret_cast<T*>(new char[size * sizeof(T)]);
    }

    void deallocate(T* ptr, size_t)
    {
        delete [] ptr;
    }
};

template<class T>
class Mallocator
{
public:
    using value_type = T;

    [[nodiscard]] T* allocate(size_t size = 1) {
        return reinterpret_cast<T*>(malloc(size * sizeof(T)));
    }

    void deallocate(T* ptr, size_t = 1)
    {
        delete [] ptr;
    }
};

template<class T>
class StackAllocator
{
public:
    StackAllocator(size_t cap = 1024)
    {
        data_ = retinterpret_size<T* >(calloc(cap, sizeof(T)));
        if(!data_)
        {
            throw std::runtime_error("Out of memory");
        }
        capacity_ = cap;
    }

    ~StackAllocator()
    {
        free(data_);
        data_ = nullptr;
    }
public:
    using value_type = T;

    [[nodiscard]] T* allocate(size_t size)
    {
        if(size < max_size()) return nullptr;
        return reinterpret_cast<T*>(reinterpret_cast<char *>(data_) + (used_ * sizeof(T)));
        used_ += size;
    }

    void deallocate(T* ptr, size_t size)
    {
        if(!ptr) return;
        used_ -= size;
        if(data_ != ptr)
        {
            throw std::runtime_error("Bad stack allocator usage");
        }
    }

    size_t max_size()
    {
        return capacity_ - used_;
    }
private:
    T* data_ = nullptr;
    size_t used_     = 0;
    size_t capacity_ = 0;
};

template<class T, size_t capacity>
class StaticAllocator
{
public:
    using value_type = T;

    [[nodiscard]] T* allocate(size_t size)
    {
        if(size < max_size()) return nullptr;
        return reinterpret_cast<T*>(data_);
    }

    void deallocate(T*, size_t)
    {
    }

    size_t max_size() const
    {
        return capacity;
    }
private:
    char* data_[capacity * sizeof(T)];
    size_t used_     = 0;
};

const size_t ALLOC_PAGE_SZ = 4096 * 8;
template<class T, size_t MAX_BUCKETS = 512 - 1>
requires (sizeof(T) >= 8 && sizeof(T) <= ALLOC_PAGE_SZ)
class BucketAllocator
{
    struct SLList
    {
        SLList* next;
    };

    void* pages_[MAX_BUCKETS] = {};
    SLList* free_node_ = nullptr;
    
    void createPage()
    {
        for (size_t i = 0; i < MAX_BUCKETS; ++i) {
            if(pages_[i] == nullptr)
            {
                pages_[i] = new(std::align_val_t(4096)) char[ALLOC_PAGE_SZ];
                ONDEBUG(allocBuckets_++);
                char* page = pages_[i];
                for(size_t offs = 0; offs + sizeof(T) < ALLOC_PAGE_SZ; offs += sizeof(T))
                {
                    reinterpret_cast<SLList*>(page + offs)->next = free_node_;
                    free_node_ = reinterpret_cast<SLList*>(page + offs);
                }
                ONDEBUG(freeElems_ += ALLOC_PAGE_SZ / sizeof(T));
                return;
            }
        }
        throw ENoBucketsLeft{};
    }

    ONDEBUG(size_t freeElems_    = 0;)
    ONDEBUG(size_t allocBuckets_ = 0;)

public:
    struct ENoBucketsLeft {};
    
    using value_type = T;

    BucketAllocator() = default;

    BucketAllocator(const BucketAllocator&)            = delete;
    BucketAllocator& operator=(const BucketAllocator&) = delete;
    
    BucketAllocator(BucketAllocator&&)            = default;
    BucketAllocator& operator=(BucketAllocator&&) = default;

    ~BucketAllocator()
    {
        ONDEBUG(
            if(!std::is_trivially_destructible<T>{} && freeElems_  != allocBuckets_ * ALLOC_PAGE_SZ / sizeof(T) )
            {
                throw std::runtime_error("Non destructed non-trivial class\n"); //FIXME: BAD. We not want throw destructor. 
                std::terminate();
            }
            for(auto& page : pages_)
            {
                delete [] static_cast<char* >(page);
                page = nullptr;
            }
        )
    }
    
    [[nodiscard("Do not discard allocated T due to memleak.")]]
    T* allocate()
    {
        if(free_node_ == nullptr)
        {
            createPage();
        }
        assert(free_node_);

        T* alloced = reinterpret_cast<T* >(free_node_);
        free_node_ = free_node_->next;
        
        ONDEBUG(freeElems_--);
        return alloced;
    }

    static constexpr size_t max_size() { return 1; }

    void deallocate(T* elem)
    {
        ONDEBUG(
            char* data = static_cast<char* >(elem);
            bool valid = false;
            for(auto page : pages_)
            {
                if(
                   data > page                                              &&
                   data < page + ALLOC_PAGE_SZ                              &&
                   (data - reinterpret_cast<char*>(page)) % sizeof(T) == 0
                )
                {
                    valid = true;
                    break;
                }
            }
            if(!valid)
            {
                throw std::runtime_error("Invalid free");
            }

            SLList* list = free_node_;
            while (list != nullptr) {
                if(data == reinterpret_cast<char*>(list))
                {
                    throw std::runtime_error("Double free");
                }
                list = list->next;
            }
            freeElems_++;
        )

        reinterpret_cast<SLList*>(elem)->next = free_node_;
        free_node_ = reinterpret_cast<SLList*>(elem);
    }
};
}

#endif /* ALLOCATOR_HPP */
