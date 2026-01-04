#pragma once

#include <map>
#include <stack>

#include "parser.h"

class LL1 : public Parser {
public:
    LL1();
    virtual SyntaxTree* parse(TokenList tokenList) override;
    static int nowLine;
    static int cur;
    static TreeNode* newSpecNode(NodeKind kind);
    static TreeNode* newDecNode(DecKind kind);
    static TreeNode* newStmtNode(StmtKind kind);
    static TreeNode* newExpNode(ExpKind kind);

private:
    int Priosity(LexType op);
    void process(int id, TokenList& tokenList);
    void syntaxError(string error);

    map<pair<LexType, LexType>, int> table;
    stack<LexType> symbol_stack;
    stack<TreeNode**> syntaxtree_stack;
    stack<TreeNode*> op_stack;
    stack<TreeNode*> num_stack;

    TreeNode* currentP;
    DecKind* temp;
    TreeNode* saveP;
    bool getExpResult = true;
    int getExpResult2 = false;
    int expflag = 0;
};