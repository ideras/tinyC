#ifndef _AST_H_ 
#define _AST_H_

#include <string>
#include <list>
#include <map>

using namespace std;

enum DataType {DT_Int, DT_Double};

struct VValue {
    double doubleValue() {
	if (type == DT_Int)
	    return (double)u.ivalue;
	else
	    return u.dvalue;
    }
    
    int intValue() {
	if (type == DT_Int)
	    return u.ivalue;
	else
	    return (int)u.dvalue;
    }
    
    DataType type;
    
    union {
        int ivalue;
        double dvalue;
    } u;
};

extern map<string, VValue> vars;

class Declaration;

typedef list<string> StringList;
typedef list<Declaration*> DeclarationList;

class Declaration {
public:
    Declaration(DataType type, StringList &idList) {
        this->type = type;
        this->idList = idList;
    }
    
    DataType type;
    StringList idList;   
};


class Expr {
public:
    virtual VValue evaluate() = 0;
};

class BinaryExpr: public Expr {
public:
    BinaryExpr(Expr *expr1, Expr *expr2) {
        this->expr1 = expr1;
        this->expr2 = expr2;
    }
    
    Expr *expr1;
    Expr *expr2;
};

class LTExpr: public BinaryExpr {
public:
    LTExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class GTExpr: public BinaryExpr {
public:
    GTExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class LTEExpr: public BinaryExpr {
public:
    LTEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class GTEExpr: public BinaryExpr {
public:
    GTEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class NEExpr: public BinaryExpr {
public:
    NEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class EQExpr: public BinaryExpr {
public:
    EQExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class AddExpr: public BinaryExpr {
public:
    AddExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class SubExpr: public BinaryExpr {
public:
    SubExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class MultExpr: public BinaryExpr {
public:
    MultExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class DivExpr: public BinaryExpr {
public:
    DivExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    VValue evaluate();
};

class NumExpr: public Expr {
public:
    NumExpr(int value) { 
        this->value.type = DT_Int;
        this->value.u.ivalue = value; 
    }
    NumExpr(double value) { 
        this->value.type = DT_Double;
        this->value.u.dvalue = value; 
    }
    VValue evaluate() { return value; }
    
    VValue value;
};

class IdExpr: public Expr {
public:
    IdExpr(string id) { this->id = id; }
    VValue evaluate();
    
    string id;
};

class IncExpr: public Expr {
public:
    IncExpr(string id) { this->id = id; }
    VValue evaluate();
    
    string id;
};

class DecExpr: public Expr {
public:
    DecExpr(string id) { this->id = id; }
    VValue evaluate();
    
    string id;
};

enum StatementKind {
    BLOCK_STATEMENT,
    PRINT_STATEMENT,
    ASSIGN_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    EXPR_STATEMENT
};

class Statement {
public:
    virtual void execute() = 0;
    virtual StatementKind getKind() = 0;
};

class BlockStatement: public Statement {
public:
    BlockStatement(list<Statement *> stList) { this->stList = stList; }
    void execute();
    StatementKind getKind() { return BLOCK_STATEMENT; }
    
    list<Statement *> stList;
};

class PrintStatement: public Statement {
public:
    PrintStatement(Expr *expr) { this->expr = expr; }
    void execute();
    StatementKind getKind() { return PRINT_STATEMENT; }
    
    Expr *expr;
};

class AssignStatement: public Statement {
public:
    AssignStatement(string id, Expr *expr) { 
        this->id = id;
        this->expr = expr; 
    }
    void execute();
    StatementKind getKind() { return ASSIGN_STATEMENT; }
    
    string id;
    Expr *expr;
};

class IfStatement: public Statement {
public:
    IfStatement(Expr *cond, Statement *trueBlock, Statement *falseBlock) { 
        this->cond = cond; 
        this->trueBlock = trueBlock;
        this->falseBlock = falseBlock;
    }
    void execute();
    StatementKind getKind() { return IF_STATEMENT; }
    
    Expr *cond;
    Statement *trueBlock;
    Statement *falseBlock;
};

class WhileStatement: public Statement {
public:
    WhileStatement(Expr *cond, Statement *loopBody) { 
        this->cond = cond; 
        this->loopBody = loopBody;
    }
    void execute();
    StatementKind getKind() { return WHILE_STATEMENT; }
    
    Expr *cond;
    Statement *loopBody;
};

class ForStatement: public Statement {
public:
    ForStatement(Statement *initAssign, Expr *cond, Statement *incAssign, Statement *loopBody) { 
        this->initAssign = initAssign;
        this->cond = cond;
        this->incAssign = incAssign;
        this->loopBody = loopBody;
    }
    void execute();
    StatementKind getKind() { return FOR_STATEMENT; }
    
    Statement *initAssign;
    Statement *incAssign;
    Expr *cond;
    Statement *loopBody;
};

class ExprStatement: public Statement {
public:
    ExprStatement(Expr *expr) {
        this->expr = expr;
    }
    void execute();
    StatementKind getKind() { return EXPR_STATEMENT; }
    
    Expr *expr;
};


#endif

