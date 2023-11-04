#ifndef LAB4_PARSER_H
#define LAB4_PARSER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_set>
#include "TreeNode.h"

#define EPS "e"
#define START "S"

using std::vector, std::string, std::unordered_map, std::unordered_set;

class Parser {
private:
    vector<std::pair<string, vector<string>>> _rules;
    unordered_set<string> _nullables;
    unordered_map<string, int> _nonTerminals;
    unordered_map<string, int> _terminals;
    unordered_map<string, unordered_set<string>> _first;
    unordered_map<string, unordered_set<string>> _follow;
    vector<vector<int>> _table;

    void computeSet(unordered_map<string, unordered_set<string>> &set);

    void computeFirstSet(const string &s);

    void computeFollowSet(const string &s);

    void buildTable();

    void printSet(const unordered_map<string, unordered_set<string>> &set);

    void findNullables();

    void throwParseError(const int &row, const string &received);

    void dfsBuild(TreeNode &node, const vector<int> &seq, int &pos);

public:
    explicit Parser(const string &input);

    void printFirst();

    void printFollow();

    void printTable();

    void printNullables();

    vector<int> parse(const string &str);

    TreeNode buildAST(const vector<int> &seq);
};


#endif
