//
// Created by arman on 6/6/2026.
//

#ifndef REGEX_VALIDATOR_AST_H
#define REGEX_VALIDATOR_AST_H

#include "Lexer.h"

#include <stack>
#include <vector>
#include <string>
#include <sstream>

struct Node {
    Token token;
    Node* left = nullptr;
    Node* right = nullptr;

    Node(Token t) : token(t) {}

    ~Node() {
        delete left;
        delete right;
    }
};

class AST {
private:
    Node* cloneTree(Node* src) {
        if (src == nullptr) {
            return nullptr;
        }
        Node* dst = new Node(src->token);
        dst->left = cloneTree(src->left);
        dst->right = cloneTree(src->right);
        return dst;
    }

    bool shouldInsertConcat(TokenType left, TokenType right) {
        bool leftOk = (left == TokenType::LITERAL ||
                       left == TokenType::NUMBER ||
                       left == TokenType::RPAREN ||
                       left == TokenType::RBRACE ||
                       left == TokenType::STAR ||
                       left == TokenType::PLUS ||
                       left == TokenType::OPTIONAL);

        bool rightOk = (right == TokenType::LITERAL ||
                        right == TokenType::NUMBER ||
                        right == TokenType::LPAREN);

        return leftOk && rightOk;
    }

    int getPrecedence(TokenType type) {
        switch (type) {
            case TokenType::STAR:
            case TokenType::PLUS:
            case TokenType::OPTIONAL:
                return 3;
            case TokenType::CONCAT:
                return 2;
            case TokenType::UNION:
                return 1;
            default:
                return 0;
        }
    }

    std::vector<Token> addExplicitConcats(const std::vector<Token>& rawTokens) {
        std::vector<Token> tokens;
        if (rawTokens.empty()) {
            return tokens;
        }

        tokens.push_back(rawTokens[0]);
        Token last = rawTokens[0];

        for (size_t i = 1; i < rawTokens.size(); i++) {
            if (shouldInsertConcat(last.type, rawTokens[i].type)) {
                tokens.push_back({TokenType::CONCAT, "."});
            }
            tokens.push_back(rawTokens[i]);
            last = rawTokens[i];
        }
        return tokens;
    }

    void applyOperator(std::stack<Node*>& values, Token op) {
        if (values.empty()) return;

        Node* node = new Node(op);
        if (op.type == TokenType::PLUS || op.type == TokenType::OPTIONAL || op.type == TokenType::STAR) {
            node->left = values.top();
            values.pop();
        }
        else if (op.type == TokenType::CONCAT || op.type == TokenType::UNION) {
            if (values.size() < 2) return;
            node->right = values.top();
            values.pop();
            node->left = values.top();
            values.pop();
        }
        values.push(node);
    }

    Node* handleBraceExpansion(Node* target, const std::vector<Token>& tokens, size_t& i) {
        i++;
        int minBound = 0;
        int maxBound = -1;
        bool hasComma = false;

        if (tokens[i].type == TokenType::NUMBER) {
            minBound = std::stoi(tokens[i].value);
            i++;
        }
        if (tokens[i].type == TokenType::COMMA) {
            hasComma = true;
            i++;
            if (tokens[i].type == TokenType::NUMBER) {
                maxBound = std::stoi(tokens[i].value);
                i++;
            }
        }

        std::vector<Node*> parts;
        for (int k = 0; k < minBound; k++) {
            parts.push_back(cloneTree(target));
        }

        if (hasComma) {
            if (maxBound == -1) {
                Node* starNode = new Node({TokenType::STAR, "*"});
                starNode->left = cloneTree(target);
                parts.push_back(starNode);
            } else {
                for (int k = minBound; k < maxBound; k++) {
                    Node* optNode = new Node({TokenType::OPTIONAL, "?"});
                    optNode->left = cloneTree(target);
                    parts.push_back(optNode);
                }
            }
        }

        delete target;

        if (parts.empty()) {
            return new Node({TokenType::LITERAL, ""});
        }

        Node* result = parts[0];
        for (size_t k = 1; k < parts.size(); k++) {
            Node* concatNode = new Node({TokenType::CONCAT, "."});
            concatNode->left = result;
            concatNode->right = parts[k];
            result = concatNode;
        }

        return result;
    }

public:
    Node* parse(const std::string& regexStr) {
        Lexer lexer(regexStr);
        const std::vector<Token> rawTokens = lexer.tokenize();
        const std::vector<Token> tokens = addExplicitConcats(rawTokens);

        std::stack<Node*> values;
        std::stack<Token> operators;

        for (size_t i = 0; i < tokens.size(); i++) {
            Token t = tokens[i];

            if (t.type == TokenType::LITERAL || t.type == TokenType::NUMBER) {
                values.push(new Node(t));
            }
            else if (t.type == TokenType::LPAREN) {
                operators.push(t);
            }
            else if (t.type == TokenType::RPAREN) {
                while (!operators.empty() && operators.top().type != TokenType::LPAREN) {
                    applyOperator(values, operators.top());
                    operators.pop();
                }
                if (!operators.empty()) {
                    operators.pop();
                }
            }
            else if (t.type == TokenType::LBRACE) {
                if (values.empty()) continue;
                Node* target = values.top();
                values.pop();
                Node* expanded = handleBraceExpansion(target, tokens, i);
                values.push(expanded);
            }
            else {
                while (!operators.empty() && getPrecedence(operators.top().type) >= getPrecedence(t.type)) {
                    applyOperator(values, operators.top());
                    operators.pop();
                }
                operators.push(t);
            }
        }

        while (!operators.empty()) {
            applyOperator(values, operators.top());
            operators.pop();
        }

        if (values.empty()) {
            return nullptr;
        }

        return values.top();
    }
};


#endif //REGEX_VALIDATOR_AST_H