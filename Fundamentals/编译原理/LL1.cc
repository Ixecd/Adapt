#include "LL1.h"
int LL1::nowLine = 0;
int LL1::cur = 0;
LL1::LL1() {
    this->table = {
        {{Program, PROGRAM}, 1},
        {{ProgramHead, PROGRAM}, 2},
        {{ProgramName, ID}, 3},
        {{DeclarePart, TYPE}, 4},
        {{DeclarePart, VAR}, 4},
        {{DeclarePart, PROCEDURE}, 4},
        {{DeclarePart, BEGIN}, 4},
        {{TypeDec, TYPE}, 6},
        {{TypeDec, VAR}, 5},
        {{TypeDec, PROCEDURE}, 5},
        {{TypeDec, BEGIN}, 5},
        {{TypeDeclaration, TYPE}, 7},
        {{TypeDecList, ID}, 8},
        {{TypeDecMore, VAR}, 9},
        {{TypeDecMore, PROCEDURE}, 9},
        {{TypeDecMore, BEGIN}, 9},
        {{TypeDecMore, ID}, 10},
        {{TypeId, ID}, 11},
        {{TypeDef, INTEGER}, 12},
        {{TypeDef, CHARR}, 12},
        {{TypeDef, ARRAY}, 13},
        {{TypeDef, RECORD}, 13},
        {{TypeDef, ID}, 14},
        {{BaseType, INTEGER}, 15},
        {{BaseType, CHARR}, 16},
        {{StructureType, ARRAY}, 17},
        {{StructureType, RECORD}, 18},
        {{ArrayType, ARRAY}, 19},
        {{Low, INTC}, 20},
        {{Top, INTC}, 21},
        {{RecType, RECORD}, 22},
        {{FieldDecList, INTEGER}, 23},
        {{FieldDecList, CHARR}, 23},
        {{FieldDecList, ARRAY}, 24},
        {{FieldDecMore, INTEGER}, 26},
        {{FieldDecMore, CHARR}, 26},
        {{FieldDecMore, ARRAY}, 26},
        {{FieldDecMore, END}, 25},
        {{IdList, ID}, 27},
        {{IdMore, SEMI}, 28},
        {{IdMore, COMMA}, 29},
        {{VarDec, VAR}, 31},
        {{VarDec, PROCEDURE}, 30},
        {{VarDec, BEGIN}, 30},
        {{VarDeclaration, VAR}, 32},
        {{VarDecList, INTEGER}, 33},
        {{VarDecList, CHARR}, 33},
        {{VarDecList, ARRAY}, 33},
        {{VarDecList, RECORD}, 33},
        {{VarDecList, ID}, 33},
        {{VarDecMore, INTEGER}, 35},
        {{VarDecMore, CHARR}, 35},
        {{VarDecMore, ARRAY}, 35},
        {{VarDecMore, RECORD}, 35},
        {{VarDecMore, PROCEDURE}, 34},
        {{VarDecMore, BEGIN}, 34},
        {{VarDecMore, ID}, 35},
        {{VarIdList, ID}, 36},
        {{VarIdMore, SEMI}, 37},
        {{VarIdMore, COMMA}, 38},
        {{ProcDec, PROCEDURE}, 40},
        {{ProcDec, BEGIN}, 39},
        {{ProcDeclaration, PROCEDURE}, 41},
        {{ProcDecMore, PROCEDURE}, 41},
        {{ProcDecMore, BEGIN}, 42},
        {{ProcName, ID}, 44},
        {{ParamList, INTEGER}, 46},
        {{ParamList, CHARR}, 46},
        {{ParamList, ARRAY}, 46},
        {{ParamList, RECORD}, 46},
        {{ParamList, VAR}, 46},
        {{ParamList, ID}, 46},
        {{ParamList, RPAREN}, 45},
        {{ParamDecList, INTEGER}, 47},
        {{ParamDecList, CHARR}, 47},
        {{ParamDecList, ARRAY}, 47},
        {{ParamDecList, RECORD}, 47},
        {{ParamDecList, VAR}, 47},
        {{ParamDecList, ID}, 47},
        {{ParamMore, SEMI}, 49},
        {{ParamMore, RPAREN}, 48},
        {{Param, INTEGER}, 50},
        {{Param, CHARR}, 50},
        {{Param, ARRAY}, 50},
        {{Param, RECORD}, 50},
        {{Param, END}, 51},
        {{Param, ID}, 50},
        {{Param, VAR}, 51},
        {{FormList, ID}, 52},
        {{FidMore, SEMI}, 53},
        {{FidMore, COMMA}, 54},
        {{FidMore, RPAREN}, 53},
        {{ProcDecPart, TYPE}, 55},
        {{ProcDecPart, VAR}, 55},
        {{ProcDecPart, PROCEDURE}, 55},
        {{ProcDecPart, BEGIN}, 55},
        {{ProcBody, BEGIN}, 56},
        {{ProgramBody, BEGIN}, 57},
        {{StmList, IF}, 58},
        {{StmList, WHILE}, 58},
        {{StmList, READ}, 58},
        {{StmList, WRITE}, 58},
        {{StmList, RETURN}, 58},
        {{StmList, ID}, 58},
        {{StmMore, END}, 59},
        {{StmMore, ELSE}, 59},
        {{StmMore, FI}, 59},
        {{StmMore, ENDWH}, 59},
        {{StmMore, SEMI}, 60},
        {{Stm, IF}, 61},
        {{Stm, WHILE}, 62},
        {{Stm, READ}, 63},
        {{Stm, WRITE}, 64},
        {{Stm, RETURN}, 65},
        {{Stm, ID}, 66},
        {{AssCall, ASSIGN}, 67},
        {{AssCall, DOT}, 67},
        {{AssCall, LMIDPAREN}, 67},
        {{AssCall, LPAREN}, 68},
        {{AssignmentRest, DOT}, 69},
        {{AssignmentRest, LMIDPAREN}, 69},
        {{AssignmentRest, ASSIGN}, 69},
        {{ConditionalStm, IF}, 70},
        {{LoopStm, WHILE}, 71},
        {{InputStm, READ}, 72},
        {{InVar, ID}, 73},
        {{OutputStm, WRITE}, 74},
        {{ReturnStm, RETURN}, 75},
        {{CallStmRest, LPAREN}, 76},
        {{ActParamList, INTC}, 78},
        {{ActParamList, ID}, 78},
        {{ActParamList, LPAREN}, 78},
        {{ActParamList, RPAREN}, 77},
        {{ActParamMore, COMMA}, 80},
        {{ActParamMore, RPAREN}, 79},
        {{RelExp, INTC}, 81},
        {{RelExp, ID}, 81},
        {{RelExp, LMIDPAREN}, 81},
        {{OtherRelE, LT}, 82},
        {{OtherRelE, EQ}, 82},
        {{Exp, INTC}, 83},
        {{OtherFactor, END}, 87},
        {{OtherFactor, THEN}, 87},
        {{OtherFactor, ELSE}, 87},
        {{OtherFactor, FI}, 87},
        {{OtherFactor, DO}, 87},
        {{OtherFactor, ENDWH}, 87},
        {{OtherFactor, SEMI}, 87},
        {{OtherFactor, COMMA}, 87},
        {{OtherFactor, RPAREN}, 87},
        {{OtherFactor, RMIDPAREN}, 87},
        {{OtherFactor, LT}, 87},
        {{OtherFactor, EQ}, 87},
        {{OtherFactor, PLUS}, 87},
        {{OtherFactor, MINUS}, 87},
        {{OtherFactor, TIMES}, 88},
        {{OtherFactor, OVER}, 88},
        {{Term, INTC}, 86},
        {{Factor, INTC}, 90},
        {{OtherTerm, END}, 84},
        {{Term, END}, 87},
        {{VariMore, END}, 93},
        {{FieldVarMore, END}, 97},
        {{OtherTerm, THEN}, 84},
        {{Term, THEN}, 87},
        {{VariMore, THEN}, 93},
        {{FieldVarMore, THEN}, 97},
        {{OtherTerm, ELSE}, 84},
        {{Term, ELSE}, 87},
        {{VariMore, ELSE}, 93},
        {{FieldVarMore, ELSE}, 97},
        {{OtherTerm, FI}, 84},
        {{Term, FI}, 87},
        {{VariMore, FI}, 93},
        {{FieldVarMore, FI}, 97},
        {{OtherTerm, DO}, 84},
        {{Term, DO}, 87},
        {{VariMore, DO}, 93},
        {{FieldVarMore, DO}, 97},
        {{OtherTerm, ENDWH}, 84},
        {{Term, ENDWH}, 87},
        {{VariMore, ENDWH}, 93},
        {{FieldVarMore, ENDWH}, 97},
        {{Exp, ID}, 83},
        {{Term, ID}, 86},
        {{Factor, ID}, 91},
        {{Variable, ID}, 92},
        {{FieldVar, ID}, 96},
        {{VariMore, DOT}, 95},
        {{OtherTerm, SEMI}, 84},
        {{Term, SEMI}, 87},
        {{VariMore, SEMI}, 93},
        {{FieldVarMore, SEMI}, 97},
        {{OtherTerm, COMMA}, 84},
        {{Term, COMMA}, 87},
        {{VariMore, COMMA}, 93},
        {{FieldVarMore, COMMA}, 97},
        {{Exp, LPAREN}, 83},
        {{Term, LPAREN}, 86},
        {{Factor, LPAREN}, 89},
        {{OtherTerm, RPAREN}, 84},
        {{Term, RPAREN}, 87},
        {{VariMore, RPAREN}, 93},
        {{FieldVarMore, RPAREN}, 97},
        {{VariMore, LMIDPAREN}, 94},
        {{FieldVarMore, LMIDPAREN}, 98},
        {{OtherTerm, RMIDPAREN}, 84},
        {{Term, RMIDPAREN}, 87},
        {{VariMore, RMIDPAREN}, 93},
        {{FieldVarMore, RMIDPAREN}, 97},
        {{OtherTerm, LT}, 84},
        {{Term, LT}, 87},
        {{VariMore, LT}, 93},
        {{FieldVarMore, LT}, 97},
        {{CmpOp, LT}, 99},
        {{OtherTerm, EQ}, 84},
        {{Term, EQ}, 87},
        {{VariMore, EQ}, 93},
        {{FieldVarMore, EQ}, 97},
        {{CmpOp, EQ}, 100},
        {{OtherTerm, PLUS}, 85},
        {{Term, PLUS}, 87},
        {{VariMore, PLUS}, 93},
        {{FieldVarMore, PLUS}, 97},
        {{AddOp, PLUS}, 101},
        {{OtherTerm, MINUS}, 85},
        {{Term, MINUS}, 87},
        {{VariMore, MINUS}, 93},
        {{FieldVarMore, MINUS}, 97},
        {{AddOp, MINUS}, 102},
        {{Term, TIMES}, 88},
        {{VariMore, TIMES}, 93},
        {{FieldVarMore, TIMES}, 97},
        {{MultOp, TIMES}, 103},
        {{Term, OVER}, 88},
        {{VariMore, OVER}, 93},
        {{FieldVarMore, OVER}, 97},
        {{MultOp, OVER}, 104},
        {{VariMore, ASSIGN}, 93},
        {{FieldVarMore, ASSIGN}, 97},
        {{CmpOp, LT}, 99},
        {{CmpOp, EQ}, 100},
        {{AddOp, PLUS}, 101},
        {{AddOp, MINUS}, 102},
        {{MultOp, TIMES}, 103},
        {{MultOp, OVER}, 104},
    };
}
TreeNode* LL1::newSpecNode(NodeKind kind) {
    auto t = new TreeNode;
    int i;
    for (i = 0; i < 3; i++) t->child[i] = nullptr;
    t->sibling = nullptr;
    t->nodekind = kind;
    t->lineno = LL1::nowLine;
    t->idnum = 0;
    for (i = 0; i < 10; i++) t->table[i] = nullptr;
    return t;
}

TreeNode* LL1::newStmtNode(StmtKind kind) {
    auto t = newSpecNode(StmtK);
    t->kind.stmt = kind;
    return t;
}

TreeNode* LL1::newDecNode(DecKind kind) {
    auto t = newSpecNode(DecK);
    t->kind.dec = kind;
    return t;
}

TreeNode* LL1::newExpNode(ExpKind kind) {
    auto t = newSpecNode(ExpK);
    t->kind.exp = kind;
    return t;
}

SyntaxTree* LL1::parse(TokenList tokenList) {
    cur = 0;
    symbol_stack.push(Program);

    SyntaxTree* syntaxTree = new SyntaxTree();
    syntaxTree->setRoot(newSpecNode(ProK));

    syntaxtree_stack.push(&syntaxTree->getRoot()->child[2]);
    syntaxtree_stack.push(&syntaxTree->getRoot()->child[1]);
    syntaxtree_stack.push(&syntaxTree->getRoot()->child[0]);

    nowLine = tokenList[cur].getLineShow();

    while (!symbol_stack.empty()) {
        nowLine = tokenList[cur].getLineShow();
        if (NTSet.find(symbol_stack.top()) != NTSet.end()) {  //非终极符
            auto ss = symbol_stack.top();
            symbol_stack.pop();
            auto iter = table.find(
                pair<LexType, LexType>(ss, tokenList[cur].getLexType()));

            if (iter != table.cend())
                process(iter->second, tokenList);

            else
                syntaxError("非期望单词" +
                            lexName[tokenList[cur].getLexType()]);
        } else if (TTSet.find(symbol_stack.top()) != TTSet.end()) {  //终极符
            if (symbol_stack.top() == tokenList[cur].getLexType()) {
                symbol_stack.pop();
                cur++;
                nowLine = tokenList[cur].getLineShow();
            } else {
                syntaxError("非期望单词" +
                            lexName[tokenList[cur].getLexType()]);
                cur++;
            }
        }
    }
    return syntaxTree;
}

int LL1::Priosity(LexType op) {  //符号优先级标识
    int pri = 0;
    switch (op) {
        case END:
            pri = -1;
            break;
        case LPAREN:
            pri = 0;
            break;
        case LT:
        case EQ:
            pri = 1;
            break;
        case PLUS:
        case MINUS:
            pri = 2;
            break;
        case TIMES:
        case OVER:
            pri = 3;
            break;
        default:
            pri = -1;
    }
    return pri;
}

void LL1::process(int id, TokenList& tokenList) {
    switch (id) {
        case 1: {
            symbol_stack.push(DOT);
            symbol_stack.push(ProgramBody);
            symbol_stack.push(DeclarePart);
            symbol_stack.push(ProgramHead);
            break;
        }
        case 2: {
            symbol_stack.push(ProgramName);
            symbol_stack.push(PROGRAM);

            currentP = newSpecNode(PheadK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;
            break;
        }
        case 3: {
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 4: {
            symbol_stack.push(ProcDec);
            symbol_stack.push(VarDec);
            symbol_stack.push(TypeDec);
            break;
        }
        case 5: {
            break;
        }
        case 6: {
            symbol_stack.push(TypeDeclaration);
            break;
        }
        case 7: {
            symbol_stack.push(TypeDecList);
            symbol_stack.push(TYPE);

            currentP = newSpecNode(TypeK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 8: {
            symbol_stack.push(TypeDecMore);
            symbol_stack.push(SEMI);
            symbol_stack.push(TypeDef);
            symbol_stack.push(EQ);
            symbol_stack.push(TypeId);

            currentP = newSpecNode(DecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 9: {
            syntaxtree_stack.pop();
            break;
        }
        case 10: {
            symbol_stack.push(TypeDecList);
            break;
        }
        case 11: {
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 12: {
            symbol_stack.push(BaseType);
            temp = &(currentP->kind.dec);
            break;
        }
        case 13: {
            symbol_stack.push(StructureType);
            break;
        }
        case 14: {
            currentP->kind.dec = IdK;
            symbol_stack.push(ID);
            //       currentP->name[currentP->idnum++] =
            //       tokenList[cur].getSem();
            currentP->type_name = tokenList[cur].getSem();
            break;
        }
        case 15: {
            symbol_stack.push(INTEGER);
            (*temp) = IntegerK;
            break;
        }
        case 16: {
            symbol_stack.push(CHARR);
            (*temp) = CharK;
            break;
        }
        case 17: {
            symbol_stack.push(ArrayType);
            break;
        }
        case 18: {
            symbol_stack.push(RecType);
            break;
        }
        case 19: {
            symbol_stack.push(BaseType);
            symbol_stack.push(OF);
            symbol_stack.push(RMIDPAREN);
            symbol_stack.push(Top);
            symbol_stack.push(UNDERANGE);
            symbol_stack.push(Low);
            symbol_stack.push(LMIDPAREN);
            symbol_stack.push(ARRAY);

            currentP->kind.dec = ArrayK;
            temp = &currentP->attr.ArrayAttr.childtype;
            break;
        }
        case 20: {
            symbol_stack.push(INTC);
            currentP->attr.ArrayAttr.low =
                atoi(tokenList[cur].getSem().c_str());
            break;
        }
        case 21: {
            symbol_stack.push(INTC);
            currentP->attr.ArrayAttr.up = atoi(tokenList[cur].getSem().c_str());
            break;
        }
        case 22: {
            symbol_stack.push(END);
            symbol_stack.push(FieldDecList);
            symbol_stack.push(RECORD);
            currentP->kind.dec = RecordK;
            saveP = currentP;
            syntaxtree_stack.push(&(currentP->child[0]));
            break;
        }
        case 23: {
            symbol_stack.push(FieldDecMore);
            symbol_stack.push(SEMI);
            symbol_stack.push(IdList);
            symbol_stack.push(BaseType);

            currentP = newSpecNode(DecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            temp = &currentP->kind.dec;
            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 24: {
            symbol_stack.push(FieldDecMore);
            symbol_stack.push(SEMI);
            symbol_stack.push(IdList);
            symbol_stack.push(ArrayType);

            currentP = newSpecNode(DecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 25: {
            syntaxtree_stack.pop();
            currentP = saveP;
            break;
        }
        case 26: {
            symbol_stack.push(FieldDecList);
            break;
        }
        case 27: {
            symbol_stack.push(IdMore);
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 28: {
            break;
        }
        case 29: {
            symbol_stack.push(IdList);
            symbol_stack.push(COMMA);
            break;
        }
        case 30: {
            break;
        }
        case 31: {
            symbol_stack.push(VarDeclaration);
            break;
        }
        case 32: {
            symbol_stack.push(VarDecList);
            symbol_stack.push(VAR);

            currentP = newSpecNode(VarK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 33: {
            symbol_stack.push(VarDecMore);
            symbol_stack.push(SEMI);
            symbol_stack.push(VarIdList);
            symbol_stack.push(TypeDef);

            currentP = newSpecNode(DecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 34: {
            syntaxtree_stack.pop();
            break;
        }
        case 35: {
            symbol_stack.push(VarDecList);
            break;
        }
        case 36: {
            symbol_stack.push(VarIdMore);
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 37: {
            break;
        }
        case 38: {
            symbol_stack.push(VarIdList);
            symbol_stack.push(COMMA);
            break;
        }
        case 39: {
            break;
        }
        case 40: {
            symbol_stack.push(ProcDeclaration);
            /*currentP = newSpecNode(ProcK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;
            syntaxtree_stack.push(&currentP->child[0]);*/
            break;
        }
        case 41: {
            symbol_stack.push(ProcDecMore);
            symbol_stack.push(ProcBody);
            symbol_stack.push(ProcDecPart);
            symbol_stack.push(SEMI);
            symbol_stack.push(RPAREN);
            symbol_stack.push(ParamList);
            symbol_stack.push(LPAREN);
            symbol_stack.push(ProcName);
            symbol_stack.push(PROCEDURE);

            currentP = newSpecNode(ProcDecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            syntaxtree_stack.push(&currentP->child[2]);  // 语句
            syntaxtree_stack.push(&currentP->child[1]);  // 函数声明
            syntaxtree_stack.push(&currentP->child[0]);  // 参数声明
            break;
        }
        case 42: {
            break;
        }
        case 43: {
            symbol_stack.push(ProcDeclaration);
            break;
        }
        case 44: {
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 45: {
            syntaxtree_stack.pop();
            break;
        }
        case 46: {
            symbol_stack.push(ParamDecList);
            break;
        }
        case 47: {
            symbol_stack.push(ParamMore);
            symbol_stack.push(Param);
            break;
        }
        case 48: {
            syntaxtree_stack.pop();
            currentP = saveP;
            temp = &currentP->kind.dec;
            break;
        }
        case 49: {
            symbol_stack.push(ParamDecList);
            symbol_stack.push(SEMI);
            break;
        }
        case 50: {
            symbol_stack.push(FormList);
            symbol_stack.push(TypeDef);

            currentP = newSpecNode(DecK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            currentP->attr.ProcAttr.paramt = valparamType;
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 51: {
            symbol_stack.push(FormList);
            symbol_stack.push(TypeDef);
            symbol_stack.push(VAR);

            currentP = newSpecNode(DecK);
            currentP->attr.ProcAttr.paramt = varparamType;
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 52: {
            symbol_stack.push(FidMore);
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 53: {
            break;
        }
        case 54: {
            symbol_stack.push(FormList);
            symbol_stack.push(COMMA);
            break;
        }
        case 55: {
            symbol_stack.push(DeclarePart);
            break;
        }
        case 56: {
            symbol_stack.push(ProgramBody);
            break;
        }
        case 57: {
            symbol_stack.push(END);
            symbol_stack.push(StmList);
            symbol_stack.push(BEGIN);

            syntaxtree_stack.pop();

            currentP = newSpecNode(StmLK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 58: {
            symbol_stack.push(StmMore);
            symbol_stack.push(Stm);
            break;
        }
        case 59: {
            syntaxtree_stack.pop();
            break;
        }
        case 60: {
            symbol_stack.push(StmList);
            symbol_stack.push(SEMI);
            break;
        }
        case 61: {
            symbol_stack.push(ConditionalStm);

            currentP = newStmtNode(IfK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 62: {
            symbol_stack.push(LoopStm);

            currentP = newStmtNode(WhileK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 63: {
            symbol_stack.push(InputStm);

            currentP = newStmtNode(ReadK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 64: {
            symbol_stack.push(OutputStm);

            currentP = newStmtNode(WriteK);
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 65: {
            symbol_stack.push(ReturnStm);

            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            currentP = newStmtNode(ReturnK);
            (*t) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }

        case 66: {
            symbol_stack.push(AssCall);
            symbol_stack.push(ID);

            currentP = newStmtNode(AssignK);

            // 赋值语句左部变量节点
            TreeNode* t = newExpNode(VariK);
            t->name[t->idnum++] = tokenList[cur].getSem();
            currentP->child[0] = t;

            auto t1 = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t1) = currentP;

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 67: {
            symbol_stack.push(AssignmentRest);
            currentP->kind.stmt = AssignK;
            break;
        }
        case 68: {
            symbol_stack.push(CallStmRest);
            currentP->child[0]->attr.ExpAttr.varkind = IdV;
            currentP->kind.stmt = CallK;
            break;
        }
        case 69: {
            symbol_stack.push(Exp);
            symbol_stack.push(ASSIGN);
            symbol_stack.push(VariMore);

            syntaxtree_stack.push(&currentP->child[1]);
            currentP = currentP->child[0];

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);
            break;
        }
        case 70: {
            symbol_stack.push(FI);
            symbol_stack.push(StmList);
            symbol_stack.push(ELSE);
            symbol_stack.push(StmList);
            symbol_stack.push(THEN);
            symbol_stack.push(RelExp);
            symbol_stack.push(IF);

            syntaxtree_stack.push(&currentP->child[2]);
            syntaxtree_stack.push(&currentP->child[1]);
            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 71: {
            symbol_stack.push(ENDWH);
            symbol_stack.push(StmList);
            symbol_stack.push(DO);
            symbol_stack.push(RelExp);
            symbol_stack.push(WHILE);

            syntaxtree_stack.push(&currentP->child[1]);
            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 72: {
            symbol_stack.push(RPAREN);
            symbol_stack.push(InVar);
            symbol_stack.push(LPAREN);
            symbol_stack.push(READ);
            break;
        }
        case 73: {
            symbol_stack.push(ID);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            break;
        }
        case 74: {
            symbol_stack.push(RPAREN);
            symbol_stack.push(Exp);
            symbol_stack.push(LPAREN);
            symbol_stack.push(WRITE);

            syntaxtree_stack.push(&currentP->child[0]);

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);
            break;
        }
        case 75: {
            symbol_stack.push(RETURN);
            break;
        }
        case 76: {
            symbol_stack.push(RPAREN);
            symbol_stack.push(ActParamList);
            symbol_stack.push(LPAREN);

            syntaxtree_stack.push(&currentP->child[1]);
            break;
        }
        case 77: {
            syntaxtree_stack.pop();
            break;
        }
        case 78: {
            symbol_stack.push(ActParamMore);
            symbol_stack.push(Exp);

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);
            break;
        }
        case 79: {
            break;
        }
        case 80: {
            symbol_stack.push(ActParamList);
            symbol_stack.push(COMMA);

            syntaxtree_stack.push(&currentP->sibling);
            break;
        }
        case 81: {
            symbol_stack.push(OtherRelE);
            symbol_stack.push(Exp);

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);
            getExpResult = false;
            break;
        }
        case 82: {
            symbol_stack.push(Exp);
            symbol_stack.push(CmpOp);

            TreeNode* currentP = newExpNode(OpK);
            currentP->attr.ExpAttr.op = tokenList[cur].getLexType();

            while (Priosity(op_stack.top()->attr.ExpAttr.op) >=
                   Priosity(currentP->attr.ExpAttr.op)) {
                TreeNode* t = op_stack.top();
                op_stack.pop();
                TreeNode* Rnum = num_stack.top();
                num_stack.pop();
                TreeNode* Lnum = num_stack.top();
                num_stack.pop();
                t->child[0] = Lnum;
                t->child[1] = Rnum;
                num_stack.push(t);
            }
            op_stack.push(currentP);
            getExpResult = true;
            break;
        }
        case 83: {
            symbol_stack.push(OtherTerm);
            symbol_stack.push(Term);
            break;
        }
        case 84: {
            if ((tokenList[cur].getLexType() == RPAREN) && (expflag != 0)) {
                while (op_stack.top()->attr.ExpAttr.op != LPAREN) {
                    TreeNode* t = op_stack.top();
                    op_stack.pop();
                    TreeNode* Rnum = num_stack.top();
                    num_stack.pop();
                    TreeNode* Lnum = num_stack.top();
                    num_stack.pop();
                    t->child[0] = Lnum;
                    t->child[1] = Rnum;
                    num_stack.push(t);
                }
                op_stack.pop();  //弹出(
                expflag--;
            } else {
                if ((getExpResult) || (getExpResult2)) {
                    while (op_stack.top()->attr.ExpAttr.op != END) {
                        TreeNode* t = op_stack.top();
                        op_stack.pop();
                        TreeNode* Rnum = num_stack.top();
                        num_stack.pop();
                        TreeNode* Lnum = num_stack.top();
                        num_stack.pop();
                        t->child[0] = Lnum;
                        t->child[1] = Rnum;
                        num_stack.push(t);
                    }
                    op_stack.pop();  //弹出END
                    currentP = num_stack.top();
                    num_stack.pop();
                    auto t = syntaxtree_stack.top();
                    syntaxtree_stack.pop();
                    (*t) = currentP;

                    if (getExpResult2) getExpResult2 = false;
                }
            }
            break;
        }
        case 85: {
            symbol_stack.push(Exp);
            symbol_stack.push(AddOp);

            TreeNode* currentP = newExpNode(OpK);
            currentP->attr.ExpAttr.op = tokenList[cur].getLexType();

            while (Priosity(op_stack.top()->attr.ExpAttr.op) >=
                   Priosity(currentP->attr.ExpAttr.op)) {
                TreeNode* t = op_stack.top();
                op_stack.pop();
                TreeNode* Rnum = num_stack.top();
                num_stack.pop();
                TreeNode* Lnum = num_stack.top();
                num_stack.pop();
                t->child[0] = Lnum;
                t->child[1] = Rnum;
                num_stack.push(t);
            }
            op_stack.push(currentP);
            break;
        }
        case 86: {
            symbol_stack.push(OtherFactor);
            symbol_stack.push(Factor);
            break;
        }
        case 87: {
            break;
        }
        case 88: {
            symbol_stack.push(Term);
            symbol_stack.push(MultOp);

            TreeNode* currentP = newExpNode(OpK);
            currentP->attr.ExpAttr.op = tokenList[cur].getLexType();

            while (Priosity(op_stack.top()->attr.ExpAttr.op) >=
                   Priosity(currentP->attr.ExpAttr.op)) {
                TreeNode* t = op_stack.top();
                op_stack.pop();
                TreeNode* Rnum = num_stack.top();
                num_stack.pop();
                TreeNode* Lnum = num_stack.top();
                num_stack.pop();
                t->child[0] = Lnum;
                t->child[1] = Rnum;
                num_stack.push(t);
            }
            op_stack.push(currentP);
            break;
        }
        case 89: {
            symbol_stack.push(RPAREN);
            symbol_stack.push(Exp);
            symbol_stack.push(LPAREN);

            currentP = newExpNode(OpK);
            currentP->attr.ExpAttr.op = LPAREN;
            //      currentP->attr.ExpAttr.op = tokenList[cur].getLexType();
            op_stack.push(currentP);
            expflag++;
            break;
        }
        case 90: {
            symbol_stack.push(INTC);

            currentP = newExpNode(ConstK);
            currentP->attr.ExpAttr.val = atoi(tokenList[cur].getSem().c_str());
            num_stack.push(currentP);
            break;
        }
        case 91: {
            symbol_stack.push(Variable);
            break;
        }
        case 92: {
            symbol_stack.push(VariMore);
            symbol_stack.push(ID);

            currentP = newExpNode(VariK);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            num_stack.push(currentP);
            break;
        }
        case 93: {
            currentP->attr.ExpAttr.varkind = IdV;
            break;
        }
        case 94: {
            symbol_stack.push(RMIDPAREN);
            symbol_stack.push(Exp);
            symbol_stack.push(LMIDPAREN);

            currentP->attr.ExpAttr.varkind = ArrayMembV;
            syntaxtree_stack.push(&currentP->child[0]);

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);

            getExpResult2 = true;
            break;
        }
        case 95: {
            symbol_stack.push(FieldVar);
            symbol_stack.push(DOT);

            currentP->attr.ExpAttr.varkind = FieldMembV;
            syntaxtree_stack.push(&currentP->child[0]);
            break;
        }
        case 96: {
            symbol_stack.push(FieldVarMore);
            symbol_stack.push(ID);

            currentP = newExpNode(VariK);
            currentP->name[currentP->idnum++] = tokenList[cur].getSem();
            auto t = syntaxtree_stack.top();
            syntaxtree_stack.pop();
            (*t) = currentP;
            break;
        }
        case 97: {
            currentP->attr.ExpAttr.varkind = IdV;
            break;
        }
        case 98: {
            symbol_stack.push(RMIDPAREN);
            symbol_stack.push(Exp);
            symbol_stack.push(LMIDPAREN);

            currentP->attr.ExpAttr.varkind = ArrayMembV;
            syntaxtree_stack.push(&currentP->child[0]);

            TreeNode* t = newExpNode(OpK);
            t->attr.ExpAttr.op = END;
            op_stack.push(t);

            getExpResult2 = true;
            break;
        }
        case 99: {
            symbol_stack.push(LT);
            break;
        }
        case 100: {
            symbol_stack.push(EQ);
            break;
        }
        case 101: {
            symbol_stack.push(PLUS);
            break;
        }
        case 102: {
            symbol_stack.push(MINUS);
            break;
        }
        case 103: {
            symbol_stack.push(TIMES);
            break;
        }
        case 104: {
            symbol_stack.push(OVER);
            break;
        }
    }
}

void LL1::syntaxError(string error) {
    cout << "第 " << nowLine << " 行出现语法错误：" << error << endl << endl;
    cout << "LL1 语法分析失败！" << endl;
    exit(-1);
}