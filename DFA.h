//
// Created by arman on 6/7/2026.
//

#ifndef REGEX_VALIDATOR_DFA_H
#define REGEX_VALIDATOR_DFA_H

#include <map>
#include <set>
#include <stack>
#include <vector>
#include "nfa.h"

struct DFAState {
    int id;
    bool isAcceptState;
    std::map<char, DFAState*> transitions;
    std::set<int> nfaStateIds;

    DFAState(int id, const std::set<int>& nfaStateIds) {
        this->id = id;
        this->isAcceptState = false;
        this->nfaStateIds = nfaStateIds;
    }
};

class DFA {
public:
    DFAState* startState;

    explicit DFA(DFAState* start) {
        this->startState = start;
    }
};

class DFABuilder {
private:
    int stateCounter = 0;

    DFAState* createState(const std::set<int>& nfaStateIds) {
        DFAState* state = new DFAState(stateCounter, nfaStateIds);
        stateCounter++;
        return state;
    }

    std::set<NFAState*> getEpsilonClosure(std::set<NFAState*> states) {
        std::stack<NFAState*> stack;
        std::set<NFAState*> closure = states;

        for (auto s : states) {
            stack.push(s);
        }
        while (!stack.empty()) {
            NFAState *s = stack.top();
            stack.pop();
            for (auto t : s->epsilonTransitions) {
                if (closure.find(t) == closure.end()) {
                    closure.insert(t);
                    stack.push(t);
                }
            }
        }

        return closure;
    }

    std::set<NFAState*> move(std::set<NFAState*> states, char symbol) {
        std::set<NFAState*> result;
        for (auto s : states) {
            if (s->transitions.find(symbol) != s->transitions.end()) {
                for (auto dest : s->transitions[symbol]) {
                    result.insert(dest);
                }
            }
        }
        return result;
    }

public:
    DFA* build(NFA* nfa) {
        std::set<NFAState*> initialStates;
        initialStates.insert(nfa->startState);
        std::set<NFAState*> startClosure = getEpsilonClosure(initialStates);

        std::set<int> startIds;
        for (auto s : startClosure) {
            startIds.insert(s->id);
        }

        DFAState* startState = createState(startIds);
        if (startIds.find(nfa->acceptState->id) != startIds.end()) {
            startState->isAcceptState = true;
        }

        std::map<std::set<int>, DFAState*> dfaMap;
        dfaMap[startIds] = startState;

        std::vector<std::set<NFAState*>> unmarked;
        unmarked.push_back(startClosure);

        int currentIndex = 0;

        while (currentIndex < unmarked.size()) {
            std::set<NFAState*> currentNFAStates = unmarked[currentIndex];
            currentIndex++;

            std::set<int> currentIds;
            for (auto s : currentNFAStates) {
                currentIds.insert(s->id);
            }
            DFAState* currentDFAState = dfaMap[currentIds];

            std::set<char> symbols;
            for (auto s : currentNFAStates) {
                for (auto const& pair : s->transitions) {
                    symbols.insert(pair.first);
                }
            }

            for (char sym : symbols) {
                std::set<NFAState*> moveRes = move(currentNFAStates, sym);
                std::set<NFAState*> targetClosure = getEpsilonClosure(moveRes);

                if (!targetClosure.empty()) {
                    std::set<int> targetIds;
                    for (auto s : targetClosure) {
                        targetIds.insert(s->id);
                    }

                    if (dfaMap.find(targetIds) == dfaMap.end()) {
                        DFAState* newState = createState(targetIds);
                        if (targetIds.find(nfa->acceptState->id) != targetIds.end()) {
                            newState->isAcceptState = true;
                        }
                        dfaMap[targetIds] = newState;
                        unmarked.push_back(targetClosure);
                    }

                    currentDFAState->transitions[sym] = dfaMap[targetIds];
                }
            }
        }

        return new DFA(startState);
    }
};
#endif //REGEX_VALIDATOR_DFA_H