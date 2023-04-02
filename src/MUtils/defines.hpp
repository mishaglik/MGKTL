#ifndef MUTILS_DEFINES_HPP
#define MUTILS_DEFINES_HPP


#ifndef NDEBUG
#define ONDEBUG(...) __VA_ARGS__
#else
#define ONDEBUG(...)
#endif


namespace mgk {
    using byte = char;
}
#endif /* MUTILS_DEFINES_HPP */
