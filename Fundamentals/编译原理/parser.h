#pragma once

#include "Lexer.h"
#include "syntaxtree.h"

class Parser {
public:
    Parser();
    virtual ~Parser() = default;
    virtual SyntaxTree* parse(TokenList tokenList) = 0;
};
