#include "RD.h"

#include <string>
#include <vector>

RD::RD() {}

SyntaxTree* RD::parse(TokenList tokenList) {
    if (!tokenList.size()) syntaxError("文件提前结束！");
    this->tokenList = tokenList;
    cur = 0;
    SyntaxTree* syntaxTree = new SyntaxTree();
    syntaxTree->setRoot(program());

    fstream file("RelTree.txt", ios::out);
    file.seekg(0, ios::beg);
    for (string i : tr) file << i;

    return syntaxTree;
}

bool RD::match(LexType lexType) {
    Token token = tokenList[cur];
    if (tokenList[cur].getLexType() == lexType) {
        cur++;
        return 1;
    }
    return 0;
}

TreeNode* RD::program() {
    tr.push_back("[Program");
    TreeNode* t = programHead();
    TreeNode* q = declarePart();
    TreeNode* s = programBody();

    TreeNode* root = new TreeNode(ProK, tokenList[cur].getLineShow());

    if (t != nullptr)
        root->child[0] = t;
    else
        syntaxError("缺少程序头！");

    if (q != nullptr) root->child[1] = q;

    if (s != nullptr)
        root->child[2] = s;
    else
        syntaxError("缺少程序体！");

    if (!match(DOT)) syntaxError("缺少程序结束符\".\"！");
    tr.push_back(" .]");
    return root;
}

TreeNode* RD::programHead() {
    tr.push_back("[ProgramHead PROGRAM ");
    TreeNode* t = new TreeNode(PheadK, tokenList[cur].getLineShow());

    if (!match(PROGRAM)) syntaxError("缺少程序开始标志\"PROGRAM\"！");
    if (!match(ID))
        syntaxError("程序名字有误或缺失！");
    else
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
    tr.push_back("[ID ");
    tr.push_back(t->name[(t->idnum) - 1]);
    tr.push_back("]]");
    return t;
}

TreeNode* RD::declarePart() {
    tr.push_back("[DeclarePart ");
    TreeNode* typeP = new TreeNode(TypeK, tokenList[cur].getLineShow());
    if (typeP != nullptr) {
        TreeNode* tp1 = typeDec();
        if (tp1 != nullptr)
            typeP->child[0] = tp1;
        else {
            delete typeP;
            typeP = nullptr;
        }
    }
    TreeNode* varP = new TreeNode(VarK, tokenList[cur].getLineShow());
    if (varP != nullptr) {
        TreeNode* tp2 = varDec();
        if (tp2 != nullptr)
            varP->child[0] = tp2;
        else {
            delete varP;
            varP = nullptr;
        }
    }
    TreeNode* procP = procDec();
    /*TreeNode* procP = new TreeNode(ProcK, tokenList[cur].getLineShow());
    if (procP != nullptr) {
        TreeNode* tp3 = procDec();
        if (tp3 != nullptr)
            procP->child[0] = tp3;
        else {
            delete procP;
            procP = nullptr;
        }
    }*/
    if (varP == nullptr) varP = procP;
    if (typeP == nullptr) typeP = varP;
    if (typeP != varP) typeP->sibling = varP;
    if (varP != procP) varP->sibling = procP;
    tr.push_back("]");
    return typeP;
}

TreeNode* RD::typeDec() {
    tr.push_back("[TypeDecPart ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case TYPE:
            t = typeDeclaration();
            break;
        case VAR:
        case PROCEDURE:
        case BEGIN:
            tr.push_back("ε");
            break;
        default:
            syntaxErrorContinue("typeDec出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::typeDeclaration() {
    match(TYPE);
    tr.push_back("[TypeDec TYPE ");
    TreeNode* t = typeDecList();
    if (t == nullptr) syntaxError("缺少类型定义内容声明！");
    tr.push_back("]");
    return t;
}

TreeNode* RD::typeDecList() {
    TreeNode* t = new TreeNode(DecK, tokenList[cur].getLineShow());
    tr.push_back("[TypeDecList ");
    if (t != nullptr) {
        typeID(t);
        if (!match(EQ)) syntaxError("类型声明缺少\"=\"");
        tr.push_back(" = ");
        typeDef(t);
        if (!match(SEMI)) syntaxError("缺少\";\"");
        tr.push_back(" ; ");
        t->sibling = typeDecMore();
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::typeDecMore() {
    tr.push_back("[TypeDecMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case VAR:
        case PROCEDURE:
        case BEGIN:
            tr.push_back("ε");
            break;
        case ID:
            t = typeDecList();
            break;
        default:
            syntaxErrorContinue("typeDecMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

void RD::typeID(TreeNode* t) {
    if (!match(ID))
        syntaxError("类型声明缺少标识符！");
    else if (t != nullptr) {
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
        tr.push_back("[ID ");
        tr.push_back(t->name[(t->idnum) - 1]);
        tr.push_back("]");
    }
}

void RD::typeDef(TreeNode* t) {
    if (t == nullptr) return;
    tr.push_back("[Typedef ");
    switch (tokenList[cur].getLexType()) {
        case INTEGER:
        case CHARR:
            baseType(t);
            break;
        case ARRAY:
        case RECORD:
            structureType(t);
            break;
        case ID:
            match(ID);
            t->kind.dec = IdK;
            t->type_name = tokenList[cur - 1].getSem();
            tr.push_back("[ID ");
            tr.push_back(t->type_name);
            tr.push_back("]");
            break;
        default:
            syntaxErrorContinue("typeDef出现未知类型：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

void RD::baseType(TreeNode* t) {
    tr.push_back("[BaseType ");
    switch (tokenList[cur].getLexType()) {
        case INTEGER:
            match(INTEGER);
            tr.push_back(" INTEGER");
            t->kind.dec = IntegerK;
            break;
        case CHARR:
            match(CHARR);
            tr.push_back(" CHAR");
            t->kind.dec = CharK;
            break;
        default:
            syntaxErrorContinue("baseType出现未知类型：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

void RD::structureType(TreeNode* t) {
    tr.push_back("[StructureType ");
    switch (tokenList[cur].getLexType()) {
        case ARRAY:
            arrayType(t);
            break;
        case RECORD:
            recType(t);
            break;
        default:
            syntaxErrorContinue("structureType出现未知类型：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

void RD::arrayType(TreeNode* t) {
    tr.push_back("[ArrayType ");
    if (!match(ARRAY)) syntaxError("缺少数组声明：\"array\"！");
    if (!match(LMIDPAREN)) syntaxError("缺少符号：\"[\"！");
    tr.push_back(" ARRAY");
    tr.push_back(" \"[\"");
    if (!match(INTC)) syntaxError("缺少数组下界！");
    t->attr.ArrayAttr.low = stoi(tokenList[cur - 1].getSem());
    tr.push_back("[LOW \"" + tokenList[cur - 1].getSem() + "\"]");

    if (!match(UNDERANGE)) syntaxError("缺少符号：\"..\"！");
    tr.push_back(" ..");

    if (!match(INTC)) syntaxError("缺少数组上界！");
    t->attr.ArrayAttr.up = stoi(tokenList[cur - 1].getSem());
    tr.push_back("[TOP \"" + tokenList[cur - 1].getSem() + "\"]");

    if (!match(RMIDPAREN)) syntaxError("缺少符号：\"]\"！");
    tr.push_back(" \"]\"");

    if (!match(OF)) syntaxError("缺少数组类型声明前置符：\"of\"！");
    tr.push_back(" OF");

    baseType(t);
    t->attr.ArrayAttr.childtype = t->kind.dec;
    t->kind.dec = ArrayK;
    tr.push_back("]");
}

void RD::recType(TreeNode* t) {
    tr.push_back("[RecordType ");
    if (!match(RECORD)) syntaxError("缺少记录声明：\"record\"！");
    tr.push_back("RECORD");
    TreeNode* p = fieldDecList();
    if (p != nullptr)
        t->child[0] = p;
    else
        syntaxError("缺少记录体！");

    if (!match(END)) syntaxError("缺少记录结束声明：\"end\"！");
    t->kind.dec = DecKind::RecordK;
    tr.push_back("END]");
}

TreeNode* RD::fieldDecList() {
    tr.push_back("[FieldDecList");
    TreeNode* t = new TreeNode(DecK, tokenList[cur].getLineShow());
    TreeNode* p = nullptr;
    if (t == nullptr) return t;
    switch (tokenList[cur].getLexType()) {
        case INTEGER:
        case CHARR:
            baseType(t);
            idList(t);
            if (!match(SEMI)) syntaxError("缺少符号：\";\"！");
            tr.push_back(";");
            p = fieldDecMore();
            break;
        case ARRAY:
            arrayType(t);
            idList(t);
            if (!match(SEMI)) syntaxError("缺少符号：\";\"！");
            tr.push_back(";");
            p = fieldDecMore();
            break;
        case RECORD:
            recType(t);
            idList(t);
            if (!match(SEMI)) syntaxError("缺少符号：\";\"！");
            tr.push_back(";");
            p = fieldDecMore();
            break;
        default:
            syntaxErrorContinue("fieldDecList记录体出现预期外变量类型：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    t->sibling = p;
    tr.push_back("]");
    return t;
}

TreeNode* RD::fieldDecMore() {
    tr.push_back("[FieldDecMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case END:
            tr.push_back("ε");
            break;
        case INTEGER:
        case CHARR:
        case ARRAY:
        case RECORD:
            t = fieldDecList();
            break;
        default:
            syntaxErrorContinue("fieldDecMore记录体出现预期外变量类型：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

void RD::idList(TreeNode* t) {
    tr.push_back("[IdList");
    if (!match(ID)) syntaxError("缺少标识符！");
    {
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
        tr.push_back("[ID " + t->name[t->idnum - 1] + "]");
    }
    idMore(t);
    tr.push_back("]");
}

void RD::idMore(TreeNode* t) {
    tr.push_back("[IdMore ");
    switch (tokenList[cur].getLexType()) {
        case SEMI:
            tr.push_back("ε");
            break;
        case COMMA:
            match(COMMA);
            tr.push_back(",");
            idList(t);
            break;
        default:
            syntaxErrorContinue("idMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

TreeNode* RD::varDec() {
    tr.push_back("[VarDecPart ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case PROCEDURE:
        case BEGIN:
            tr.push_back("ε");
            break;
        case VAR:
            t = varDeclaration();
            break;
        default:
            syntaxErrorContinue("varDec出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::varDeclaration() {
    if (!match(VAR)) syntaxError("缺少变量声明：\"var\"");
    tr.push_back("[VarDec VAR ");
    TreeNode* t = varDecList();
    if (t == nullptr) syntaxError("缺少变量声明内容！");
    tr.push_back("]");
    return t;
}

TreeNode* RD::varDecList() {
    tr.push_back("[VarDecList ");
    TreeNode* t = new TreeNode(NodeKind::DecK, tokenList[cur].getLineShow());
    if (t == nullptr) return t;
    typeDef(t);
    varIDList(t);
    if (!match(SEMI)) syntaxError("缺少符号：\";\"");
    tr.push_back(";");
    t->sibling = varDecMore();
    tr.push_back("]");
    return t;
}

TreeNode* RD::varDecMore() {
    tr.push_back("[VarDecMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case PROCEDURE:
        case BEGIN:
            tr.push_back("ε");
            break;
        case INTEGER:
        case CHARR:
        case ARRAY:
        case RECORD:
        case ID:
            t = varDecList();
            break;
        default:
            syntaxErrorContinue("varDecMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

void RD::varIDList(TreeNode* t) {
    tr.push_back("[VarIdList ");
    if (!match(ID)) syntaxError("缺少变量标识符！");
    if (t != nullptr) {
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
        tr.push_back("[ID " + t->name[t->idnum - 1] + "]");
    }
    varIDMore(t);
    tr.push_back("]");
}

void RD::varIDMore(TreeNode* t) {
    tr.push_back("[VarIdMore ");
    switch (tokenList[cur].getLexType()) {
        case SEMI:
            tr.push_back("ε");
            break;
        case COMMA:
            match(COMMA);
            tr.push_back(",");
            varIDList(t);
            break;
        default:
            syntaxErrorContinue("varIDMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

TreeNode* RD::procDec() {
    TreeNode* t = nullptr;
    tr.push_back("[ProDecpart ");
    switch (tokenList[cur].getLexType()) {
        case BEGIN:
            tr.push_back("ε");
            break;
        case PROCEDURE:
            t = procDeclaration();
            break;
        default:
            syntaxErrorContinue("procDec出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::procDeclaration() {
    tr.push_back("[ProDec ");
    TreeNode* t = new TreeNode(ProcDecK, tokenList[cur].getLineShow());
    if (!match(PROCEDURE)) syntaxError("缺少函数声明：\"procedure\"！");
    tr.push_back("PROCEDURE");

    if (!match(ID)) syntaxError("缺少函数名标识符！");
    if (t != nullptr) {
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
        tr.push_back("[ProcName [ID " + t->name[t->idnum - 1] + "]]");
    }

    if (!match(LPAREN)) syntaxError("缺少符号：\"(\"！");
    tr.push_back("(");

    paramList(t);

    if (!match(RPAREN)) syntaxError("缺少符号：\")\"！");
    if (!match(SEMI)) syntaxError("缺少符号：\";\"！");
    tr.push_back(");");

    t->child[1] = procDecPart();
    t->child[2] = procBody();
    t->sibling = procDecMore();
    tr.push_back("]");
    return t;
}

TreeNode* RD::procDecMore() {
    tr.push_back("[ProDecMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case BEGIN:
            tr.push_back("ε");
            break;
        case PROCEDURE:
            t = procDeclaration();
            break;
        default:
            syntaxErrorContinue("procDecMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

void RD::paramList(TreeNode* t) {
    tr.push_back("[ParamList ");
    TreeNode* p = nullptr;
    switch (tokenList[cur].getLexType()) {
        case RPAREN:
            tr.push_back("ε");
            break;  //此处实验指导书上错了
        case INTEGER:
        case CHARR:
        case ARRAY:
        case RECORD:
        case ID:
        case VAR:
            p = paramDecList();
            t->child[0] = p;
            break;
        default:
            syntaxErrorContinue("paramList出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

TreeNode* RD::paramDecList() {
    tr.push_back("[ParamDecList");
    TreeNode* t = param();
    t->sibling = paramMore();
    tr.push_back("]");
    return t;
}

TreeNode* RD::paramMore() {
    tr.push_back("[ParamMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case RPAREN:
            tr.push_back("ε");
            break;
        case SEMI:
            match(SEMI);
            tr.push_back(";");
            t = paramDecList();
            if (t == nullptr) syntaxError("缺少函数声明！");
            break;
        default:
            syntaxErrorContinue("paramMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::param() {
    tr.push_back("[Param");
    TreeNode* t = new TreeNode(DecK, tokenList[cur].getLineShow());
    switch (tokenList[cur].getLexType()) {
        case VAR:
            match(VAR);
            tr.push_back(" VAR");
        case INTEGER:
        case CHARR:
        case ARRAY:
        case RECORD:
        case ID:
            t->attr.ProcAttr.paramt = valparamType;
            typeDef(t);
            formList(t);
            break;
        default:
            syntaxErrorContinue("param出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

void RD::formList(TreeNode* t) {
    tr.push_back("[FormList");
    if (!match(ID)) syntaxError("缺少参数标识符！");
    t->name[t->idnum++] = tokenList[cur - 1].getSem();
    tr.push_back("[ID " + t->name[t->idnum - 1] + "]");
    fidMore(t);
    tr.push_back("]");
}

void RD::fidMore(TreeNode* t) {
    tr.push_back("[FidMore ");
    switch (tokenList[cur].getLexType()) {
        case SEMI:
        case RPAREN:
            tr.push_back("ε");
            break;
        case COMMA:
            match(COMMA);
            tr.push_back(",");
            formList(t);
            break;
        default:
            syntaxErrorContinue("fidMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

TreeNode* RD::procDecPart() {
    tr.push_back("[ProcDecPart");
    TreeNode* t = declarePart();
    tr.push_back("]");
    return t;
}

TreeNode* RD::procBody() {
    tr.push_back("[ProcBody");
    TreeNode* t = programBody();
    if (t == nullptr) syntaxError("缺少程序体！");
    tr.push_back("]");
    return t;
}

TreeNode* RD::programBody() {
    tr.push_back("[ProgramBody ");
    if (!match(BEGIN)) syntaxError("缺少程序开始符：\"begin\"！");
    tr.push_back("BEGIN");

    TreeNode* t = new TreeNode(StmLK, tokenList[cur].getLineShow());
    if (t != nullptr) t->child[0] = stmList();

    if (!match(END)) syntaxError("缺少程序结束符：\"end\"！");
    tr.push_back("END]");
    return t;
}

TreeNode* RD::stmList() {
    tr.push_back("[StmList ");
    TreeNode* t = stm();
    if (t != nullptr) t->sibling = stmMore();
    tr.push_back("]");
    return t;
}

TreeNode* RD::stmMore() {
    tr.push_back("[StmMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case ELSE:
        case FI:
        case END:
        case ENDWH:
            tr.push_back("ε");
            break;
        case SEMI:
            match(SEMI);
            tr.push_back(";");
            t = stmList();
            break;
        default:
            syntaxErrorContinue("stmMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::stm() {
    tr.push_back("[Stm");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case IF:
            t = conditionalStm();
            break;
        case WHILE:
            t = loopStm();
            break;
        case READ:
            t = inputStm();
            break;
        case WRITE:
            t = outputStm();
            break;
        case RETURN:
            t = returnStm();
            break;
        case ID:
            match(ID);
            temp_name = tokenList[cur - 1].getSem();
            tr.push_back("[ID" + temp_name + "]");
            t = assCall();
            break;
        default:
            syntaxErrorContinue("stm出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::assCall() {
    tr.push_back("[AssCall");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case ASSIGN:
        case LMIDPAREN:
        case DOT:  //此处实验指导书上少了LMIDPAREN、DOT
            t = assignmentRest();
            break;
        case LPAREN:
            t = callStmRest();
            break;
        default:
            syntaxErrorContinue("assCall出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::assignmentRest() {
    tr.push_back("[AssignmentRest");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t == nullptr) return t;
    t->kind.stmt = AssignK;

    TreeNode* p = new TreeNode(ExpK, tokenList[cur].getLineShow());
    p->kind.exp = VariK;
    p->attr.ExpAttr.varkind = IdV;
    p->name[p->idnum++] = temp_name;
    variMore(p);

    if (!match(ASSIGN)) syntaxError("缺少符号：\":=\"");
    tr.push_back(":=");

    t->child[0] = p;
    t->child[1] = exp();

    tr.push_back("]");
    return t;
}

TreeNode* RD::conditionalStm() {
    tr.push_back("[ConditionalStm ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = IfK;

    if (!match(IF)) syntaxError("缺少条件语句标志符：\"if\"！");
    tr.push_back("IF");

    if (t != nullptr) t->child[0] = relExp();

    if (!match(THEN)) syntaxError("缺少条件语句标志符：\"then\"！");
    tr.push_back("THEN");

    if (t != nullptr) t->child[1] = stmList();

    if (match(ELSE))
        tr.push_back("ELSE");
    else {
        // syntaxError("缺少条件语句标志符：\"else\"！");
    }
    if (t != nullptr) t->child[2] = stmList();
    if (!match(FI)) syntaxError("缺少条件语句标志符：\"fi\"！");
    tr.push_back("FI]");
    return t;
}

TreeNode* RD::loopStm() {
    tr.push_back("[LoopStm ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = WhileK;

    if (!match(WHILE)) syntaxError("缺少循环语句标志符：\"while\"！");
    tr.push_back("WHILE");

    if (t != nullptr) t->child[0] = relExp();

    if (!match(DO)) syntaxError("缺少循环语句标志符：\"do\"！");
    tr.push_back("DO");

    if (t != nullptr) t->child[1] = stmList();

    if (!match(ENDWH)) syntaxError("缺少循环语句标志符：\"endwh\"！");
    tr.push_back("ENDWH]");
    return t;
}

TreeNode* RD::inputStm() {
    tr.push_back("[InputStm ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = ReadK;

    if (!match(READ)) syntaxError("缺少输入语句标志符：\"read\"！");
    if (!match(LPAREN)) syntaxError("缺少符号：\"(\"！");
    tr.push_back("READ (");

    if (!match(ID)) syntaxError("缺少输入标识符！");
    t->name[t->idnum++] = tokenList[cur - 1].getSem();
    tr.push_back("[ID" + t->name[t->idnum - 1] + "]");

    if (!match(RPAREN)) syntaxError("缺少符号：\")\"！");
    tr.push_back(")]");
    return t;
}

TreeNode* RD::outputStm() {
    tr.push_back("[OutputStm ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = WriteK;
    if (!match(WRITE)) syntaxError("缺少输出语句标志符：\"write\"！");
    if (!match(LPAREN)) syntaxError("缺少符号：\"(\"！");
    tr.push_back("WRITE (");
    t->child[0] = exp();
    if (!match(RPAREN)) syntaxError("缺少符号：\")\"！");
    tr.push_back(")]");
    return t;
}

TreeNode* RD::returnStm() {
    tr.push_back("[ReturnStm ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = ReturnK;
    if (!match(RETURN)) syntaxError("缺少返回语句标志符：\"return\"！");
    tr.push_back("RETURN]");
    return t;
}

TreeNode* RD::callStmRest() {
    tr.push_back("[CallStmRest ");
    TreeNode* t = new TreeNode(StmtK, tokenList[cur].getLineShow());
    if (t != nullptr) t->kind.stmt = CallK;

    if (!match(LPAREN)) syntaxError("缺少符号：\"(\"！");
    tr.push_back("(");
    if (t != nullptr) {
        TreeNode* p = new TreeNode(ExpK, tokenList[cur].getLineShow());
        p->kind.exp = VariK;
        p->attr.ExpAttr.varkind = IdV;
        p->name[t->idnum++] = temp_name;
        t->child[0] = p;
        t->child[1] = actParamList();
    }
    if (!match(RPAREN)) syntaxError("缺少符号：\")\"！");
    tr.push_back(")]");
    return t;
}

TreeNode* RD::actParamList() {
    tr.push_back("[ActParamList ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case RPAREN:
            tr.push_back("ε");
            break;
        case ID:
        case INTC:
            t = exp();
            if (t != nullptr) t->sibling = actParamMore();
            break;
        default:
            syntaxErrorContinue("actParamList出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::actParamMore() {
    tr.push_back("[ActParamMore ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case RPAREN:
            tr.push_back("ε");
            break;
        case COMMA:
            match(COMMA);
            tr.push_back(",");
            t = actParamList();
            break;
        default:
            syntaxErrorContinue("actParamMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::relExp() {
    tr.push_back("[RelExp ");
    TreeNode* t = exp();
    TreeNode* p = nullptr;
    switch (tokenList[cur].getLexType()) {
        case LT:
        case EQ:
            tr.push_back("[OtherRelE ");
            p = new TreeNode(ExpK, tokenList[cur].getLineShow());
            if (p == nullptr) {
                tr.push_back("ε");
            } else {
                p->kind.exp = OpK;
                p->child[0] = t;
                cmpOp(p);
                p->child[1] = exp();
            }
            t = p;
            tr.push_back("]");
            break;
        default:
            syntaxErrorContinue("relExp出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::exp() {
    tr.push_back("[Exp ");
    TreeNode* t = term();
    while (tokenList[cur].getLexType() == PLUS ||
           tokenList[cur].getLexType() == MINUS) {
        TreeNode* p = new TreeNode(ExpK, tokenList[cur].getLineShow());
        if (p != nullptr) {
            p->kind.exp = OpK;
            p->child[0] = t;
            addOp(p);
            p->child[1] = term();
        }
        t = p;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::term() {
    tr.push_back("[Term ");
    TreeNode* t = factor();
    while (tokenList[cur].getLexType() == TIMES ||
           tokenList[cur].getLexType() == OVER) {
        TreeNode* p = new TreeNode(ExpK, tokenList[cur].getLineShow());
        if (p != nullptr) {
            p->kind.exp = OpK;
            p->child[0] = t;
            multOp(p);
            p->child[1] = factor();
        }
        t = p;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::factor() {
    tr.push_back("[Factor ");
    TreeNode* t = nullptr;
    switch (tokenList[cur].getLexType()) {
        case INTC:
            t = new TreeNode(ExpK, tokenList[cur].getLineShow());
            t->kind.exp = ConstK;
            match(INTC);
            t->attr.ExpAttr.val = stoi(tokenList[cur - 1].getSem());
            tr.push_back("\"" + tokenList[cur - 1].getSem() + "\"");
            break;
        case ID:
            t = variable();
            break;
        case LPAREN:
            match(LPAREN);
            tr.push_back("(");
            t = exp();
            if (!match(RPAREN)) syntaxError("缺少符号：\")\"");
            tr.push_back(")");
            break;
        default:
            syntaxErrorContinue("factor出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
    return t;
}

TreeNode* RD::variable() {
    tr.push_back("[Variable ");
    TreeNode* t = new TreeNode(ExpK, tokenList[cur].getLineShow());
    if (t != nullptr) {
        t->kind.exp = VariK;
        t->attr.ExpAttr.varkind = IdV;
        if (!match(ID)) syntaxError("缺少变量标识符！");
        t->name[t->idnum++] = tokenList[cur - 1].getSem();
        tr.push_back("[ID " + t->name[t->idnum - 1] + "]");
    }
    variMore(t);
    tr.push_back("]");
    return t;
}

void RD::variMore(TreeNode* t) {
    tr.push_back("[VariMore ");
    switch (tokenList[cur].getLexType()) {
        case ASSIGN:
        case TIMES:
        case EQ:
        case LT:
        case PLUS:
        case MINUS:
        case OVER:
        case RPAREN:
        case RMIDPAREN:
        case SEMI:
        case COMMA:
        case THEN:
        case ELSE:
        case FI:
        case DO:
        case ENDWH:
        case END:
            tr.push_back("ε");
            break;
        case LMIDPAREN:
            match(LMIDPAREN);
            tr.push_back("\"[\"");
            t->attr.ExpAttr.varkind = ArrayMembV;
            t->child[0] = exp();
            if (!match(RMIDPAREN)) syntaxError("缺少符号：\"]\"！");
            tr.push_back("\"]\"");
            break;
        case DOT:
            match(DOT);
            tr.push_back(".");
            t->attr.ExpAttr.varkind = FieldMembV;
            t->child[0] = fieldVar();
            break;
        default:
            syntaxErrorContinue("variMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

TreeNode* RD::fieldVar() {
    tr.push_back("[FieldVar ");
    TreeNode* t = new TreeNode(ExpK, tokenList[cur].getLineShow());
    t->kind.exp = VariK;
    t->attr.ExpAttr.varkind = IdV;
    if (!match(ID)) syntaxError("缺少变量标识符！");
    t->name[t->idnum++] = tokenList[cur - 1].getSem();
    tr.push_back("[ID " + t->name[t->idnum - 1] + "]");

    fieldVarMore(t);
    tr.push_back("]");
    return t;
}

void RD::fieldVarMore(TreeNode* t) {
    tr.push_back("[FieldVarMore ");
    switch (tokenList[cur].getLexType()) {
        case ASSIGN:
        case TIMES:
        case EQ:
        case LT:
        case PLUS:
        case MINUS:
        case OVER:
        case RPAREN:
        case SEMI:
        case COMMA:
        case THEN:
        case ELSE:
        case FI:
        case DO:
        case ENDWH:
        case END:
            tr.push_back("ε");
            break;
        case LMIDPAREN:
            match(LMIDPAREN);
            tr.push_back("\"[\"");
            t->attr.ExpAttr.varkind = ArrayMembV;
            t->child[0] = exp();
            if (!match(RMIDPAREN)) syntaxError("缺少符号：\"]\"！");
            tr.push_back("\"]\"");
            break;
        default:
            syntaxErrorContinue("fieldVarMore出现预期外的符号：\"" +
                                tokenList[cur].getSem() + "\"！");
            cur++;
            break;
    }
    tr.push_back("]");
}

void RD::cmpOp(TreeNode* t) {
    tr.push_back("[CmpOp ");
    if (match(LT)) {
        t->attr.ExpAttr.op = LT;
        tr.push_back("<");
    } else if (match(EQ)) {
        t->attr.ExpAttr.op = EQ;
        tr.push_back("=");
    } else
        syntaxError("缺少比较运算符！");
    tr.push_back("]");
}

void RD::addOp(TreeNode* t) {
    tr.push_back("[AddOp ");
    if (match(PLUS)) {
        t->attr.ExpAttr.op = PLUS;
        tr.push_back("+");
    } else if (match(MINUS)) {
        t->attr.ExpAttr.op = MINUS;
        tr.push_back("-");
    } else
        syntaxError("缺少加减运算符！");
    tr.push_back("]");
}

void RD::multOp(TreeNode* t) {
    tr.push_back("[MulOp ");
    if (match(TIMES)) {
        t->attr.ExpAttr.op = TIMES;
        tr.push_back("*");
    } else if (match(OVER)) {
        t->attr.ExpAttr.op = OVER;
        tr.push_back("/");
    } else
        syntaxError("缺少乘除运算符！");
    tr.push_back("]");
}

void RD::syntaxError(string error) {
    cout << "第 " << tokenList[cur].getLineShow() << " 行出现语法错误："
         << error << endl
         << endl;
    cout << "递归下降语法分析失败！" << endl;
    exit(-1);
}

void RD::syntaxErrorContinue(string error) {
    cout << "第 " << tokenList[cur].getLineShow() << " 行出现语法错误："
         << error << endl
         << endl;
}
