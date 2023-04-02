#ifndef MGKTL_MDATA_POINTERS_HPP
#define MGKTL_MDATA_POINTERS_HPP
#include <cassert>
#include <cstddef>
#include <utility>
#include <MUtils/utils.hpp>
#include <MUtils/defines.hpp>
namespace mgk {

    template<class T>
    class UniquePtr {
    public:
        UniquePtr() = default;
        UniquePtr(T* t) : object_(t) {}
        
        UniquePtr operator=(std::nullptr_t) { delete object_; object_ = nullptr; }

        UniquePtr(const UniquePtr&) = delete;
        UniquePtr operator=(const UniquePtr&) = delete;


        UniquePtr(UniquePtr&& oth) : object_(oth.object_) { oth.object_ = nullptr; }
        UniquePtr operator=(UniquePtr&& oth) {std::swap(object_, oth.object_);}

        ~UniquePtr() {delete object_; object_ = nullptr;}

        T* release() {T* obj = object_; object_ = nullptr; return obj;}

        T& operator*() {return *object_;}
        const T& operator*() const {return *object_;}

        T* operator->() {return object_;}
        const T* operator->() const {return object_;}

        operator bool() const {return object_ != nullptr;}
    private:
        T* object_ = nullptr;
    };

    template<class T, class ...Args>
    UniquePtr<T> make_unique(Args&& ...args)
    {
        return UniquePtr<T>(new T(mgk::forward<Args>(args)...));
    }

    template<class T>
    class CringePtr {
    public:
        CringePtr() = default; 
        CringePtr(T* t) : object_(t), nRefs_(new size_t(1)) {}
        
        CringePtr(const CringePtr& oth) : object_(oth.object_), nRefs_(oth.nRefs_) { if(nRefs_) ++*nRefs_; }
        CringePtr operator=(const CringePtr& oth) { if(oth.object_ == object_) {return *this;} cleanup(); new(this) CringePtr(oth); return *this;}

        CringePtr(std::nullptr_t) : object_(nullptr), nRefs_(nullptr) {}
        CringePtr operator=(std::nullptr_t) { cleanup(); object_ = nullptr; nRefs_ = nullptr; return *this;}

        CringePtr(CringePtr&& oth) : object_(oth.object_), nRefs_(oth.nRefs_) { oth.object_ = nullptr; oth.nRefs_ = nullptr; }
        CringePtr operator=(CringePtr&& oth) {std::swap(object_, oth.object_); std::swap(nRefs_, oth.nRefs_); return *this;}

        ~CringePtr() {cleanup();}

        T* release() {T* obj = object_; object_ = nullptr; return obj;}

        T& operator*() {return *object_;}
        const T& operator*() const {return *object_;}

        T* operator->() {return object_;}
        const T* operator->() const {return object_;}

        operator bool() const {return object_ != nullptr;}

    private:
        
        template<class ...Args>
        friend CringePtr<T> make_cringe(Args&& ...args);
         
        CringePtr(T* obj, std::size_t* cnt) : object_(obj), nRefs_(cnt) {*nRefs_ = 1ul | ~ValueMask;}

        T* object_ = nullptr;
        std::size_t* nRefs_ = nullptr;
        const std::size_t ValueMask = 0x7FFF'FFFF'FFFF'FFFFul;

        void cleanup() {
            if(nRefs_ == nullptr) {
                assert(object_ == nullptr);
                return;
            }
            if(!(--*nRefs_ & ValueMask)) {
                if(*nRefs_ & ~ValueMask){
                    object_->~T();
                    delete [] reinterpret_cast<mgk::byte* >(object_);
                    object_ = nullptr;
                    nRefs_ = nullptr;
                    return;
                }
                delete object_;
                delete nRefs_;
                object_ = nullptr;
                nRefs_ = nullptr;
            }
        }
    };

    template<class T, class ...Args>
    CringePtr<T> make_cringe(Args&& ...args)
    {
        byte* data = new byte[sizeof(T) + sizeof(size_t)];
        new(data) T(mgk::forward<Args>(args)...);
        return CringePtr<T>(data, data + sizeof(T));
    }
}

#endif /* MGKTL_MDATA_POINTERS_HPP */
