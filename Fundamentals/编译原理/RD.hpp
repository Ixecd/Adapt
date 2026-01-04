#pragma once

#include "parser.h"
#include "syntaxtree.h"

//递归下降法的语法分析程序
class RD : public Parser {
public:
    RD();
    virtual SyntaxTree* parse(TokenList tokenList) override;
    vector<string> tr;  //语法树
private:
    bool match(LexType lexType);

    TreeNode* program();
    TreeNode* programHead();
    TreeNode* declarePart();
    TreeNode* typeDec();
    TreeNode* typeDeclaration();
    TreeNode* typeDecList();
    TreeNode* typeDecMore();
    void typeID(TreeNode* t);
    void typeDef(TreeNode* t);
    void baseType(TreeNode* t);
    void structureType(TreeNode* t);
    void arrayType(TreeNode* t);
    void recType(TreeNode* t);
    TreeNode* fieldDecList();
    TreeNode* fieldDecMore();
    void idList(TreeNode* t);
    void idMore(TreeNode* t);
    TreeNode* varDec();
    TreeNode* varDeclaration();
    TreeNode* varDecList();
    TreeNode* varDecMore();
    void varIDList(TreeNode* t);
    void varIDMore(TreeNode* t);
    TreeNode* procDec();
    TreeNode* procDeclaration();
    TreeNode* procDecMore();
    void paramList(TreeNode* t);
    TreeNode* paramDecList();
    TreeNode* paramMore();
    TreeNode* param();
    void formList(TreeNode* t);
    void fidMore(TreeNode* t);
    TreeNode* procDecPart();
    TreeNode* procBody();
    TreeNode* programBody();
    TreeNode* stmList();
    TreeNode* stmMore();
    TreeNode* stm();
    TreeNode* assCall();
    TreeNode* assignmentRest();
    TreeNode* conditionalStm();
    TreeNode* loopStm();
    TreeNode* inputStm();
    TreeNode* outputStm();
    TreeNode* returnStm();
    TreeNode* callStmRest();
    TreeNode* actParamList();
    TreeNode* actParamMore();
    TreeNode* relExp();
    TreeNode* exp();
    TreeNode* term();
    TreeNode* factor();
    TreeNode* variable();
    void variMore(TreeNode* t);
    TreeNode* fieldVar();
    void fieldVarMore(TreeNode* t);

    void cmpOp(TreeNode* t);
    void addOp(TreeNode* t);
    void multOp(TreeNode* t);

    void syntaxError(std::string error);
    void syntaxErrorContinue(std::string error);

private:
    TokenList tokenList;
    int cur;

    std::string temp_name;
};
