#include "RegexValidator.h"
#include <iostream>
#include <string>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: RegexValidator <regex>\n";
        return -1;
    }

    std::string pattern = argv[1];
    auto start = std::chrono::high_resolution_clock::now();
    RegexValidator validator(pattern);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "DFA compiled successfully in: " << duration << " microseconds\n";
    std::cout << "[ Validating input for the regular expression: " << pattern << " ]\n";
    std::cout << ">> ";

    std::string text;
    while (std::getline(std::cin, text)) {
        bool isMatch = validator.match(text);

        if (isMatch) {
            std::cout << "VALID\n";
        } else {
            std::cout << "INVALID\n";
        }

        std::cout << ">> ";
    }

    return 0;
}