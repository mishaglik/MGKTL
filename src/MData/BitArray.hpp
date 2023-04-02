#ifndef BITARRAY_HPP
#define BITARRAY_HPP
#include <bits/iterator_concepts.h>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <cassert>
#include <concepts>
#include <cinttypes>
#include <new>
#include <cstring>

namespace mgk {


class BitArray
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

    BitArray() {}
    
    BitArray(size_t n, bool value = false)
    {
        assign(n, value);
    }
    
    BitArray& operator=(const BitArray& oth)
    {
        reserve(oth.size_);
        size_ = oth.size_;
        return *this;
    }
    
    BitArray(const BitArray& oth)
    {
        *this = oth;
    }

    BitArray& operator=(BitArray&& oth)
    {
        swap(oth);
        return *this;
    }
    
    BitArray(BitArray&& oth)
    {
        *this = std::move(oth);
    }

    ~BitArray() noexcept(true)
    {
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = size_ = 0;
    }

    void swap(BitArray& other)
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
            ((data_ != nullptr) || (size_ == 0 && capacity_ == 0 && nBlocks_ == 0)) &&
            capacity_ >= size_;
    }

    void validateThrow() const noexcept(false)
    {
        if(!validate()) throw Error::BadObject;
    }

    void reserveBlocks(size_t newCapacity)
    {
        validateThrow();
        if(nBlocks_ > newCapacity) return;
        newCapacity = std::min(newCapacity, 8ul);
        uint64_t* newData_  = static_cast<uint64_t*>(::operator new(newCapacity * sizeof(uint64_t)));

        if(!newData_)
        {
            throw Error::OutOfMemory;
        }
        
        if(data_) {
            memcpy(newData_, data_, nBlocks_);
        }
        memset(newData_ + nBlocks_, 0, newCapacity - nBlocks_);

        ::operator delete(data_);
        data_ = newData_;

        capacity_ = 64 * newCapacity;
        nBlocks_ = newCapacity;
    }


    void reserve(size_t cap)
    {
        reserveBlocks(cap / 64 + 1);
    }

    void resize(size_t newSize, bool fill = false)
    {
        validateThrow();
        if(size_ >= newSize)
        {
            size_ = newSize;
            return;
        }
        
        if(newSize > capacity_)
        {
            reserve(newSize);
        }

        size_t lastBlock = size_ / 64;
        data_[lastBlock] &= (1ull << (size_ & 63)) - 1;

        if (fill)
        {
            data_[lastBlock] |= (1ull << (size_ & 63)) - 1;
        }

        for(size_t i = lastBlock; i <= newSize / 64; ++i)
        {
            data_[i] = fill ? ~0ull : 0ull;
        }

        size_ = newSize;
    }

    void assign(size_t n, bool fill)
    {
        clean();
        resize(n, fill);
    }

    void clean() { resize(0);}

    bool empty() const {return size_ == 0;}


    struct BitRef
    {
        BitRef(uint64_t* cell, uint64_t offset) : cell_(cell), mask_(1ull << offset) { assert(cell_ != nullptr); } 

        BitRef(const BitRef&) = default;
        BitRef(BitRef&&)      = default;

        BitRef& operator=(bool x)
        {
            if(x)
                *cell_ |= mask_;
            else
                *cell_ &= mask_;
            return *this;
        }

        BitRef& operator|=(bool x)
        {
            if(x) *cell_ |= mask_;
            return *this;
        }

        BitRef& operator&=(bool x)
        {
            if(!x) *cell_ &= ~mask_;
            return *this;
        }

        BitRef& operator^=(bool x)
        {
            if(x) *cell_ ^= mask_;
            return *this;
        }

        operator bool() const {return *cell_ & mask_;}

        BitRef& operator=(const BitRef& oth)
        {
            return *this = static_cast<bool>(oth);
        }

        bool operator==(const BitRef& other) const {return state() == other.state();}

        void swap(BitRef& other)
        {
            // std::swap(cell_, other.cell_);
            // std::swap(mask_, other.mask_);
            if(other.state() != state())
            {
                *this ^= 1;
                other ^= 1;
            }
        }

    private:
        uint64_t* cell_;
        uint64_t mask_;

        bool state() const
        {
            return (*cell_ & mask_);
        }
    };

    
    struct RAIterator
    {

        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        // using pointer = void;
        using reference = BitArray::BitRef;
        using iterator_category = std::random_access_iterator_tag;
        
        RAIterator() = default;

        BitArray::BitRef operator*() const
        {
            validateThrow();
            return (*container_)[position_];
        }

        RAIterator& operator+=(ptrdiff_t diff)
        {
            position_ += diff;
            validateThrow();
            return *this;
        }

        RAIterator& operator-=(ptrdiff_t diff)
        {
            return *this += -diff;
        }

        RAIterator& operator++()
        {
            position_++;
            validateThrow();
            return *this;
        }

        RAIterator& operator--()
        {
            position_--;
            validateThrow();
            return *this;
        }

        RAIterator operator++(int)
        {
            RAIterator copy = *this;
            position_++;
            validateThrow();
            return copy;
        }

        RAIterator operator--(int)
        {
            RAIterator copy = *this;
            position_--;
            validateThrow();
            return copy;
        }   



        std::strong_ordering operator<=>(const RAIterator& other) const
        {
            if(container_ != other.container_) throw BitArray::Error::DifferentContainerIterator;
            return position_ <=> other.position_;
        }

        bool operator==(const RAIterator& ) const = default;
        bool operator!=(const RAIterator& ) const = default;

        ptrdiff_t operator-(const RAIterator& other) const
        {
            if(container_ != other.container_) throw BitArray::Error::DifferentContainerIterator;
            return position_ - other.position_;
        }

        RAIterator operator+(ptrdiff_t diff) const
        {
            return RAIterator(*this) += diff;
        }

        RAIterator operator-(ptrdiff_t diff) const
        {
            return RAIterator(*this) -= diff;
        }

        BitRef operator[](ptrdiff_t diff) const
        {
            return container_->operator[](position_ + diff);
        }


    private:
        friend class BitArray;

        RAIterator(BitArray* container, size_t position) : container_(container), position_(position) {}

        void validateThrow() const
        {
            if(position_ > container_->size()) // Also checks overflow below zero
            {
                throw BitArray::Error::OutOfRange;
            }
        }

        BitArray* container_ = nullptr;
        size_t position_;
    };

    struct RAConstIterator : public RAIterator
    {

        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        // using pointer = void;
        // using reference = BitArray::BitRef;
        using iterator_category = std::random_access_iterator_tag;
        
        RAConstIterator() = default;

        bool operator*() const
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
            if(container_ != other.container_) throw BitArray::Error::DifferentContainerIterator;
            return position_ <=> other.position_;
        }

        bool operator==(const RAConstIterator& ) const = default;
        bool operator!=(const RAConstIterator& ) const = default;

        ptrdiff_t operator-(const RAConstIterator& other) const
        {
            if(container_ != other.container_) throw BitArray::Error::DifferentContainerIterator;
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

        bool operator[](ptrdiff_t diff) const
        {
            return container_->operator[](position_ + diff);
        }

    private:
        friend class BitArray;

        RAConstIterator(const BitArray* container, size_t position) : container_(container), position_(position) {}

        void validateThrow() const
        {
            if(!container_)
            {
                throw BitArray::Error::BadObject;
            }

            if(position_ > container_->size()) // Also checks overflow below zero
            {
                throw BitArray::Error::OutOfRange;
            }
        }

        const BitArray* container_ = nullptr;
        size_t position_;
    };

#if 0
    struct LegacyIterator : private RAIterator, public std::iterator<std::random_access_iterator_tag, bool, ptrdiff_t, void, BitRef>
    {
        using difference_type = ptrdiff_t;
        using value_type = bool;

        LegacyIterator() : RAIterator() {}

        LegacyIterator(const RAIterator& iter) : RAIterator(iter) {} 

        BitArray::BitRef operator*() const
        {
            return RAIterator::operator*();
        }

        LegacyIterator& operator+=(ptrdiff_t diff)
        {
            RAIterator::operator+=(diff);
            return *this;
        }

        LegacyIterator& operator-=(ptrdiff_t diff)
        {
            return *this += -diff;
        }

        LegacyIterator& operator++()
        {
            RAIterator::operator++();
            return *this;
        }

        LegacyIterator& operator--()
        {
            RAIterator::operator--();
            return *this;
        }

        LegacyIterator operator++(int)
        {
            LegacyIterator copy = *this;
            RAIterator::operator++();
            return copy;
        }

        LegacyIterator operator--(int)
        {
            LegacyIterator copy = *this;
            RAIterator::operator++();
            return copy;
        }   



        std::strong_ordering operator<=>(const LegacyIterator& other) const
        {
            return RAIterator::operator<=>(other);
        }

        bool operator==(const LegacyIterator& other) const {return (*this <=> other) == std::strong_ordering::equal; }
        bool operator!=(const LegacyIterator& other) const = default;

        ptrdiff_t operator-(const LegacyIterator& other) const
        {
            return RAIterator::operator-(other);
        }

        LegacyIterator operator+(ptrdiff_t diff) const
        {
            return RAIterator(*this) += diff;
        }

        LegacyIterator operator-(ptrdiff_t diff) const
        {
            return RAIterator(*this) -= diff;
        }

        BitRef operator[](ptrdiff_t diff) const
        {
            return RAIterator::operator[](diff);
        }

    };
#endif
    bool operator[](size_t i) const 
    {
        validateThrow();
        if(i > size_)
        {
            throw Error::OutOfRange;
        }
        return data_[i / 64] & (1ull << (i & 63));
    }

    BitRef operator[](size_t i) 
    {
        validateThrow();
        if(i > size_)
        {
            throw Error::OutOfRange;
        }
        return BitRef(data_ + i / 64, i & 63);
    }

    void push_back(bool x)
    {
        validateThrow();
        if(size_ == capacity_)
        {
            expand_();
        }

        size_++;
        BitRef back(data_ + size_ / 64, size_ % 64);
        back = x;
    }


    RAIterator begin() {return RAIterator(this, 0);}
    RAIterator end() {return RAIterator(this, size_);}

    RAConstIterator begin() const {return RAConstIterator(this, 0);}
    RAConstIterator end() const {return RAConstIterator(this, size_);}

    std::reverse_iterator<RAIterator> rbegin() { return std::reverse_iterator<RAIterator>(end()); }
    std::reverse_iterator<RAIterator> rend() { return std::reverse_iterator<RAIterator>(begin()); }

    std::reverse_iterator<RAConstIterator> rbegin() const { return std::reverse_iterator<RAConstIterator>(end()); }
    std::reverse_iterator<RAConstIterator> rend() const   { return std::reverse_iterator<RAConstIterator>(begin()); }

    /**
     * @brief Comarasion operator is deleted by design. We are not allowing this implict opetaion. 
     */
    bool operator==(const BitArray&) = delete;

private:
    uint64_t* data_ = nullptr;

    size_t size_     = 0;
    size_t capacity_ = 0;
    size_t nBlocks_   = 0;

    void expand_()
    {
        reserve(2 * size_ + 1);
    }
};

[[maybe_unused]]
static inline BitArray::RAIterator operator+(ptrdiff_t i, const BitArray::RAIterator& iter)
{
    return iter + i;
}

// [[maybe_unused]]
// static inline BitArray::RAConstIterator operator+(ptrdiff_t i, const BitArray::RAConstIterator& iter)
// {
//     return iter + i;
// }

// [[maybe_unused]]
// static inline BitArray::LegacyIterator operator+(ptrdiff_t i, const BitArray::LegacyIterator& iter)
// {
//     return iter + i;
// }

// namespace std {

inline void swap(BitArray::BitRef b1, BitArray::BitRef b2)
{
    if(b1 != b2)
    {
        b1 ^= 1;
        b2 ^= 1;
    }
}
// }
}

#endif /* BITARRAY_HPP */
