# Regex Automata Validator

---
1. [Project description](#1-project-description)
   - [HowToRun](#how-to-run)
2. [Formal context and Problem space](#2-formal-context-and-problem-space)
3. [Example analysis](#3-example-analysis)
---

## 1. Project description
The project is a **C++** library implementation that compiles an initial regex pattern into a high-performance state-machine for text validation.
The pipeline parses it into an **Abstract Syntax tree (AST)**. The AST is then constructed into a **Non-deterministic Finite Automata (NFA)**, which is then converted to a **Deterministic Finite Automata (DFA)**.The resulting DFA provides optimal, predictable $O(N)$ validation time.

## How to run
* Clone repository, create a `\build` directory and navigate to it:
  ```bash
  mkdir build && cd build
  ```
* Configure the CMake project:
  ```bash
  cmake ..
  ```
* Build the executable:
  ```bash
  cmake --build .
  ```
* Navigate to the executable in the `\Debug` directory and start the program by passing the input regex as an argument. e.g.:
  ```bash
  # Linux/macOS
  ./RegexValidator "(a|b)+c*c(ba){2}"
  # Windows 
  .\RegEx_validator.exe "(a|b)+c*c(ba){2}"
  ```

  ---

## 2. Formal context and Problem space
  
    The application operates within the domain of **Formal Language Theory**, evaluating whether a given input string belongs to a **Regular Language** defined by a Regular Expression. 
The program parses the regular expression into an Abstract Syntax Tree (AST) and translates it into an equivalent Non-deterministic Finite Automaton ($\epsilon$-NFA) via **Thompson's Construction**. \\
    The structure is then further optimized by converting the $\epsilon$-NFA into a Deterministic Finite Automaton (DFA) using the **Subset Construction algorithm**. 
While NFA-to-DFA conversion has a theoretical worst-case space complexity of $O(2^m)$, where $m$ is the number of NFA states, the resulting state-program guarantees a linear validation time complexity of **$O(N)$**, where $N$ is the length of the input text.

  ---

## 3. Example analysis 
**RE:** `/[-a-zA-Z0-9_]+(/[-a-zA-Z0-9_]+)*/?`; <br>
**Let $L(RE)$ be the Regular Language of standard Unix directory paths.**

---

### 3.1. Lexical Analysis & Token Expansion

Before building the execution graph, the `Lexer` expands the shortcut notation `[...]` into an atomic disjunction (`UNION`: `(...|...)`). Let $C$ be the complete set of valid characters defined by `[-a-zA-Z0-9_]`. 

The tokenization phase flattens the input stream into a deterministic sequence of structural `Token` objects stored in a flat `std::vector<Token>`:

Memory layout (`std::vector<Token>`):
```bash
[ LITERAL, '/' ]
[ LPAREN, '(' ][ LITERAL, '-' ][ UNION, '|' ][ LITERAL, 'a' ][ UNION, '|' ]  ... [ LITERAL, '_' ][ RPAREN, ')' ][ PLUS, '+' ] 
[ LPAREN, '(' ][ LITERAL, '/' ] 
[ LPAREN, '(' ][ LITERAL, '-' ][ UNION, '|' ]...[ LITERAL, '_' ][ RPAREN, ')' ]
[ PLUS, '+' ][ RPAREN, ')' ][ STAR, '*' ]
[ LITERAL, '/' ][ OPTIONAL, '?' ]
```

---

### 3.2. AST Parsing

The flat token vector is parsed into an ast by applying operator priorities (*, +, ? > CONCAT > UNION) and injecting explicit concatenation points ($\cdot$). 

The resulting tree structure in memory manages ownership via raw parent-to-child pointers (Node* left, Node* right).
```bash
                 Node: CONCAT (.)
                /                \
        Node: CONCAT (.)        Node: OPTIONAL (?)
       /                \                |
Node: LITERAL (/)    Node: STAR (*)   Node: LITERAL (/)
                        |
                     Node: CONCAT (.)
                    /                \
            Node: LITERAL (/)     Node: PLUS (+)
                                     |
                            [UNION Sub-Tree: LITERAL(C)]
```
---

### 3.3. Thompson $\epsilon$-NFA Generation

The `NFABuilder` recursively evaluates the AST. Each unary operator encapsulates its underlying sub-graph by drawing internal $\epsilon$-transitions to control the state stream:

* **Plus (+)**: Connects the accept state of the Class $C$ sub-graph back to its own start state via an $\epsilon$-transition, forcing at least one mandatory match.
* **Star (*)**: Applies for the entire structural directory sub-graph (/ • C+), adding an external $\epsilon$-transition path from its initial state directly to its accept state, enabling zero or more repetitions.

---

### 3.4. Subset Construction (DFA Compilation)

The DFABuilder converts the branching graph into a strict state machine by resolving all $\epsilon$-transitions. It groups these paths into single states, where each state maps an input character directly to its next destination (`std::map<char, DFAState*>`).

 <img width="737" height="142" alt="image" src="https://github.com/user-attachments/assets/d982b91e-f172-4227-a835-8ce5974daa9a" />

Where **q2** and **q3** are accept states: Q = {q0, q1, q2, q3} , F = {q2, q3} <br>
*note: We assumed that missing transitions lead to a trap state.

---
