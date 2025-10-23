#include <cstdint>
#include <ostream>
#include <string>


enum class TokenType {
    TK_ERR,
    KEYWORD,
    PUNCTUATOR,
    STRING,
    NUMBER,
    TK_EOF
};

struct Token {
    TokenType ttype;
    std::string value;
    int64_t lineno;
    int64_t startindex;
    const std::string* line=nullptr;
};

std::string tokenToString(const Token& tok) {
    std::string typeStr;
    switch (tok.ttype) {
        case TokenType::KEYWORD:    typeStr = "KEYWORD";    break;
        case TokenType::PUNCTUATOR: typeStr = "PUNCTUATOR"; break;
        case TokenType::STRING:     typeStr = "STRING";     break;
        case TokenType::NUMBER:     typeStr = "NUMBER";     break;
        case TokenType::TK_EOF:     typeStr = "EOF";        break;
        case TokenType::TK_ERR:     typeStr = "ERROR";        break;
        default:                    typeStr = "UNKNOWN";    break;
    }
    std::string lineExcerpt = (tok.line && !tok.line->empty()) ? *tok.line : "<no line>";

    return "Token(" + typeStr +", value=" + tok.value+')';
}
std::ostream& operator<<(std::ostream& out,Token t) {
    out<<tokenToString(t);
    return out;
}
