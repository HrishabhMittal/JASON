#include "parser.cpp"
#include <iostream>
#include <ostream>

int main(int argc,char**argv) {
    if (argc!=2) return 1;
    try {
        JsonParser p(argv[1]);
        auto x=p.ParseJSON();
    } catch (...) {
        std::cout<<"parsing unsuccessful!"<<std::endl;
        return 1;
    }
    std::cout<<"parsing successful"<<std::endl;
}
