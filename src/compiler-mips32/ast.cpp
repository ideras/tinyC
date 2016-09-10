#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include "ast.h"

extern DeclarationList *decl_list;
map<string, DataType> vars;
map<string, string> float_constants;

enum RelOp { RO_LT, RO_LE, RO_GT, RO_GE, RO_EQ, RO_NE };
enum ArithOp { AO_ADD, AO_SUB, AO_MUL, AO_DIV };

const char *i_temps[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};

const char *f_temps[] = {
                         "$f0", "$f2", "$f4", "$f6",
			 "$f8", "$f10", "$f12", "$f14",
			 "$f16", "$f18", "$f20", "$f22",
			 "$f24", "$f26", "$f28", "$f30"
		        };

#define ITMP_COUNT (sizeof(i_temps)/sizeof(i_temps[0]))
#define FTMP_COUNT (sizeof(f_temps)/sizeof(f_temps[0]))

set<string> itmp_map;
set<string> ftmp_map;
map<string, string> ftmp_pair_map;
int labelCount = 0;
int dconstCount = 0;

string newLabel()
{
    stringstream ss;
    
    ss << "L" << labelCount;
    
    labelCount++;
    
    return ss.str();
}

string newDoubleConstantName()
{
    stringstream ss;
    
    ss << "DC" << dconstCount;
    
    dconstCount++;
    
    return ss.str();
}

string newIntTemp()
{
    int i;
    
    for (i=0; i<ITMP_COUNT; i++) {
        if (itmp_map.find(i_temps[i]) == itmp_map.end()) {
            itmp_map.insert(i_temps[i]);
            
            return string(i_temps[i]);
        }
    }
    
    return string("");
}

void releaseIntTemp(string &temp)
{
    itmp_map.erase(temp);
}

string newFloatTemp()
{
    int i;
    
    for (i=0; i<FTMP_COUNT; i++) {
        if (ftmp_map.find(f_temps[i]) == ftmp_map.end()) {
            ftmp_map.insert(f_temps[i]);
            
            return string(f_temps[i]);
        }
    }
    
    return string("");
}

void releaseFloatTemp(string &temp)
{
    ftmp_map.erase(temp);
}

string getFTmpRegPair(string &ftmp_reg)
{
    if (ftmp_pair_map.empty()) {
        //Init map
        ftmp_pair_map["$f0"] = "$f1";
        ftmp_pair_map["$f2"] = "$f3";
        ftmp_pair_map["$f4"] = "$f5";
        ftmp_pair_map["$f6"] = "$f7";
        ftmp_pair_map["$f8"] = "$f9";
        ftmp_pair_map["$f10"] = "$f11";
        ftmp_pair_map["$f12"] = "$f13";
        ftmp_pair_map["$f14"] = "$f15";
        ftmp_pair_map["$f16"] = "$f17";
        ftmp_pair_map["$f18"] = "$f19";
        ftmp_pair_map["$f20"] = "$f21";
        ftmp_pair_map["$f22"] = "$f23";
        ftmp_pair_map["$f24"] = "$f25";
        ftmp_pair_map["$f26"] = "$f27";
        ftmp_pair_map["$f28"] = "$f29";
        ftmp_pair_map["$f30"] = "$f31";
    }
    
    return ftmp_pair_map[ftmp_reg];
}

void inline int_rel_codegen(ExprContext &ctx1, ExprContext &ctx2, ExprContext &ctx, RelOp op)
{
    stringstream ss;

    ctx.place = newIntTemp();

    switch (op) {
        case RO_LT: {
            ss << "slt " << ctx.place << ", " << ctx1.place << ", " << ctx2.place << endl;
            break;
        }           
        case RO_GT: {
            ss << "slt " << ctx.place << ", " << ctx2.place << ", " << ctx1.place << endl;
            break;
        }
        case RO_LE: {
            ss << "slt " << ctx.place << ", " << ctx2.place << ", " << ctx1.place << endl
               << "nor " << ctx.place << ", " << ctx.place << ", " << ctx.place << endl
               << "andi " << ctx.place << ", " << ctx.place << ", 1" << endl;
            break;
        } 
        case RO_GE: {
            ss << "slt " << ctx.place << ", " << ctx1.place << ", " << ctx2.place << endl
               << "nor " << ctx.place << ", " << ctx.place << ", " << ctx.place << endl
               << "andi " << ctx.place << ", " << ctx.place << ", 1" << endl;
            break;
        }
        case RO_EQ: {
            string lbl_true = newLabel();
            string lbl_end = newLabel();

            ss << "beq " << ctx1.place << ", " << ctx2.place << ", " << lbl_true << endl
               << "li " << ctx.place << ", 0" << endl
               << "j " << lbl_end  << endl
               << lbl_true << ":" << endl
               << "li " << ctx.place << ", 1" << endl
               << lbl_end << ":" << endl;

            break;
        }
        case RO_NE: {
            string lbl_true = newLabel();
            string lbl_end = newLabel();

            ss << "bne " << ctx1.place << ", " << ctx2.place << ", " << lbl_true << endl
               << "li " << ctx.place << ", 0" << endl
               << "j " << lbl_end  << endl
               << lbl_true << ":" << endl
               << "li " << ctx.place << ", 1" << endl
               << lbl_end << ":" << endl;

            break;
        }
        default:
            cout << __FUNCTION__ << ": BUG in the machine" << endl;
            exit(1);
    }

    ctx.code = ss.str();
}

#define FLOAT_REL_CODEGEN(ASM_OPCODE, REL_OP) \
    do {                           \
        string ltrue = newLabel(); \
        string lend = newLabel();  \
                                   \
        ctx.place = newIntTemp();  \
        if ((REL_OP != RO_GT) && (REL_OP != RO_GE)) \
            ss << ASM_OPCODE " " << ctx1.place << ", " << ctx2.place << endl; \
        else                       \
            ss << ASM_OPCODE " " << ctx2.place << ", " << ctx1.place << endl; \
                                                 \
        ss << "bc1t " << ltrue << endl           \
          << "li " << ctx.place << ", 0" << endl \
          << "j " << lend << endl                \
          << ltrue << ":" << endl                \
          << "li " << ctx.place << ", 1" << endl \
          << lend << ":" << endl;                \
    } while (0)
  
#define IMPLEMENT_RELATIONAL_CODEGEN(CLASS, REL_OP, FASM_OPCODE) \
    void CLASS::generateCode(ExprContext &ctx)                  \
    {                                   \
	ExprContext ctx1, ctx2;         \
	stringstream ss;                \
					\
        expr1->generateCode(ctx1);      \
        expr2->generateCode(ctx2);      \
                                        \
        ctx.type = DT_Int;              \
                                        \
        if (ctx1.type == DT_Int && ctx2.type == DT_Int) { \
            ctx1.releasePlace();                          \
            ctx2.releasePlace();                          \
            int_rel_codegen(ctx1, ctx2, ctx, REL_OP);     \
	    ss << ctx1.code                               \
	       << ctx2.code                               \
	       << ctx.code;                               \
	}                                                 \
        else {                            \
            ctx1.castToDouble();          \
            ctx2.castToDouble();          \
                                          \
            ctx1.releasePlace();          \
            ctx2.releasePlace();          \
                                          \
	    ss << ctx1.code << ctx2.code; \
            FLOAT_REL_CODEGEN(FASM_OPCODE, REL_OP); \
        }                                                  \
        ctx.code = ss.str();                               \
    }

inline string int_arith_codegen(ExprContext &ctx1, ExprContext &ctx2, ExprContext &ctx, ArithOp aop)
{
    stringstream ss;
    
    ctx.place = newIntTemp();
    
    switch (aop) {
        case AO_ADD: ss << "add " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        case AO_SUB: ss << "sub " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        case AO_MUL: {
            ss << "mult " << ctx1.place << ", " << ctx2.place << endl
               << "mflo " << ctx.place;
            break;
        }
        case AO_DIV: {
            ss << "div " << ctx1.place << ", " << ctx2.place << endl
               << "mflo " << ctx.place;
            break;
        }
        default:
            break;
    }
    
    return ss.str();
}

inline string float_arith_codegen(ExprContext &ctx1, ExprContext &ctx2, ExprContext &ctx, ArithOp aop)
{
    stringstream ss;
    
    ctx.place = newFloatTemp();
    
    switch (aop) {
        case AO_ADD: ss << "add.d " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        case AO_SUB: ss << "sub.d " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        case AO_MUL: ss << "mul.d " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        case AO_DIV: ss << "div.d " << ctx.place << ", " << ctx1.place << ", " << ctx2.place; break;
        default:
            break;
    }
    
    return ss.str();
}

#define IMPLEMENT_ARITHMETIC_CODEGEN(CLASS, OP)    \
    void CLASS::generateCode(ExprContext &ctx)     \
    {                                              \
	ExprContext ctx1, ctx2; \
	stringstream ss;        \
        int itmp;               \
                                \
        expr1->generateCode(ctx1); \
        expr2->generateCode(ctx2); \
                                   \
        if (ctx1.type == DT_Int && ctx2.type == DT_Int) { \
            ctx.type = DT_Int;                            \
            ctx1.releasePlace();                          \
            ctx2.releasePlace();                          \
            ss << ctx1.code                               \
	       << ctx2.code                               \
	       << int_arith_codegen(ctx1, ctx2, ctx, OP) << endl;   \
	}                                                 \
        else {                    \
            ctx.type = DT_Double; \
            ctx1.castToDouble();  \
            ctx2.castToDouble();  \
                                  \
            ctx1.releasePlace();  \
            ctx2.releasePlace();  \
                                  \
	    ss << ctx1.code << ctx2.code  \
	       << float_arith_codegen(ctx1, ctx2, ctx, OP) << endl;      \
                                                           \
        }                                                  \
        ctx.code = ss.str();                               \
    }

/*
 * Code generation for relational expressions
 */
IMPLEMENT_RELATIONAL_CODEGEN(LTExpr, RO_LT, "c.lt.d")
IMPLEMENT_RELATIONAL_CODEGEN(LTEExpr, RO_LE, "c.le.d")
IMPLEMENT_RELATIONAL_CODEGEN(GTExpr, RO_GT, "c.lt.d")
IMPLEMENT_RELATIONAL_CODEGEN(GTEExpr, RO_GE, "c.le.d")
IMPLEMENT_RELATIONAL_CODEGEN(NEExpr, RO_NE, "c.ne.d")
IMPLEMENT_RELATIONAL_CODEGEN(EQExpr, RO_EQ, "c.eq.d")

/*
 * Code generation for arithmetic expressions
 */
IMPLEMENT_ARITHMETIC_CODEGEN(AddExpr, AO_ADD)
IMPLEMENT_ARITHMETIC_CODEGEN(SubExpr, AO_SUB)
IMPLEMENT_ARITHMETIC_CODEGEN(MultExpr, AO_MUL)
IMPLEMENT_ARITHMETIC_CODEGEN(DivExpr, AO_DIV)

void IdExpr::generateCode(ExprContext &ctx) 
{    
    if (vars.find(id) == vars.end()) {
        printf("Identifier '%s' has not been declared\n", id.c_str());
        exit(1);
    }
    
    ctx.type = vars[id];
    stringstream ss;
    
    if (ctx.type == DT_Int) {
        ctx.place = newIntTemp();
    
        ss << "la " << ctx.place << ", " << id << endl
	   << "lw " << ctx.place << ", 0(" << ctx.place << ")" << endl;
    } else {
        string intPlace = newIntTemp();
        ctx.place = newFloatTemp();
        
        ss << "la " << intPlace << ", " << id << endl
	   << "ldc1 " << ctx.place << ", 0(" << intPlace << ")" << endl;

        releaseIntTemp(intPlace);
    }
    
    ctx.code = ss.str();
}

void IntNumExpr::generateCode(ExprContext& ctx)
{
    stringstream ss;
    
    ctx.place = newIntTemp();
    ctx.type = DT_Int;
    
    ss << "li " << ctx.place << ", " << value << endl;
    
    ctx.code = ss.str();
}

void FloatNumExpr::generateCode(ExprContext& ctx)
{
    stringstream ss;
    string label;
    
    string intPlace = newIntTemp();
    ctx.place = newFloatTemp();
    ctx.type = DT_Double;
    
    if (float_constants.find(value) != float_constants.end()) {
        label = float_constants[value];
    } else {
        label = newDoubleConstantName();
        float_constants[value] = label;
    }
    
    ss << "la " << intPlace << ", " << label << endl
       << "ldc1 " << ctx.place << ", 0(" << intPlace << ")" << endl;
    
    releaseIntTemp(intPlace);
    
    ctx.code = ss.str();
}

void IncExpr::generateCode(ExprContext& ctx)
{ 
    stringstream ss;
    DataType vtype = vars[id];
    
    switch (vtype) {
        case DT_Int: {
            string itmpPlace1 = newIntTemp();
            
            ctx.type = DT_Int;
            ctx.place = newIntTemp();
            
            ss << "la " << itmpPlace1 << ", " << id << endl
               << "lw " << ctx.place << ", 0(" << itmpPlace1 << ")" << endl
               << "addi " << ctx.place << ", " << ctx.place << ", 1" << endl
               << "sw " << ctx.place << ", 0(" << itmpPlace1 << ")" << endl;

            releaseIntTemp(itmpPlace1);
            
            ctx.code = ss.str();
            
            break;
        }
        case DT_Double: {
            stringstream ss;
            string constName;

            string intPlace = newIntTemp();
            string ftmpPlace = newFloatTemp();
            ctx.place = newFloatTemp();
            ctx.type = DT_Double;

            if (float_constants.find("1.0") != float_constants.end()) {
                constName = float_constants["1.0"];
            } else {
                constName = newDoubleConstantName();
                float_constants["1.0"] = constName;
            }

            ss << "la " << intPlace << ", " << constName << endl
               << "ldc1 " << ftmpPlace << ", 0(" << intPlace << ")" << endl
               << "la " << intPlace << ", " << id << endl
               << "ldc1 " << ctx.place << ", 0(" << intPlace << ")" << endl
               << "add.d " << ctx.place << ", " << ctx.place << ", " << ftmpPlace << endl
               << "sdc1 " << ctx.place << ", 0(" << intPlace << ")" << endl;

            releaseIntTemp(intPlace);
            releaseFloatTemp(ftmpPlace);

            ctx.code = ss.str();
            break;
        }
        default:
            cout << "BUG in the MACHINE" << endl;
            exit(1);
    }
}

void DecExpr::generateCode(ExprContext& ctx)
{
    stringstream ss;
    DataType vtype = vars[id];
    
    switch (vtype) {
        case DT_Int: {
            string itmpPlace1 = newIntTemp();
            
            ctx.type = DT_Int;
            ctx.place = newIntTemp();
            
            ss << "la " << itmpPlace1 << ", " << id << endl
               << "lw " << ctx.place << ", 0(" << itmpPlace1 << ")" << endl
               << "addi " << ctx.place << ", " << ctx.place << ", -1" << endl
               << "sw " << ctx.place << ", 0(" << itmpPlace1 << ")" << endl;

            releaseIntTemp(itmpPlace1);
            
            ctx.code = ss.str();
            
            break;
        }
        case DT_Double: {
            stringstream ss;
            string constName;

            string intPlace = newIntTemp();
            string ftmpPlace = newFloatTemp();
            ctx.place = newFloatTemp();
            ctx.type = DT_Double;

            if (float_constants.find("-1.0") != float_constants.end()) {
                constName = float_constants["-1.0"];
            } else {
                constName = newDoubleConstantName();
                float_constants["-1.0"] = constName;
            }

            ss << "la " << intPlace << ", " << constName << endl
               << "ldc1 " << ftmpPlace << ", 0(" << intPlace << ")" << endl
               << "la " << intPlace << ", " << id << endl
               << "ldc1 " << ctx.place << ", 0(" << intPlace << ")" << endl
               << "add.d " << ctx.place << ", " << ctx.place << ", " << ftmpPlace << endl
               << "sdc1 " << ctx.place << ", 0(" << intPlace << ")" << endl;

            releaseIntTemp(intPlace);
            releaseFloatTemp(ftmpPlace);

            ctx.code = ss.str();
            break;
        }
        default:
            cout << "BUG in the MACHINE" << endl;
            exit(1);
    }
}

/*
 * Code generation for statements
 */

string BlockStatement::generateCode()
{
    stringstream ss;
    list<Statement *>::iterator it = stList.begin();
    
    while (it != stList.end()) {
        Statement *st = *it;
        
        ss << st->generateCode();
        it++;
    }   
    
    return ss.str();
}

string PrintStatement::generateCode() 
{
    stringstream ss;
    ExprContext ctx;
    expr->generateCode(ctx);
    
    switch (ctx.type) {
        case DT_Int: {
            ss << "# Print integer expression '" << expr->toString() << "'" << endl
               << ctx.code
               << "move $a0, " << ctx.place << endl
               << "li $v0, 1" << endl
               << "syscall" << endl;
            break;
        }
        case DT_Double: {
            ss << "# Print float expression '" << expr->toString() << "'" << endl
               << ctx.code
               << "mov.d $f12, " << ctx.place << endl
               << "li $v0, 3" << endl
               << "syscall" << endl;
            break;
        }
    }
    
    ss << "la $a0, _endl" << endl
       << "li $v0, 4" << endl
       << "syscall" << endl;
        
    ctx.releasePlace();
    
    return ss.str();
}

string AssignStatement::generateCode()
{
    if (vars.find(id) == vars.end()) {
        printf("Identifier '%s' has not been declared\n", id.c_str());
        exit(1);
    }
    
    ExprContext ctx;
    stringstream ss;
    string tempPlace;
    
    expr->generateCode(ctx);
    DataType vtype = vars[id];
    
    tempPlace = newIntTemp(); //Temporary variable to load the address
    
    if (vtype == DT_Int) {
        ctx.castToInt();
        
        ss << "# Int assignment for '" << id << " = " << expr->toString() << "'" << endl
           << ctx.code
	   << "la " << tempPlace << ", " << id << endl
	   << "sw " << ctx.place << ", 0(" << tempPlace << ")" << endl;
        
        releaseIntTemp(tempPlace);
        releaseIntTemp(ctx.place);
    }
    else {
        ctx.castToDouble();
        
        ss << "# Float assignment for '" << id << " = " << expr->toString() << "'" << endl
           << ctx.code
	   << "la " << tempPlace << ", " << id << endl
	   << "sdc1 " << ctx.place << ", 0(" << tempPlace << ")" << endl;
        
        releaseIntTemp(tempPlace);
        releaseFloatTemp(ctx.place);
    }
    
    ctx.releasePlace();

    return ss.str();
}

string IfStatement::generateCode()
{
    stringstream ss;
    ExprContext ctx;

    cond->generateCode(ctx);
    ctx.releasePlace();
    
    if (ctx.type != DT_Int) {
        printf("if condition should be int type\n");
        exit(1);
    }
    
    string lbl_false= newLabel();
    string lbl_end = newLabel();
    string trueBlockCode = trueBlock->generateCode();
    string falseBlockCode = falseBlock != NULL? (falseBlock->generateCode()) : "# NO code for else block";
    
    ss << "# if (" << cond->toString() << ") ... " << endl
       << ctx.code 
       << "beq " << ctx.place << ", $zero, " + lbl_false << endl
       << trueBlockCode
       << "j " << lbl_end << endl
       << lbl_false << ":" << endl
       << falseBlockCode
       << lbl_end << ":" << endl;
    
    return ss.str();
}

string WhileStatement::generateCode()
{
    stringstream ss;
    ExprContext ctx;
    
    cond->generateCode(ctx);
    ctx.releasePlace();

    if (ctx.type != DT_Int) {
        printf("while condition should be int type\n");
        exit(1);
    }
    
    string lbl_loop = newLabel();
    string lbl_end = newLabel();
    string blockCode = loopBody->generateCode();
    
    ss << "# while (" << cond->toString() << ") ... " << endl
       << lbl_loop << ":" << endl
       << ctx.code 
       << "beq " << ctx.place << ", $zero, " + lbl_end << endl
       << blockCode
       << "j " << lbl_loop << endl
       << lbl_end << ":" << endl;
    
    return ss.str();
}

string ForStatement::generateCode()
{
    stringstream ss;
    
    return ss.str();
}

string ExprStatement::generateCode()
{
    ExprContext ctx;
    
    expr->generateCode(ctx);
    ctx.releasePlace();
        
    return ctx.code;
}
