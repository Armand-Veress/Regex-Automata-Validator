//
// Created by arman on 6/6/2026.
//

#ifndef REGEX_VALIDATOR_NFA_H
#define REGEX_VALIDATOR_NFA_H
#include <map>

#include "Lexer.h"

struct NFAState {
    int id;
    bool isAcceptState;
    std::map<char, std::vector<NFAState*>> transitions;
    std::vector<NFAState*> epsilonTransitions;

    NFAState(int id) {
        this->id = id;
        this->isAcceptState = false;
    }
};

class NFA {
public:
    NFAState* startState;
    NFAState* acceptState;

    NFA(NFAState* start, NFAState* accept) {
        this->startState = start;
        this->acceptState = accept;
    }
};

class NFABuilder {
private:
    int stateCounter = 0;

    NFAState* createState() {
        NFAState* state = new NFAState(stateCounter);
        stateCounter++;
        return state;
    }

public:
    NFA* build(Node* node) {
        if (node == nullptr) {
            return nullptr;
        }

        if (node->token.type == TokenType::LITERAL || node->token.type == TokenType::NUMBER) {
            NFAState* startState = createState();
            NFAState* acceptState = createState();
            acceptState->isAcceptState = true;
            char c = node->token.value[0];
            startState->transitions[c].push_back(acceptState);
            return new NFA(startState, acceptState);
        }
        if (node->token.type == TokenType::CONCAT) {
            NFA* leftNFA = build(node->left);
            NFA* rightNFA = build(node->right);
            leftNFA->acceptState->isAcceptState = false;
            leftNFA->acceptState->epsilonTransitions.push_back(rightNFA->startState);
            return new NFA(leftNFA->startState, rightNFA->acceptState);
        }
        if (node->token.type == TokenType::UNION) {
            NFA* leftNFA = build(node->left);
            NFA* rightNFA = build(node->right);
            NFAState* startState = createState();
            NFAState* acceptState = createState();
            acceptState->isAcceptState = true;
            startState->epsilonTransitions.push_back(leftNFA->startState);
            startState->epsilonTransitions.push_back(rightNFA->startState);
            leftNFA->acceptState->isAcceptState = false;
            rightNFA->acceptState->isAcceptState = false;
            leftNFA->acceptState->epsilonTransitions.push_back(acceptState);
            rightNFA->acceptState->epsilonTransitions.push_back(acceptState);
            return new NFA(startState, acceptState);
        }
        if (node->token.type == TokenType::OPTIONAL) {
            NFA* innerNFA = build(node->left);
            NFAState* startState = createState();
            NFAState* acceptState = createState();
            acceptState->isAcceptState = true;
            startState->epsilonTransitions.push_back(innerNFA->startState);
            startState->epsilonTransitions.push_back(acceptState);
            innerNFA->acceptState->isAcceptState = false;
            innerNFA->acceptState->epsilonTransitions.push_back(acceptState);
            return new NFA(startState, acceptState);
        }
        if (node->token.type == TokenType::STAR) {
            NFA* innerNFA = build(node->left);
            NFAState* startState = createState();
            NFAState* acceptState = createState();
            acceptState->isAcceptState = true;
            startState->epsilonTransitions.push_back(innerNFA->startState);
            startState->epsilonTransitions.push_back(acceptState);
            innerNFA->acceptState->isAcceptState = false;
            innerNFA->acceptState->epsilonTransitions.push_back(innerNFA->startState);
            innerNFA->acceptState->epsilonTransitions.push_back(acceptState);
            return new NFA(startState, acceptState);
        }
        if (node->token.type == TokenType::PLUS) {
            NFA* innerNFA = build(node->left);
            NFAState* startState = createState();
            NFAState* acceptState = createState();
            acceptState->isAcceptState = true;
            startState->epsilonTransitions.push_back(innerNFA->startState);
            innerNFA->acceptState->isAcceptState = false;
            innerNFA->acceptState->epsilonTransitions.push_back(innerNFA->startState);
            innerNFA->acceptState->epsilonTransitions.push_back(acceptState);
            return new NFA(startState, acceptState);
        }

        return nullptr;
    }
};


#endif //REGEX_VALIDATOR_NFA_H