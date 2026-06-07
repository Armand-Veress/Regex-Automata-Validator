//
// Created by arman on 6/7/2026.
//

#ifndef REGEX_VALIDATOR_REGEXVALIDATOR_H
#define REGEX_VALIDATOR_REGEXVALIDATOR_H

#include <string>
#include <set>
#include <stack>
#include "ast.h"
#include "nfa.h"
#include "dfa.h"

class RegexValidator {
private:
    DFA* dfa;

    void cleanupDFA(DFAState* start) {
        if (start == nullptr) {
            return;
        }

        std::set<DFAState*> visited;
        std::stack<DFAState*> stack;
        stack.push(start);
        visited.insert(start);

        while (!stack.empty()) {
            DFAState* curr = stack.top();
            stack.pop();

            for (auto const& pair : curr->transitions) {
                DFAState* dest = pair.second;
                if (visited.find(dest) == visited.end()) {
                    visited.insert(dest);
                    stack.push(dest);
                }
            }
        }

        for (auto state : visited) {
            delete state;
        }
    }

    void cleanupNFA(NFAState* start) {
        if (start == nullptr) {
            return;
        }

        std::set<NFAState*> visited;
        std::stack<NFAState*> stack;
        stack.push(start);
        visited.insert(start);

        while (!stack.empty()) {
            NFAState* curr = stack.top();
            stack.pop();

            for (auto t : curr->epsilonTransitions) {
                if (visited.find(t) == visited.end()) {
                    visited.insert(t);
                    stack.push(t);
                }
            }

            for (auto const& pair : curr->transitions) {
                for (auto dest : pair.second) {
                    if (visited.find(dest) == visited.end()) {
                        visited.insert(dest);
                        stack.push(dest);
                    }
                }
            }
        }

        for (auto state : visited) {
            delete state;
        }
    }

public:
    RegexValidator(const std::string& pattern) {
        AST ast;
        Node* root = ast.parse(pattern);

        NFABuilder nfaBuilder;
        NFA* nfa = nfaBuilder.build(root);

        DFABuilder dfaBuilder;
        this->dfa = dfaBuilder.build(nfa);

        cleanupNFA(nfa->startState);
        delete nfa;
        delete root;
    }

    ~RegexValidator() {
        if (dfa != nullptr) {
            cleanupDFA(dfa->startState);
            delete dfa;
        }
    }

    bool match(const std::string& text) {
        if (dfa == nullptr || dfa->startState == nullptr) {
            return false;
        }

        DFAState* currentState = dfa->startState;

        for (char c : text) {
            if (currentState->transitions.find(c) != currentState->transitions.end()) {
                currentState = currentState->transitions[c];
            } else {
                return false;
            }
        }

        return currentState->isAcceptState;
    }
};

#endif //REGEX_VALIDATOR_REGEXVALIDATOR_H