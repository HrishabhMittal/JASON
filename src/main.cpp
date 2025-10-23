#include "parser.cpp"
#include <fstream>
#include <iostream>
#include <ostream>

int main(int argc,char**argv) {
    if (argc!=2) return 1;
    std::ifstream file(argv[1]);
    JsonParser p(file);
    auto x=p.ParseJSON();
    if (x.value==INVALID) {
        std::cout<<"invalid"<<std::endl;
    } else std::cout<<"valid"<<std::endl;
}
