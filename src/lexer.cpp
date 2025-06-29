#include "header.hpp"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
const std::string JSON_SYMBOLS = "{}[]:,";
bool is_json_symbol(char c) {
    return JSON_SYMBOLS.find(c) != std::string::npos;
}
class JsonLexer {
    std::vector<std::string> data;
    int64_t pos_line = 0;
    int64_t pos_char = 0;
    bool eof=0;
public:
    JsonLexer(std::istream& file) {
        std::string line;
        while (std::getline(file, line)) {
            data.push_back(line);
        }
    }
    JsonLexer(const std::string& file) {
        std::string s;
        for (auto c:file) {
            if (c=='\n') {
                data.push_back(s);
                s="";
            } else s.push_back(c);
        }
        if (s!="") data.push_back(s);
    }
    Token peektoken() {
        int64_t saved_line = pos_line;
        int64_t saved_char = pos_char;
        Token token = gettoken();
        pos_line = saved_line;
        pos_char = saved_char;
        return token;
    }
    Token gettoken() {
        if (eof) return {TokenType::TK_EOF, "", pos_line + 1, pos_char, nullptr};
        while (pos_line < static_cast<int64_t>(data.size())) {
            std::string& line = data[pos_line];
            while (pos_char < static_cast<int64_t>(line.size()) &&
                   std::isspace(static_cast<unsigned char>(line[pos_char]))) {
                pos_char++;
            }
            if (pos_char >= static_cast<int64_t>(line.size())) {
                pos_line++;
                pos_char = 0;
                continue;
            }
            char c = line[pos_char++];
            const int64_t start_index = pos_char - 1;
            std::string value;
            if (c == '"') {
                value += c;
                bool escape = false;
                while (pos_char < static_cast<int64_t>(line.size())) {
                    char ch = line[pos_char++];
                    value += ch;
                    if (!escape) {
                        if ((unsigned char)ch < 0x20) {
                            return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                        }
                        if (ch == '\\') {
                            escape = true;
                        } else if (ch == '"') {
                            break;
                        }
                    } else {
                        if (ch == '"' || ch == '\\' || ch == '/' || ch == 'b' || ch == 'f' ||
                            ch == 'n' || ch == 'r' || ch == 't') {
                            escape = false;
                        } else if (ch == 'u') {
                            for (int i = 0; i < 4; ++i) {
                                if (pos_char >= static_cast<int64_t>(line.size()) ||
                                    !std::isxdigit(line[pos_char])) {
                                    return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                                }
                                value += line[pos_char++];
                            }
                            escape = false;
                        } else {
                            return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                        }
                    }
                }
                return {TokenType::STRING, value, pos_line + 1, start_index, &line};
            }
            if (c == '-' || std::isdigit(c)) {
                value += c;
                bool has_digit = (c != '-');
                bool dot = false;
                bool exp = false;
                bool exp_sign = false;
                bool exp_digit = false;
                size_t number_start = value.size() - 1;
                size_t digit_count = (c == '-' ? 0 : 1);
                if (c == '0') {
                    if (pos_char < static_cast<int64_t>(line.size())) {
                        char next = line[pos_char];
                        if (std::isdigit(next)) {
                            return {TokenType::TK_ERR, value + next, pos_line + 1, start_index, &line};
                        }
                    }
                }
                while (pos_char < static_cast<int64_t>(line.size())) {
                    char next = line[pos_char];
                    if (std::isdigit(next)) {
                        value += next;
                        pos_char++;
                        has_digit = true;
                        if (exp) exp_digit = true;
                        digit_count++;
                    }
                    else if (next == '.' && !dot && !exp) {
                        value += next;
                        pos_char++;
                        dot = true;
                        if (pos_char >= static_cast<int64_t>(line.size()) || !std::isdigit(line[pos_char])) {
                            return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                        }
                    }
                    else if ((next == 'e' || next == 'E') && !exp) {
                        value += next;
                        pos_char++;
                        exp = true;
                        exp_sign = false;
                        exp_digit = false;
                        if (pos_char < static_cast<int64_t>(line.size()) && (line[pos_char] == '+' || line[pos_char] == '-')) {
                            value += line[pos_char++];
                            exp_sign = true;
                        }
                        if (pos_char >= static_cast<int64_t>(line.size()) || !std::isdigit(line[pos_char])) {
                            return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                        }
                    }
                    else {
                        break;
                    }
                }
                if (exp && !exp_digit) {
                    return {TokenType::TK_ERR, value, pos_line + 1, start_index, &line};
                }
                if (has_digit) {
                    return {TokenType::NUMBER, value, pos_line + 1, start_index, &line};
                }
            }
            if (std::isalpha(c)) {
                value += c;
                while (pos_char < static_cast<int64_t>(line.size()) && 
                       std::isalpha(line[pos_char])) {
                    value += line[pos_char++];
                }
                return find_json_token(value, pos_line + 1, start_index, &line);
            }
            if (is_json_symbol(c)) {
                value += c;
                return {TokenType::PUNCTUATOR, value, pos_line + 1, start_index, &line};
            }
            return {TokenType::TK_ERR, std::string(1, c), pos_line + 1, start_index, &line};
        }
        eof=true;
        return {TokenType::TK_EOF, "", pos_line + 1, pos_char, nullptr};
    }
private:
    Token find_json_token(const std::string& s, int64_t line, int64_t index, const std::string* line_text) {
        if (s == "true" || s == "false" || s == "null") {
            return {TokenType::KEYWORD, s, line, index, line_text};
        }
        return {TokenType::TK_ERR, s, line, index, line_text};
    }
};
