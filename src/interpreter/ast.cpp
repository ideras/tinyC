#include <cstdio>
#include <cstdlib>
#include "ast.h"

extern DeclarationList *decl_list;
map<string, VValue> vars;

#define IMPLEMENT_RELATIONAL_EVALUATE(CLASS, OP)   \
    VValue CLASS::evaluate()            \
    {                                   \
        VValue value1 = expr1->evaluate();  \
        VValue value2 = expr2->evaluate();  \
        VValue result;                      \
                                            \
        result.type = DT_Int;               \
                                            \
        if (value1.type == DT_Int && value2.type == DT_Int)         \
            result.u.ivalue = value1.u.ivalue OP value2.u.ivalue;    \
        else {              \
            double d1, d2;  \
                            \
            d1 = value1.doubleValue();  \
            d2 = value2.doubleValue();  \
                                        \
            result.u.ivalue = d1 OP d2;  \
        }                               \
                                        \
        return result;                  \
    }

#define IMPLEMENT_ARITHMETIC_EVALUATE(CLASS, OP)    \
    VValue CLASS::evaluate()                \
    {                                       \
        VValue value1 = expr1->evaluate();  \
        VValue value2 = expr2->evaluate();  \
        VValue result;                      \
                                            \
        if (value1.type == DT_Int && value2.type == DT_Int) {       \
            result.type = DT_Int;                                   \
            result.u.ivalue = value1.u.ivalue OP value2.u.ivalue;   \
        }                                                           \
        else {              \
            double d1, d2;  \
                            \
            result.type = DT_Double;    \
            d1 = value1.doubleValue();  \
            d2 = value2.doubleValue();  \
                                        \
            result.u.dvalue = d1 OP d2; \
        }                               \
                                        \
        return result;                  \
    }

IMPLEMENT_RELATIONAL_EVALUATE(LTExpr, <)
IMPLEMENT_RELATIONAL_EVALUATE(LTEExpr, <=)
IMPLEMENT_RELATIONAL_EVALUATE(GTExpr, >)
IMPLEMENT_RELATIONAL_EVALUATE(GTEExpr, >=)
IMPLEMENT_RELATIONAL_EVALUATE(NEExpr, !=)
IMPLEMENT_RELATIONAL_EVALUATE(EQExpr, ==)

IMPLEMENT_ARITHMETIC_EVALUATE(AddExpr, +)
IMPLEMENT_ARITHMETIC_EVALUATE(SubExpr, -)
IMPLEMENT_ARITHMETIC_EVALUATE(MultExpr, *)
IMPLEMENT_ARITHMETIC_EVALUATE(DivExpr, /)

VValue IdExpr::evaluate() 
{    
    if (vars.find(id) == vars.end()) {
        printf("Identifier '%s' has not been declared\n", id.c_str());
        exit(1);
    }
    
    return vars[id];
}

VValue IncExpr::evaluate() 
{ 
    VValue value = vars[id];
    
    switch (value.type) {
        case DT_Int:
            value.u.ivalue++;
            break;
        case DT_Double:
            value.u.dvalue++;
            break;
    }
    vars[id] = value; 
    
    return value;
}

VValue DecExpr::evaluate() 
{
    VValue value = vars[id];
    
    switch (value.type) {
        case DT_Int:
            value.u.ivalue--;
            break;
        case DT_Double:
            value.u.dvalue--;
            break;
    }
    vars[id] = value; 
    return value;
}

void BlockStatement::execute()
{
    list<Statement *>::iterator it = stList.begin();
    
    while (it != stList.end()) {
        Statement *st = *it;
        
        st->execute();
        it++;
    }   
}

void PrintStatement::execute() 
{
    VValue result = expr->evaluate();
    
    switch (result.type) {
        case DT_Int:
            printf("%d\n", result.u.ivalue);
            break;
        case DT_Double:
            printf("%lf\n", result.u.dvalue);
            break;
    }
}

void AssignStatement::execute()
{
    if (vars.find(id) == vars.end()) {
        printf("Identifier '%s' has not been declared\n", id.c_str());
        exit(1);
    }
    
    VValue result = expr->evaluate();
    VValue vv = vars[id];
    
    if (vv.type == DT_Int)
       vv.u.ivalue = result.intValue();
    else
      vv.u.dvalue = result.doubleValue();
    
    vars[id] = vv;
}

void IfStatement::execute()
{
    VValue result = cond->evaluate();
    
    if (result.type != DT_Int) {
        printf("Condition of 'if' statement should be int type\n");
        exit(1);
    }
    if (result.u.ivalue) {
        trueBlock->execute();
    } else if (falseBlock != 0) {
        falseBlock->execute();
    }
}

void WhileStatement::execute()
{
    VValue result = cond->evaluate();

    if (result.type != DT_Int) {
        printf("Condition of 'while' statement should be int type\n");
        exit(1);
    }
    
    while (result.u.ivalue) {
        loopBody->execute();
        
        result = cond->evaluate();
    }
}

void ForStatement::execute()
{
    VValue result;
    
    if (initAssign != NULL)
        initAssign->execute();
    
    result = cond->evaluate();
    
    if (result.type != DT_Int) {
        printf("Condition of 'for' statement should be int type\n");
        exit(1);
    }
    
    while (result.u.ivalue) {
        loopBody->execute();
        
        if (incAssign != NULL)
            incAssign->execute();

        result = cond->evaluate();
    }
}

void ExprStatement::execute()
{
    expr->evaluate();
}
