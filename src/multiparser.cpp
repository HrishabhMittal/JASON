#include "lexer.cpp"
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
enum ValueType {
    ARRAY,
    OBJECT,
    STRING,
    NUMBER,
    TRUE,
    FALSE,
    VT_NULL
};
std::string repeat(const std::string& s,int i) {
    std::string str;
    for (int j=0;j<i;j++) str+=s;
    return str;
}
struct Value {
    ValueType value;
    std::string str;
    double num;
    std::vector<Value> arr;
    std::unordered_map<std::string,Value> obj;
};
double to_number(const std::string& str) {
    try {
        size_t idx = 0;
        double val = std::stod(str, &idx);
        if (idx != str.size()) {
            throw std::invalid_argument("Invalid number: " + str);
        }
        return val;
    } catch (...) {
        throw std::runtime_error("Invalid number: " + str);
    }
}
class JSONParser;
class JSON {
    std::vector<Token> tokens;
public:
    JSON(const std::string& filename) {
        std::ifstream file(filename);
        JsonLexer jl(file);
        Token v=jl.gettoken();
        while (v.ttype!=TokenType::TK_EOF) {
            tokens.push_back(v);
            v=jl.gettoken();
        }
    }
    Value parse();
};
class JSONParser {
    std::vector<Token>& tokens;
    int start,end,cur;
    Token currentToken;
    Token gettoken() {
        if (cur<=end) return tokens[cur++];
        return {TokenType::TK_EOF};
    }
    Token peektoken() {
        if (cur<=end) return tokens[cur];
        return {TokenType::TK_EOF};
    }
    void next() {
        currentToken = gettoken();
    }
    bool matchnext(TokenType type, const std::string& val = "") {
        Token x=peektoken();
        if (x.ttype==TokenType::TK_EOF&&type==TokenType::TK_EOF) return 1;
        return x.ttype == type && (val.empty() || x.value == val);
    }
    bool match(TokenType type, const std::string& val = "") {
        return currentToken.ttype == type && (val.empty() || (!currentToken.value.empty() && currentToken.value == val));
    }
    Token expect(TokenType type, const std::string& val = "") {
        if (!match(type, val)) {
            if (currentToken.line!=nullptr) {
            throw std::runtime_error("Unexpected token: \""+currentToken.value+"\", at line: "+std::to_string(currentToken.lineno)+
                    "\n    "+*currentToken.line+
                    "\n    "+repeat(" ",currentToken.startindex)+repeat("^", currentToken.value.size()));
            } else {
                throw std::runtime_error("Unexpected token: \""+currentToken.value+"\", at line: "+std::to_string(currentToken.lineno));
            }
        }
        Token tok = currentToken;
        next();
        return tok;
    }
    void error(std::string s) {
        throw std::runtime_error(s);
    }
    Value JSONObject() {
        Value v;
        v.value=OBJECT;
        expect(TokenType::PUNCTUATOR,"{");
        if (match(TokenType::PUNCTUATOR,"}")) {
            expect(TokenType::PUNCTUATOR,"}");
            return v;
        }
        while (true) {
            Token t=expect(TokenType::STRING);
            expect(TokenType::PUNCTUATOR,":");
            v.obj[t.value]=JSONValue();
            if (match(TokenType::PUNCTUATOR,",")) expect(TokenType::PUNCTUATOR,",");
            else break;
        }
        expect(TokenType::PUNCTUATOR,"}");
        return v;
    }
    Value JSONArray() {
        Value v;
        v.value=ARRAY;
        expect(TokenType::PUNCTUATOR,"[");
        if (match(TokenType::PUNCTUATOR,"]")) {
            expect(TokenType::PUNCTUATOR,"]");
            return v;
        }
        while (true) {
            v.arr.push_back(JSONValue());
            if (match(TokenType::PUNCTUATOR,",")) expect(TokenType::PUNCTUATOR,",");
            else break;
        }
        expect(TokenType::PUNCTUATOR,"]");
        return v;
    }
    Value JSONValue() {
        Value v;
        if (match(TokenType::STRING)) {
            Token t=expect(TokenType::STRING);
            v.value=STRING;
            v.str=t.value;
            return v;
        } else if (match(TokenType::NUMBER)) {
            Token t=expect(TokenType::NUMBER);
            v.value=NUMBER;
            v.num=to_number(t.value);
            return v;
        } else if (match(TokenType::PUNCTUATOR,"{")) {
            return JSONObject();
        } else if (match(TokenType::PUNCTUATOR,"[")) {
            return JSONArray();
        } else if (match(TokenType::KEYWORD)) {
            Token t=expect(TokenType::KEYWORD);
            if (t.value=="null") {
                v.value=VT_NULL;
            } else if (t.value=="true") {
                v.value=TRUE;
            } else if (t.value=="false") {
                v.value=FALSE;
            } else throw std::runtime_error("unknown keyword??? how???");
            return v;
        } else if (match(TokenType::TK_ERR)) {
            throw std::runtime_error("Error token found");
        }
        Token t=gettoken();
        throw std::runtime_error("Unexpected value found: "+tokenToString(t));
    }
public:
    JSONParser(std::vector<Token>& ref,int start,int end): start(start),end(end),tokens(ref),cur(start) {
        
    }
    Value parse() {
        return JSONValue();
    }
};

Value JSON::parse() {
    JSONParser jp(tokens,0,tokens.size());
    return jp.parse();
}
