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

public:
    JsonLexer(const std::string& filename) {
        std::ifstream infile(filename);
        std::string line;
        while (std::getline(infile, line)) {
            data.push_back(line);
        }
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

                    if (escape) {
                        if (ch == 'u') {
                            for (int i = 0; i < 4 && pos_char < static_cast<int64_t>(line.size()); ++i) {
                                value += line[pos_char++];
                            }
                        }
                        escape = false;
                    } else if (ch == '\\') {
                        escape = true;
                    } else if (ch == '"') {
                        break;
                    }
                }
                return {TokenType::STRING, value, pos_line + 1, start_index, &line};
            }

            if (c == '-' || std::isdigit(c)) {
                value += c;
                bool has_digit = (c != '-');
                bool dot = false;
                bool exp = false;

                while (pos_char < static_cast<int64_t>(line.size())) {
                    char next = line[pos_char];
                    
                    if (std::isdigit(next)) {
                        value += next;
                        pos_char++;
                        has_digit = true;
                    }
                    else if (next == '.' && !dot && !exp) {
                        value += next;
                        pos_char++;
                        dot = true;
                    }
                    else if ((next == 'e' || next == 'E') && !exp) {
                        value += next;
                        pos_char++;
                        exp = true;
                        if (pos_char < static_cast<int64_t>(line.size()) && 
                            (line[pos_char] == '+' || line[pos_char] == '-')) {
                            value += line[pos_char++];
                        }
                    }
                    else {
                        break;
                    }
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
