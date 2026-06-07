#include "RegexValidator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: RegexValidator <regex>\n";
        return -1;
    }

    std::string pattern = argv[1];
    RegexValidator validator(pattern);

    std::cout << "-- Validating input for the regular expression: " << pattern << " --\n";
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