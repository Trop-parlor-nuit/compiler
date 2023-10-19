#include <iostream>
#include <string>
#include <stack>
#include <map>
#include <set>
#include <vector>

using namespace std;

struct TreeNode {
    char value;
    TreeNode* left;
    TreeNode* right;
    int position;
    set<int> firstpos, lastpos;
    bool nullable;
};

class RegexParser {
public:
    RegexParser(const string& regex) : regex(regex), pos(0) {}

    TreeNode* parse() {
        auto node = expression();
        if (pos != regex.size()) {
            throw runtime_error("Unexpected character");
        }
        return node;
    }

private:
    string regex;
    int pos;

    TreeNode* expression() {
        auto node = term();

        while (pos < regex.size() && regex[pos] == '|') {
            ++pos;
            auto rightNode = term();
            auto unionNode = new TreeNode{ '|', node, rightNode };
            node = unionNode;
        }

        return node;
    }

    TreeNode* term() {
        auto node = factor();

        while (pos < regex.size() && (regex[pos] != ')' && regex[pos] != '|')) {
            auto rightNode = factor();
            auto concatNode = new TreeNode{ '.', node, rightNode };  // '.'表示连接操作
            node = concatNode;
        }

        return node;
    }

    TreeNode* factor() {
        auto node = primary();

        while (pos < regex.size() && regex[pos] == '*') {
            ++pos;
            auto starNode = new TreeNode{ '*', node };
            node = starNode;
        }

        return node;
    }

    TreeNode* primary() {
        if (pos >= regex.size()) {
            throw runtime_error("Unexpected end of regex");
        }

        if (regex[pos] == '(') {
            ++pos;
            auto node = expression();
            if (regex[pos] != ')') {
                throw runtime_error("Expected ')'");
            }
            ++pos;
            return node;
        }
        else {
            auto leafNode = new TreeNode{ regex[pos] };
            ++pos;
            return leafNode;
        }
    }
};

class DFABuilder {
    int nextPosition = 1;
    map<int, char> positionToChar;
    map<int, set<int>> followpos;

public:
    // 根据语法树计算nullable、firstpos、lastpos和followpos
    void calculate(TreeNode* node) {
        if (!node) return;

        if (node->value == '|') {
            calculate(node->left);
            calculate(node->right);
            node->nullable = node->left->nullable || node->right->nullable;
            node->firstpos.insert(node->left->firstpos.begin(), node->left->firstpos.end());
            node->firstpos.insert(node->right->firstpos.begin(), node->right->firstpos.end());
            node->lastpos.insert(node->left->lastpos.begin(), node->left->lastpos.end());
            node->lastpos.insert(node->right->lastpos.begin(), node->right->lastpos.end());
        }
        else if (node->value == '.') {
            calculate(node->left);
            calculate(node->right);
            node->nullable = node->left->nullable && node->right->nullable;
            if (node->left->nullable) {
                node->firstpos.insert(node->left->firstpos.begin(), node->left->firstpos.end());
                node->firstpos.insert(node->right->firstpos.begin(), node->right->firstpos.end());
            }
            else {
                node->firstpos = node->left->firstpos;
            }

            if (node->right->nullable) {
                node->lastpos.insert(node->left->lastpos.begin(), node->left->lastpos.end());
                node->lastpos.insert(node->right->lastpos.begin(), node->right->lastpos.end());
            }
            else {
                node->lastpos = node->right->lastpos;
            }

            for (int position : node->left->lastpos) {
                followpos[position].insert(node->right->firstpos.begin(), node->right->firstpos.end());
            }
        }
        else if (node->value == '*') {
            calculate(node->left);
            node->nullable = true;
            node->firstpos = node->left->firstpos;
            node->lastpos = node->left->lastpos;
            for (int position : node->left->lastpos) {
                followpos[position].insert(node->left->firstpos.begin(), node->left->firstpos.end());
            }
        }
        else { // 叶子节点
            node->nullable = false;
            node->position = nextPosition++;
            positionToChar[node->position] = node->value;
            node->firstpos.insert(node->position);
            node->lastpos.insert(node->position);
        }
    }

};

int main() {
    string regex;
    cout << "Enter regex: ";
    cin >> regex;

    try {
        RegexParser parser(regex);
        TreeNode* tree = parser.parse();

        DFABuilder dfaBuilder;
        dfaBuilder.calculate(tree);

    }
    catch (const runtime_error& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}

