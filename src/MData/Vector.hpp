#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cstddef>
#include <iterator>
#include <utility>
#include <cassert>
#include <concepts>
#include <new>
#include "Allocator.hpp"

namespace mgk {

template<class T, class Allocator = DefaultDynamicAllocator<T>>
requires std::destructible<T> 
class Vector;


template<class T>
struct RAConstIterator
    {

        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;
        
        RAConstIterator() = default;

        const T& operator*() const
        {
            validateThrow();
            return (*container_)[position_];
        }

        RAConstIterator& operator+=(ptrdiff_t diff)
        {
            position_ += diff;
            validateThrow();
            return *this;
        }

        RAConstIterator& operator-=(ptrdiff_t diff)
        {
            return *this += -diff;
        }

        RAConstIterator& operator++()
        {
            position_++;
            validateThrow();
            return *this;
        }

        RAConstIterator& operator--()
        {
            position_--;
            validateThrow();
            return *this;
        }

        RAConstIterator operator++(int)
        {
            RAConstIterator copy = *this;
            position_++;
            validateThrow();
            return copy;
        }

        RAConstIterator operator--(int)
        {
            RAConstIterator copy = *this;
            position_--;
            validateThrow();
            return copy;
        }   



        std::strong_ordering operator<=>(const RAConstIterator& other) const
        {
            if(container_ != other.container_) throw Vector<T>::Error::DifferentContainerIterator;
            return position_ <=> other.position_;
        }

        bool operator==(const RAConstIterator& ) const = default;
        bool operator!=(const RAConstIterator& ) const = default;

        ptrdiff_t operator-(const RAConstIterator& other) const
        {
            if(container_ != other.container_) throw Vector<T>::Error::DifferentContainerIterator;
            return position_ - other.position_;
        }

        RAConstIterator operator+(ptrdiff_t diff) const
        {
            return RAConstIterator(*this) += diff;
        }

        RAConstIterator operator-(ptrdiff_t diff) const
        {
            return RAConstIterator(*this) -= diff;
        }

        const T& operator[](ptrdiff_t diff) const
        {
            return container_->operator[](position_ + diff);
        }

    protected:
        friend class Vector<T>;

        RAConstIterator(const Vector<T>* container, size_t position) : container_(container), position_(position) {}

        void validateThrow() const
        {
            if(position_ > container_->size()) // Also checks overflow below zero
            {
                throw Vector<T>::Error::OutOfRange;
            }
        }

        const Vector<T>* container_ = nullptr;
        size_t position_;
    };

template<class T>
requires std::destructible<T>
struct RAIterator : public RAConstIterator<T>
{
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;
    
    RAIterator() = default;

    T& operator*() const
    {
        return const_cast<T&>(RAConstIterator<T>::operator[]);
    }

    RAIterator& operator+=(ptrdiff_t diff)
    {
        RAConstIterator<T>::operator+=(diff);
        return *this;
    }

    RAIterator& operator-=(ptrdiff_t diff)
    {
        return *this += -diff;
    }

    RAIterator& operator++()
    {
        RAConstIterator<T>::operator++();
        return *this;
    }

    RAIterator& operator--()
    {
        RAConstIterator<T>::operator--();
        return *this;
    }

    RAIterator operator++(int)
    {
        RAIterator copy = *this;
        RAConstIterator<T>::operator++();
        return copy;
    }

    RAIterator operator--(int)
    {
        RAIterator copy = *this;
        RAConstIterator<T>::operator--();
        return copy;
    }  

    RAIterator operator-(ptrdiff_t diff) const
    {
        return RAIterator(*this) -= diff;
    }

    ptrdiff_t operator-(const RAIterator& other) const
    {
        return RAConstIterator<T>::operator-(other);
    }

    T& operator[](ptrdiff_t diff) const
    {
        return const_cast<T&>(RAConstIterator<T>::operator[](diff));
    }

private:
    friend class Vector<T>;
};



template<class T, class Allocator>
requires std::destructible<T> 
class Vector
{
public:
    enum class Error
    {
        Ok,
        OutOfRange,
        OutOfMemory,
        BadObject,
        DifferentContainerIterator,
    };

    Vector() {}
    
    Vector(size_t n)
    {
        resize(n);
    }

    Vector(size_t n, const T& fill)
    {
        assign(n, fill);
    }
    
    Vector& operator=(const Vector& oth)
    {
        reserve(oth.size_);
        clean();
        size_ = oth.size_;
        copyFrom_(oth.data_);
        return *this;
    }
    
    Vector(const Vector& oth)
    {
        *this = oth;
    }

    Vector& operator=(Vector&& oth)
    {
        swap(oth);
        return *this;
    }
    
    Vector(Vector&& oth)
    {
        *this = std::move(oth);
    }

    ~Vector() noexcept(true)
    {
        eraseData_(data_, size_);
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = size_ = 0;
    }

    void swap(Vector<T>& other)
    {
        other.validateThrow();
        validateThrow();

        std::swap(data_    , other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_    , other.size_);
    }

    size_t size() const { return size_; }

    bool validate() const noexcept(true)
    {
        return 
            ((data_ != nullptr) || (size_ == 0 && capacity_ == 0)) &&
            capacity_ >= size_;
    }

    void validateThrow() noexcept(false)
    {
        if(!validate()) throw Error::BadObject;
    }

    void reserve(size_t newCapacity)
    {
        if(capacity_ > newCapacity) return;
        
        T* newData_  = allocator_.allocate(newCapacity);

        if(!newData_)
        {
            throw Error::OutOfMemory;
        }

        moveTo_(newData_);

        allocator_.deallocate(data_, capacity_);
        data_ = newData_;

        capacity_ = newCapacity;
    }

    void resize(size_t newSize, const T& fill)
    {
        if(size_ >= newSize)
        {
            eraseData_(data_ + newSize, size_ - newSize);
            size_ = newSize;
            return;
        }
        
        if(newSize > capacity_)
        {
            reserve(newSize);
        }

        fillData_(data_ + size_, newSize - size_, fill);
    }

    void resize(size_t newSize)
    {
        if(size_ >= newSize)
        {
            eraseData_(data_ + newSize, size_ - newSize);
            size_ = newSize;
            return;
        }
        
        if(newSize > capacity_)
        {
            reserve(newSize);
        }

        fillDataDefault_(data_ + size_, newSize - size_);
    }

    void assign(size_t n, const T& fill)
    {
        clean();
        resize(n, fill);
    }

    void clean() { resize(0);}

    bool empty() const {return size_ == 0;}

    const T& operator[](size_t i) const 
    {
        if(i > size_)
        {
            throw Error::OutOfRange;
        }
        return data_[i];
    }

    T& operator[](size_t i) 
    {
        if(i > size_)
        {
            throw Error::OutOfRange;
        }
        return data_[i];
    }

    void push_back(const T& t)
    {
        if(size_ == capacity_)
        {
            expand_();
        }

        try 
        {
            new (&data_[size_]) T(t);
        } catch (...)
        {
            /* Really do nothing */
            throw;
        }
        size_++;
    }

    RAIterator<T> begin() { return RAIterator<T>(this, 0); }
    RAIterator<T> end() { return RAIterator<T>(this, size_); }

    RAConstIterator<T> begin() const { return RAConstIterator<T>(this, 0); }
    RAConstIterator<T> end() const { return RAConstIterator<T>(this, size_); }

    std::reverse_iterator<RAIterator<T>> rbegin() { return std::reverse_iterator<RAIterator<T>>(end()); }
    std::reverse_iterator<RAIterator<T>> rend() { return std::reverse_iterator<RAIterator<T>>(begin()); }

    std::reverse_iterator<RAConstIterator<T>> rbegin() const { return std::reverse_iterator<RAConstIterator<T>>(end()); }
    std::reverse_iterator<RAConstIterator<T>> rend() const   { return std::reverse_iterator<RAConstIterator<T>>(begin()); }
    
    /**
     * @brief Comarasion operator is deleted by design. We are not allowing this implict opetaion. 
     * 
     */
    bool operator==(const Vector&) = delete;
private:
    T* data_ = nullptr;

    size_t size_     = 0;
    size_t capacity_ = 0;

    Allocator allocator_;

    void moveTo_(T* newData)
    {
        assert(newData != nullptr);

        for(size_t i = 0; i < size_; ++i)
        {
            try
            {
                new(&newData[i]) T(std::move(data_[i]));
            } catch (...) {
                size_ = i;
                this->~Vector();
                throw;
            }
        }
    }

    void fillData_(T* data, size_t n, const T& fillElem)
    {
        assert(data != nullptr || n == 0);
        for(size_t i = 0; i < n; ++i)
        {   
            try { 
                new(&data[i]) T(fillElem);
            } catch(...) {
                size_ = i;
                this->~Vector();
                throw;
            }
        }
    }

    void fillDataDefault_(T* data, size_t n) 
    {
        assert(data != nullptr || n == 0);
        for(size_t i = 0; i < n; ++i)
        {
            try {
                new(&data[i]) T;
            } catch (...) {
                size_ = i;
                this->~Vector();
                throw;
            }
        }
    }

    void eraseData_(T* data, size_t n) noexcept(true)
    {
        assert(data != nullptr || n == 0);

        for(size_t i = 0; i < n; ++i)
        {
            data[i].~T();
        }
    }

    void copyFrom_(T* othData)
    {
        assert(othData != nullptr || size_ == 0);
        for(size_t i = 0; i < size_; ++i)
        {
            try
            {
                new(&data_[i]) T(othData[i]);
            }
            catch(...)
            {
                size_ = i;
                this->~Vector();
                throw;
            }
        }
    }

    void expand_()
    {
        reserve(2 * size_ + 1);
    }
};
template<class T>
RAIterator<T> operator+(const RAIterator<T>& iter, ptrdiff_t diff)
{
    return RAIterator<T>(iter) += diff;
}

template<class T>
RAIterator<T> operator+(ptrdiff_t shift, const RAIterator<T>& other)
{
    return other + shift;
}

template<class T>
RAConstIterator<T> operator+(ptrdiff_t shift, const RAConstIterator<T>& other)
{
    return other + shift;
}

}
#endif /* VECTOR_HPP */
