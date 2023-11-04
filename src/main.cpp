#include <iostream>
#include <string>
#include "Parser.h"

void printAST(const std::string &prefix, const TreeNode &node, const bool &isLast) {
    std::cout << prefix;
    std::cout << (!isLast ? "|-- " : "\\-- ");
    std::cout << node.value << "\n";

    for (int i = 0; i < node.children.size(); i++)
        printAST(prefix + (!isLast ? "|   " : "    "), node.children[i], i == node.children.size() - 1);
}

int main() {
    std::string line, input;
    std::cout << "Input grammar (e for epsilon) (press q to stop):\n";

    while (std::getline(std::cin, line)) {
        if (line == "q") break;
        if (!line.empty()) input += line + "\n";
    }

    Parser *p;
    try {
        p = new Parser(input);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << "\n" << "Press any key to exit...";
        std::getchar();
        return 0;
    }

    std::cout << "\nNullables:\n";
    p->printNullables();
    std::cout << "\nFirst set:\n";
    p->printFirst();
    std::cout << "\nFollow set:\n";
    p->printFollow();
    std::cout << "\nTable:\n";
    p->printTable();

    std::cout << "\nEnter tokens: ";
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        try {
            const auto &result = p->parse(line);
            std::cout << "rules: ";
            for (const auto &r: result)
                std::cout << r << " ";
            std::cout << "\n";

            const auto &root = p->buildAST(result);
            std::cout << "\nAST:\n";
            printAST("", root, true);
        } catch (const std::runtime_error &e) {
            std::cout << e.what() << "\n";
        }
        std::cout << "\nEnter tokens: ";
    }

    return 0;
}
