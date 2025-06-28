#include "lexer.cpp"
#include <iostream>
#include <ostream>
int main(int argc,char**argv) {
    if (argc!=2) return 1;
    JsonLexer l(argv[1]);
    Token t;
    t.ttype=TokenType::IDENTIFIER;
    while (t.ttype!=TokenType::TK_EOF) {
        t=l.gettoken();
        std::cout<<t<<std::endl;
    }
}
