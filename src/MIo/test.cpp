#include "print.hpp"
#include "stream.hpp"
#include <MUtils/function.hpp>

static void F(mgk::UniqueFunction<void(void)> f){
    f();
}

static void v(){
    mgk::out << "Lel";
}

int main()
{
    F([]{mgk::out << "F\n";});
    F(v);
    mgk::out << "Hello, world " << nullptr <<  mgk::endl;
    mgk::out << -123 << ' ' <<  &mgk::out << " 0" << mgk::format::oct{1024} << '\n';
    mgk::print("Abooba %$ %% %$\n", "Kekw", mgk::format::oct{1024});
}