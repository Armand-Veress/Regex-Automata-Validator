//
// Created by arman on 6/6/2026.
//

#ifndef REGEX_VALIDATOR_LEXER_H
#define REGEX_VALIDATOR_LEXER_H

#include <string>
#include <vector>
#include <cctype>

enum class TokenType {
    LITERAL,
    NUMBER,
    UNION,
    CONCAT,
    STAR,
    PLUS,
    OPTIONAL,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    COMMA,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
private:
    std::string input;
    std::size_t idx;

    Token nextToken() {
        if (idx >= input.length()) {
            return {TokenType::END_OF_FILE, ""};
        }

        char c = input[idx];
        idx++;

        if (c == '(') {
            return {TokenType::LPAREN, std::string(1, c)};
        }
        if (c == ')') {
            return {TokenType::RPAREN, std::string(1, c)};
        }
        if (c == '*') {
            return {TokenType::STAR, std::string(1, c)};
        }
        if (c == '+') {
            return {TokenType::PLUS, std::string(1, c)};
        }
        if (c == '?') {
            return {TokenType::OPTIONAL, std::string(1, c)};
        }
        if (c == '|') {
            return {TokenType::UNION, std::string(1, c)};
        }
        if (c == '{') {
            return {TokenType::LBRACE, std::string(1, c)};
        }
        if (c == '}') {
            return {TokenType::RBRACE, std::string(1, c)};
        }
        if (c == ',') {
            return {TokenType::COMMA, std::string(1, c)};
        }
        if (c == '\\') {
            if (idx >= input.length()) {
                return {TokenType::LITERAL, "\\"};
            }
            char escapedChar = input[idx];
            idx++;
            return {TokenType::LITERAL, std::string(1, escapedChar)};
        }
        if (std::isdigit(c)) {
            std::string numStr(1, c);
            while (idx < input.length() && std::isdigit(input[idx])) {
                numStr += input[idx];
                idx++;
            }
            return {TokenType::NUMBER, numStr};
        }

        return {TokenType::LITERAL, std::string(1, c)};
    }

public:
    explicit Lexer(const std::string& input): input(input), idx(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (idx < input.length()) {
            if (input[idx] == '[') {
                idx++;
                std::vector<char> charactersInClass;

                while (idx < input.length() && input[idx] != ']') {
                    if (idx + 2 < input.length() && input[idx + 1] == '-') {
                        char start = input[idx];
                        char end = input[idx + 2];
                        idx += 3;

                        if (start <= end) {
                            for (char ch = start; ch <= end; ch++) {
                                charactersInClass.push_back(ch);
                            }
                        }
                    } else {
                        charactersInClass.push_back(input[idx]);
                        idx++;
                    }
                }

                if (idx < input.length() && input[idx] == ']') {
                    idx++;
                }

                if (!charactersInClass.empty()) {
                    tokens.push_back({TokenType::LPAREN, "("});
                    tokens.push_back({TokenType::LITERAL, std::string(1, charactersInClass[0])});

                    for (size_t k = 1; k < charactersInClass.size(); k++) {
                        tokens.push_back({TokenType::UNION, "|"});
                        tokens.push_back({TokenType::LITERAL, std::string(1, charactersInClass[k])});
                    }
                    tokens.push_back({TokenType::RPAREN, ")"});
                }
            } else {
                Token t = nextToken();
                if (t.type != TokenType::END_OF_FILE) {
                    tokens.push_back(t);
                }
            }
        }

        return tokens;
    }
};


#endif //REGEX_VALIDATOR_LEXER_H