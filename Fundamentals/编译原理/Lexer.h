#pragma once
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "def.h"
using namespace std;

static struct Word {
    string str;          //语义信息
    LexType tok;         //词法信息
} ReservedWords[21] = {  //保留字
    {"program", PROGRAM}, {"type", TYPE},
    {"var", VAR},         {"procedure", PROCEDURE},
    {"begin", BEGIN},     {"end", END},
    {"array", ARRAY},     {"of", OF},
    {"record", RECORD},   {"if", IF},
    {"then", THEN},       {"else", ELSE},
    {"fi", FI},           {"while", WHILE},
    {"do", DO},           {"endwh", ENDWH},
    {"read", READ},       {"write", WRITE},
    {"return", RETURN},   {"integer", INTEGER},
    {"char", CHARR}};

class Token {
public:
    Token();
    Token(int _lineShow, LexType _lex, string _sem);
    void setLineShow(int lineShow);
    int& getLineShow();
    void setLexType(LexType lexType);
    LexType& getLexType();
    void setSem(string sem);
    string& getSem();

    int line;  //在源程序中的行数
    Word word;
};

class TokenList {
public:
    TokenList();
    void append(Token& token);     //添加元素
    void remove(int index);        //删除元素
    Token& operator[](int index);  //获取元素
    int size();                    //获取元素个数
private:
    vector<Token> tokenList;
};

class Lexer {
public:
    bool IsSeparater(char ch);
    bool IsOperator(char ch);
    bool IsKeyWord(string ch);
    bool IsFilter(char ch);
    bool IsLetter(char ch);
    bool IsDigit(char ch);
    Token tokenList[10000];
    TokenList tlist;
    void WordAnalyse(FILE* fpin);
    LexType GetTokenType(string charList);
    Word reservedLookup(string s);
    string convert(char chx);
    string toString(int lexType);

    Lexer(){};
    Token* getTokenList(FILE* fpin);
    TokenList getTList(FILE* fpin);
    void PrintTokenList();
};
