cmake_minimum_required(VERSION 4.1)
project(RegexValidator)

set(CMAKE_CXX_STANDARD 20)

add_executable(RegexValidator main.cpp
        Lexer.h
        ast.h
        nfa.h
        DFA.h
        RegexValidator.h)
