#ifndef MUTILS_FUNCTION_HPP
#define MUTILS_FUNCTION_HPP
#include <MUtils/utils.hpp>
namespace mgk {

template<typename RetVal, typename... Args>
class CallableBase
{
public:
    CallableBase() = default;
    virtual ~CallableBase() noexcept(true) {};
    virtual RetVal operator()(Args...) = 0;
};

template<class FuncT, typename RetVal, typename... Args>
class Callable final : public CallableBase<RetVal, Args...>
{
public:

    Callable(FuncT func) : func_(move(func)) {}
    RetVal operator()(Args&& ...args) noexcept(noexcept(func_(args...))) override
    {
        return func_(forward<Args>(args)...);
    }
private:
    FuncT func_;
};

template<typename>
class UniqueFunction;

/**
 * @brief Unique function - non copyable callable object;
 * 
 * @tparam RetVal - return value
 * @tparam Args  - agrument of function.
 */
template<typename RetVal, typename... Args>
class UniqueFunction<RetVal (Args...)>
{
public:
    UniqueFunction() = default;
    ~UniqueFunction() { delete callable_; }

    // UniqueFunction(const UniqueFunction& oth) : callable_(new Callable(*oth.callable_)) {}
    // UniqueFunction& operator=(const UniqueFunction& oth) { delete callable_; callable_ = new Callable(*oth.callable_); }
// Non-copyable
    UniqueFunction(const UniqueFunction& oth)            = delete;
    UniqueFunction& operator=(const UniqueFunction& oth) = delete;

// Movalble
    UniqueFunction(UniqueFunction&& oth)            { std::swap(callable_, oth.callable_); }
    UniqueFunction& operator=(UniqueFunction&& oth) { std::swap(callable_, oth.callable_); }


    template<class FuncT>
    /* implict */ UniqueFunction(FuncT func) : callable_(new Callable<FuncT, RetVal, Args...>(move(func))) {}

    RetVal operator()(Args&& ...args) noexcept(noexcept((*callable_)(args...))) { return callable_->operator()(forward<Args>(args)...); }

private:
    CallableBase<RetVal, Args...>* callable_ = nullptr;
};

}

#endif /* MUTILS_FUNCTION_HPP */
