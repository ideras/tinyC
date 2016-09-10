#ifndef _AST_H_ 
#define _AST_H_

#include <string>
#include <list>
#include <map>
#include <sstream>

using namespace std;

enum DataType {DT_Int, DT_Double};

string newIntTemp();
string newFloatTemp();
void releaseFloatTemp(string &temp);
void releaseIntTemp(string &temp);
string getFTmpRegPair(string &ftmp_reg);

struct ExprContext {
    void castToDouble() {
	if (type == DT_Int) {
            string tempPlace, tempPlace_RegPair;
	    stringstream ss;
	    
	    tempPlace = newFloatTemp();
            tempPlace_RegPair = getFTmpRegPair(tempPlace);
	    
	    ss << code
               << "mtc1 " << this->place << ", " << tempPlace << endl
               << "mtc1 $zero, " << tempPlace_RegPair << endl
	       << "cvt.d.w " << tempPlace << ", " << tempPlace << endl;
            
            releaseIntTemp(this->place);
            
            this->type = DT_Double;
            this->place = tempPlace;
            this->code = ss.str();
	}
	else {
	    /* Nothing to do, the value is a float already */
	}
    }
    
    void castToInt() {
	if (type == DT_Int) {
	    /* Nothing to do, the value is a integer already */
	} else {
	    stringstream ss;
	    string tempPlace = newIntTemp();
	    
	    ss << code
               << "cvt.w.d " << this->place << ", " << this->place << endl
	       << "mfc1 " << tempPlace << ", " << this->place << endl;
	    
	    releaseFloatTemp(this->place);
            
            this->type = DT_Int;
            this->place = tempPlace;
            this->code = ss.str();
	}
    }
    
    void releasePlace() {
        if (type == DT_Int)
            releaseIntTemp(place);
        else
            releaseFloatTemp(place);
    }
    
    DataType type;
    string place;
    string code;
};

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

inline string addParenthesis(string &str)
{
    stringstream ss;
    
    ss << "(" << str << ")";
    return ss.str();
}

#define IMPLEMENT_BINARY_EXPR_TOSTRING(OP, PRECEDENCE)  \
    string toStringHelper(int &prec) {     \
        stringstream ss;    \
        string str1, str2;  \
        int prec1, prec2;   \
                            \
        str1 = expr1->toStringHelper(prec1); \
        str2 = expr2->toStringHelper(prec2); \
        if (prec1 < PRECEDENCE)          \
            str1 = addParenthesis(str1); \
        if (prec2 < PRECEDENCE)          \
            str2 = addParenthesis(str2); \
                                         \
        ss << str1 << " " OP " " << str2; \
        prec = PRECEDENCE;               \
        return ss.str();                 \
    }

class Expr {
public:
    virtual void generateCode(ExprContext &ctx) = 0;
    virtual string toStringHelper(int &prec) = 0;
    
    string toString() {
        int tmp;
        
        return toStringHelper(tmp);
    }
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
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("<", 10)
};

class GTExpr: public BinaryExpr {
public:
    GTExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING(">", 10)
};

class LTEExpr: public BinaryExpr {
public:
    LTEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("<=", 10)
};

class GTEExpr: public BinaryExpr {
public:
    GTEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING(">=", 10)
};

class NEExpr: public BinaryExpr {
public:
    NEExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("!=", 10)
};

class EQExpr: public BinaryExpr {
public:
    EQExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("==", 10)
};

class AddExpr: public BinaryExpr {
public:
    AddExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("+", 20)
};

class SubExpr: public BinaryExpr {
public:
    SubExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("-", 20)
};

class MultExpr: public BinaryExpr {
public:
    MultExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("*", 30)
};

class DivExpr: public BinaryExpr {
public:
    DivExpr(Expr *expr1, Expr *expr2): BinaryExpr(expr1, expr2) {}
    
    void generateCode(ExprContext &ctx);
    
    IMPLEMENT_BINARY_EXPR_TOSTRING("/", 30)
};

class IntNumExpr: public Expr {
public:
    IntNumExpr(string value) { this->value = value; }
    void generateCode(ExprContext &ctx);
    string toStringHelper(int &prec) { prec = 100; return value; }
    
    string value;
};

class FloatNumExpr: public Expr {
public:
    FloatNumExpr(string value) { this->value = value; }
    void generateCode(ExprContext &ctx);
    string toStringHelper(int &prec) { prec = 100; return value; }
    
    string value;
};

class IdExpr: public Expr {
public:
    IdExpr(string id) { this->id = id; }
    void generateCode(ExprContext &ctx);
    string toStringHelper(int &prec) { prec = 100; return id; }
    
    string id;
};

class IncExpr: public Expr {
public:
    IncExpr(string id) { this->id = id; }
    void generateCode(ExprContext &ctx);
    string toStringHelper(int &prec) { prec=100; return (id + "++"); }
    
    string id;
};

class DecExpr: public Expr {
public:
    DecExpr(string id) { this->id = id; }
    void generateCode(ExprContext &ctx);
    string toStringHelper(int &prec) { prec=100; return (id + "--"); }
    
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
    virtual string generateCode() = 0;
    virtual StatementKind getKind() = 0;
};

class BlockStatement: public Statement {
public:
    BlockStatement(list<Statement *> stList) { this->stList = stList; }
    string generateCode();
    StatementKind getKind() { return BLOCK_STATEMENT; }
    
    list<Statement *> stList;
};

class PrintStatement: public Statement {
public:
    PrintStatement(Expr *expr) { this->expr = expr; }
    string generateCode();
    StatementKind getKind() { return PRINT_STATEMENT; }
    
    Expr *expr;
};

class AssignStatement: public Statement {
public:
    AssignStatement(string id, Expr *expr) { 
        this->id = id;
        this->expr = expr; 
    }
    string generateCode();
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
    string generateCode();
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
    string generateCode();
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
    string generateCode();
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
    string generateCode();
    StatementKind getKind() { return EXPR_STATEMENT; }
    
    Expr *expr;
};


#endif

