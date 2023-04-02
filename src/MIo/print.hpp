#ifndef MIO_PRINT_HPP
#define MIO_PRINT_HPP

#include <stdexcept>
#include <utility>
#include <MUtils/utils.hpp>
#include "stream.hpp"
namespace mgk {

void print(const char* format);

template<typename T, typename ...Ts>
void print(const char* format, T&& arg1, Ts&& ...args)
{

    while (*format != '\0' && *(format++) != '%')
    {
        mgk::out << *format;
    }


    while (1) {
    
        switch (*(format++)) {
            case '\0':
                throw std::runtime_error("Too many print arguments");   
            case 'd':
            case 'f':
            case 'g':
            case 'o':
            case 'p':
            case 's':
            case 'u':
            case 'x':
            case '$':
                mgk::out << arg1;
                print(format, mgk::forward<Ts>(args)...);
                return;
            case '%':
                mgk::out << '%';
                print(format, mgk::forward<T>(arg1), mgk::forward<Ts>(args)...);
                return;
            default:
                break;
        } 
    }
}

}

#endif /* MIO_PRINT_HPP */
