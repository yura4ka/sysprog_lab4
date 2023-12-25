#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <stack>
#include "Parser.h"

using std::vector, std::string, std::unordered_map, std::unordered_set;

void throwError(const int &line, const char *error) {
    std::ostringstream err;
    err << "error on line " << line << " : " << error;
    throw std::runtime_error(err.str());
}

bool isNonTerminal(const string &s) {
    return s[0] >= 'A' && s[0] <= 'Z';
}

Parser::Parser(const string &input) {
    std::istringstream ss(input);
    string temp, word;
    int lineNumber = 0;
    while (getline(ss, temp)) {
        lineNumber++;
        std::istringstream line(temp);

        line >> word;
        if (word.empty())
            throwError(lineNumber, "wrong left side");
        if (word == "->")
            throwError(lineNumber, "'->' is a reserved word");
        if (!isNonTerminal(word))
            throwError(lineNumber, "left side can only be a non-terminal");
        _nonTerminals.try_emplace(word, _nonTerminals.size());
        string key = word;

        line >> word;
        if (word != "->")
            throwError(lineNumber, "'->' hasn't been found");

        vector<string> symbols;
        while (line >> word) {
            if (word == "->")
                throwError(lineNumber, "'->' is a reserved word");
            symbols.push_back(word);
            if (isNonTerminal(word)) _nonTerminals.try_emplace(word, _nonTerminals.size());
            else _terminals.try_emplace(word, _terminals.size());
        }

        if (symbols.empty())
            throwError(lineNumber, "right side is empty");

        _rules.emplace_back(key, symbols);
    }

    _follow[START].insert(EPS);
    findNullables();
    computeSet(_first);
    computeSet(_follow);
    buildTable();
}

void Parser::computeSet(unordered_map<string, unordered_set<string>> &set) {
    size_t prevCount = 1;
    size_t newCount = 0;

    while (newCount - prevCount != 0) {
        prevCount = newCount;
        newCount = 0;
        for (const auto &[t, _]: _nonTerminals) {
            if (set == _first) computeFirstSet(t);
            else computeFollowSet(t);
            newCount += set[t].size();
        }
    }
}

void Parser::computeFirstSet(const string &s) {
    for (const auto &rule: _rules) {
        if (rule.first != s) continue;

        bool addEps = true;
        for (const auto &symbol: rule.second) {
            if (_terminals.find(symbol) != _terminals.end()) {
                _first[s].insert(symbol);
                addEps = false;
                break;
            }

            const auto &firstOfSymbol = _first[symbol];
            if (firstOfSymbol.empty()) {
                addEps = false;
                break;
            }

            auto eps = firstOfSymbol.find(EPS);
            if (eps != firstOfSymbol.end()) {
                _first[s].insert(firstOfSymbol.begin(), eps);
                _first[s].insert(++eps, firstOfSymbol.end());
            } else {
                _first[s].insert(firstOfSymbol.begin(), firstOfSymbol.end());
                addEps = false;
                break;
            }
        }
        if (addEps) _first[s].insert(EPS);
    }
}

void Parser::computeFollowSet(const string &s) {
    for (const auto &r: _rules) {
        const auto &pos = std::find(r.second.begin(), r.second.end(), s);
        if (pos == r.second.end()) continue;

        bool addFollow = true;
        for (auto it = pos + 1; it < r.second.end(); ++it) {
            if (_terminals.find(*it) != _terminals.end()) {
                _follow[s].insert(*it);
                addFollow = false;
                break;
            }
            const auto &first = _first[*it];
            auto eps = first.find(EPS);
            if (eps == first.end()) {
                _follow[s].insert(first.begin(), first.end());
                addFollow = false;
                break;
            } else {
                _follow[s].insert(first.begin(), eps);
                _follow[s].insert(++eps, first.end());
            }
        }

        if (addFollow && r.first != s) {
            _follow[s].insert(_follow[r.first].begin(), _follow[r.first].end());
        }
    }
}

void Parser::printSet(const unordered_map<string, unordered_set<string>> &set) {
    for (const auto &[t, _]: _nonTerminals) {
        std::cout << t << ": ";
        const auto &symbols = set.at(t);
        for (auto it = symbols.begin(); it != symbols.end();) {
            std::cout << "'" << *it << "'";
            if (++it != symbols.end()) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

void Parser::printFirst() {
    printSet(_first);
}

void Parser::printFollow() {
    printSet(_follow);
}

void Parser::buildTable() {
    _table = vector<vector<int>>(_nonTerminals.size(), vector<int>(_terminals.size()));

    for (int i = 0; i < _rules.size(); i++) {
        const auto &[left, right] = _rules[i];

        bool addFollow = false;
        for (const auto &s: right) {
            addFollow = false;
            if (s == EPS) {
                addFollow = true;
                break;
            }
            if (_terminals.find(s) != _terminals.end()) {
                _table[_nonTerminals[left]][_terminals[s]] = i + 1;
                break;
            }
            const auto &first = _first[s];
            for (const auto &f: first) {
                if (f != EPS)
                    _table[_nonTerminals[left]][_terminals[f]] = i + 1;
                else
                    addFollow = true;
            }
            if (!addFollow)
                break;
        }

        if (addFollow) {
            for (const auto &f: _follow[left])
                _table[_nonTerminals[left]][_terminals[f]] = i + 1;
        }
    }
}

void Parser::printTable() {
    std::cout << std::setw(2) << "" << " | ";
    for (const auto &[t, _]: _terminals)
        std::cout << std::setw(3) << t << " | ";
    std::cout << "\n";

    for (const auto &[t, i]: _nonTerminals) {
        std::cout << std::setw(2) << t << " | ";
        for (const auto &[_, j]: _terminals) {
            std::cout << std::setw(3);
            if (_table[i][j] == 0) std::cout << "";
            else std::cout << _table[i][j];
            std::cout << " | ";
        }
        std::cout << "\n";
    }
}

void Parser::findNullables() {
    size_t prevSize = 1;
    while (_nullables.size() - prevSize != 0 && _nullables.size() != _nonTerminals.size()) {
        prevSize = _nullables.size();
        for (const auto &[left, right]: _rules) {
            bool isNullable = true;
            for (const auto &s: right) {
                if (s == EPS) continue;
                if (!isNonTerminal(s) || _nullables.find(s) == _nullables.end()) {
                    isNullable = false;
                    break;
                }
            }
            if (isNullable) _nullables.insert(left);
        }
    }
}

void Parser::printNullables() {
    for (auto it = _nullables.begin(); it != _nullables.end();) {
        std::cout << "'" << *it << "'";
        if (++it != _nullables.end()) std::cout << ", ";
    }
    std::cout << "\n";
}

void Parser::throwParseError(const int &row, const string &received) {
    std::ostringstream err;
    err << "syntax error: expected ";
    for (const auto &[t, j]: _terminals) {
        if (_table[row][j] != 0)
            err << "'" << t << "' ";
    }
    err << "received " << received;
    throw std::runtime_error(err.str());
}

vector<int> Parser::parse(const string &lexeme) {
    std::istringstream input(lexeme);
    string current;
    std::stack<string> stack;
    stack.emplace(START);
    vector<int> rules;

    input >> current;
    if (_terminals.find(current) == _terminals.end())
        throw std::runtime_error("wrong symbol");
    while (true) {
        if (stack.empty()) {
            if (current == EPS) break;
            throwParseError(_nonTerminals[stack.top()], EPS);
        }
        string top = stack.top();
        if (isNonTerminal(top)) {
            stack.pop();
            int row = _nonTerminals[top];
            int rule = _table[row][_terminals[current]];
            if (rule == 0)
                throwParseError(row, current);
            rules.push_back(rule);
            for (auto it = _rules[rule - 1].second.rbegin(); it != _rules[rule - 1].second.rend(); ++it) {
                if (*it != EPS)
                    stack.push(*it);
            }
        } else {
            stack.pop();
            if (top != current)
                throw std::runtime_error("syntax error");
            if (!(input >> current))
                current = EPS;
            if (_terminals.find(current) == _terminals.end())
                throw std::runtime_error("wrong symbol");
        }
    }

    return rules;
}

void Parser::dfsBuild(TreeNode &node, const vector<int> &seq, int &pos) {
    for (const auto &s: _rules[seq[pos] - 1].second) {
        node.children.emplace_back(s);
        if (pos + 1 != seq.size() && _rules[seq[pos + 1] - 1].first == s) {
            dfsBuild(node.children.back(), seq, ++pos);
        }
    }
}

TreeNode Parser::buildAST(const vector<int> &seq) {
    TreeNode root(START);
    int pos = 0;
    dfsBuild(root, seq, pos);
    return root;
}
