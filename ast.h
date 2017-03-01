/*
 * Aayush Saxena : 201201021
 * Kalpit Thakkar : 201201071
 */

/*
 * Grammar for the Decaf Language :
 *
 * Program 					=> HEADER '{' FieldDeclList MethodDeclList '}'
 * MethodDecl 				=> Type ID '(' ParameterDeclList ')' Block
 *							| VOID ID '(' ParameterDeclList ')' Block
 * MethodDeclList 			=> empty
 *							| nonEmptyMethodDeclList
 * nonEmptyMethodDeclList 	=> MethodDecl
 *							| nonEmptyMethodDeclList MethodDecl
 * FieldDeclList			=> empty
 *							| FieldDeclList FieldDecl
 * StatementDeclList		=> empty
 *							| StatementDeclList StatementDecl
 * FieldDecl				=> Type variableList ';'
 * variableList				=> Variable
 *							| variableList ',' Variable
 * Variable					=> ID
 *							| ID '[' IntegerLiteral ']'
 * Type						=> TYPES
 * IntegerLiteral			=> DEC_LITERAL
 *							| HEX_LITERAL
 * StatementDecl			=> Location AssignOp Expr ';'
 *							| MethodCall ';'
 *							| IF '(' Expr ')' Block
 *							| IF '(' Expr ')' Block ELSE Block
 *							| FOR ID AssignOp Expr ',' Expr Block
 *							| RETURN Expr ';'
 *							| BREAK ';'
 *							| CONTINUE ';'
 *							| Block
 * MethodCall				=> ID '(' ExprList ')'
 *							| CALLOUT '(' STRING_LITERAL ',' CalloutArgList ')'
 * ParameterDeclList		=> empty
 							| nonEmptyParameterDeclList
 * nonEmptyParameterDeclList=> ParameterDecl
 *							| nonEmptyParameterDeclList ',' ParameterDecl
 * ParameterDecl			=> Type ID
 *							| Type ID '[' ']'
 * Block					=> '{' FieldDeclList StatementDeclList '}'
 * AssignOp					=> ASSIGN
 *							| PLUSASSIGN
 *							| MINUSASSIGN
 * Location					=> ID
 *							| ID '[' Expr ']'
 * ExprList					=> empty
 *							| nonEmptyExprList
 * nonEmptyExprList			=> Expr
 *							| nonEmptyExprList ',' Expr
 * Expr						=> Location
 *							| MethodCall
 *							| IntegerLiteral
 *							| MINUS Expr %prec UNARY
 *							| BING Expr
 *							| CHAR_LITERAL
 *							| BOOL_LITERAL
 *							| '(' Expr ')'
 *							| BinaryExpr
 * BinaryExpr				=> Expr MULT Expr
 *							| Expr DIV Expr
 *							| Expr PLUS Expr
 *							| Expr MINUS Expr
 *							| Expr MOD Expr
 *							| Expr AND Expr
 *							| Expr OR Expr
 *							| Expr EQ Expr
 *							| Expr NEQ Expr
 *							| Expr LT Expr
 *							| Expr GT Expr
 *							| Expr GTEQ Expr
 *							| Expr LTEQ Expr
 * CalloutArgList			=> CalloutArg
 *							| CalloutArgList ',' CalloutArg
 * CalloutArg				=> Expr
 *							| STRING_LITERAL
 *
 */

#ifndef __AST_H__
#define __AST_H__

#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include <list>
#include "stdllvm.h"			// LLVM Header files necessary for Code Generation
using namespace std;
using namespace llvm;

/* Global Variable Definitions => Used for operator and type ids */
const int _int_ = 1;
const int _bool_ = 2;
const int _void_ = 3;

const int _error = 0;
const int _mult = 4;
const int _div = 8;
const int _plus = 16;
const int _minus = 32;
const int _eq = 64;
const int _neq = 128;
const int _and = 256;
const int _or = 512;
const int _gt = 1024;
const int _lt = 2048;
const int _gteq = 4096;
const int _lteq = 8192;
const int _unaryminus = 16384;
const int _negate = 32768;
const int _mod = 65536;

/* Parent class of the Abstract Syntax Tree */
class ASTNode {
	public:
		virtual Value *accept(class Visitor *) = 0;
};

class ASTExpressionNode : public ASTNode {
	public:
		ASTExpressionNode* left;
		ASTExpressionNode* right;

		ASTExpressionNode(ASTExpressionNode *l, ASTExpressionNode *r) : left(l), right(r) {}
		ASTExpressionNode() : left(NULL), right(NULL) {}
};

class ASTParameterDecl {
	public:
		ASTParameterDecl(int t, string name, bool arr) {
			type_ = t;
			varName_ = name;
			isArray_ = arr;
		}
		const int getType() const {
			return type_;
		}
		string getVarName() {
			return varName_;
		}
		const bool getIfArray() const {
			return isArray_;
		}

	private:
		int type_;
		string varName_;
		bool isArray_;
};

class ASTStatementDeclNode : public ASTNode {
	public:
		ASTStatementDeclNode(const int id) : statementId_(id) {}

		const int getStatementId() const {
			return statementId_;
		}

	private:
		int statementId_;
};

class ASTBlock : public ASTNode {
	public:
		ASTBlock(list<ASTStatementDeclNode *> *s) {
			statementList_ = s;
		}
		list<ASTStatementDeclNode *> *getStatementList() {
			return statementList_;
		}
		Value *accept(Visitor *) override;

	private:
		list<ASTStatementDeclNode *> *statementList_;
};

class ASTMethodDeclNode : public ASTNode {
	public:
		ASTMethodDeclNode(int t, string name, list<ASTParameterDecl *> *p, ASTBlock *b) {
			type_ = t;
			methodName_ = name;
			params_ = p;
			block_ = b;
		}
		const int getType() const {
			return type_;
		}
		string getMethodName() const {
			return methodName_;
		}
		list<ASTParameterDecl *> *getParamList() {
			return params_;
		}
		ASTBlock *getBlock() const {
			return block_;
		}
		Value *accept(Visitor *) override;

	private:
		int type_;
		string methodName_;
		list<ASTParameterDecl *> *params_;
		ASTBlock *block_;
};

class ASTBlockStatementNode : public ASTStatementDeclNode {
	public:
		ASTBlockStatementNode(ASTBlock *b) : ASTStatementDeclNode(1) {
			block_ = b;
		}
		ASTBlock *getBlock() const {
			return block_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTBlock *block_;
};

class ASTLocationNode : public ASTNode {
	public:
		ASTLocationNode(bool isArray) : isArray_(isArray) {}

		bool isArray_;
};

class ASTProgramNode : public ASTNode {
	public:
		ASTProgramNode(list<ASTMethodDeclNode *> *List) {
			methodDeclList_ = List;
		}
		list<ASTMethodDeclNode *> *getMethodDeclList() const {
			return methodDeclList_;
		}
		Value *accept(Visitor *) override;

	private:
		list<ASTMethodDeclNode *> *methodDeclList_;
};

class ASTAssignmentStatementNode : public ASTStatementDeclNode {
	public:
		ASTAssignmentStatementNode(ASTLocationNode *loc, string op, ASTExpressionNode *ex) : ASTStatementDeclNode(1) {
			location_ = loc;
			operator_ = op;
			expr_ = ex;
		}
		ASTLocationNode *getLocation() const {
			return location_;
		}
		ASTExpressionNode *getExpression() const {
			return expr_;
		}
		const string getAssignmentOperator() const {
			return operator_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTLocationNode *location_;
		ASTExpressionNode *expr_;
		string operator_;
};

class ASTMethodCallStatementNode : public ASTStatementDeclNode {
	public:
		ASTMethodCallStatementNode(bool isCallout) : ASTStatementDeclNode(1) {
			isCallout_ = isCallout;
		}

	private:
		bool isCallout_;
};

class ASTSimpleMethodCallNode : public ASTMethodCallStatementNode {
	public:
		ASTSimpleMethodCallNode(string name, list<ASTExpressionNode *> *list) : ASTMethodCallStatementNode(false) {
			methodName_ = name;
			exprList_ = list;
		}
		const string getMethodName() const {
			return methodName_;
		}
		list<ASTExpressionNode *> *getExpressionList() const {
			return exprList_;
		}
		Value *accept(Visitor *) override;

	private:
		string methodName_;
		list<ASTExpressionNode *> *exprList_;
};

class ASTCalloutArg : public ASTNode {};

class ASTCalloutMethodCallNode : public ASTMethodCallStatementNode {
	public:
		ASTCalloutMethodCallNode(string fname, list<ASTCalloutArg *> *List) : ASTMethodCallStatementNode(true) {
			func_ = fname;
			argList_ = List;
		}
		const string getFuncName() const {
			return func_;
		}
		list<ASTCalloutArg *> *getArgumentList() const {
			return argList_;
		}
		Value *accept(Visitor *) override;

	private:
		string func_;
		list<ASTCalloutArg *> *argList_;
};

class ASTIfStatementDeclNode : public ASTStatementDeclNode {
	public:
		ASTIfStatementDeclNode(ASTExpressionNode *ifExp, ASTBlock *ifBlock, ASTBlock *elseBlock) : ASTStatementDeclNode(1) {
			ifExpression_ = ifExp;
			ifBlock_ = ifBlock;
			elseBlock_ = elseBlock;
		}
		ASTExpressionNode *getIfExpression() const {
			return ifExpression_;
		}
		ASTBlock *getIfBlock() const {
			return ifBlock_;
		}
		ASTBlock *getElseBlock() const {
			return elseBlock_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTExpressionNode *ifExpression_;
		ASTBlock *ifBlock_;
		ASTBlock *elseBlock_;
};

class ASTForStatementDeclNode : public ASTStatementDeclNode {
	public:
		ASTForStatementDeclNode(string it, ASTExpressionNode *init, ASTExpressionNode *end, ASTBlock *b) : ASTStatementDeclNode(1) {
			iterName_ = it;
			initExpression_ = init;
			finalExpression_ = end;
			block_ = b;
		}
		const string getIterVarName() const {
			return iterName_;
		}
		ASTExpressionNode *getInitExpression() const {
			return initExpression_;
		}
		ASTExpressionNode *getFinalExpression() const {
			return finalExpression_;
		}
		ASTBlock *getForBody() {
			return block_;
		}
		Value *accept(Visitor *) override;

	private:
		string iterName_;
		ASTExpressionNode *initExpression_;
		ASTExpressionNode *finalExpression_;
		ASTBlock *block_;
};

class ASTReturnStatementNode : public ASTStatementDeclNode {
	public:
		ASTReturnStatementNode(ASTExpressionNode *ex) : ASTStatementDeclNode(1) {
			returnExpr_ = ex;
		}
		ASTExpressionNode *getReturnExpression() const {
			return returnExpr_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTExpressionNode *returnExpr_;
};

class ASTBreakStatementNode : public ASTStatementDeclNode {
	public:
		ASTBreakStatementNode() : ASTStatementDeclNode(1) {}
		Value *accept(Visitor *) override;
};

class ASTContinueStatementNode : public ASTStatementDeclNode {
	public:
		ASTContinueStatementNode() : ASTStatementDeclNode(1) {}
		Value *accept(Visitor *) override;
};

class ASTVarLocationNode : public ASTLocationNode {
	public:
		ASTVarLocationNode(string id) : var_(id), ASTLocationNode(false) {}
		const string getVar() const {
			return var_;
		}
		Value *accept(Visitor *) override;

	private:
		string var_;
};

class ASTArrayLocationNode : public ASTLocationNode {
	public:
		ASTArrayLocationNode(string id, ASTExpressionNode *ex) : var_(id), expr_(ex), ASTLocationNode(true) {}
		const string getVar() const {
			return var_;
		}
		ASTExpressionNode *getExpression() const {
			return expr_;
		}
		Value *accept(Visitor *) override;

	private:
		string var_;
		ASTExpressionNode *expr_;
};

class ASTExpressionCalloutArg : public ASTCalloutArg {
	public:
		ASTExpressionCalloutArg(ASTExpressionNode *ex) {
			expr_ = ex;
		}
		ASTExpressionNode *getExpression() const {
			return expr_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTExpressionNode *expr_;
};

class ASTStringCalloutArg : public ASTCalloutArg {
	public:
		ASTStringCalloutArg(string arg) {
			arg_ = arg;
		}
		string getString() const {
			return arg_;
		}
		Value *accept(Visitor *) override;

	private:
		string arg_;
};

class ASTMethodCallExpressionNode : public ASTExpressionNode {
	public:
		ASTMethodCallExpressionNode(ASTMethodCallStatementNode *method) : ASTExpressionNode() {
			methodNode_ = method;
		}
		ASTMethodCallStatementNode *getMethodCallStatement() {
			return methodNode_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTMethodCallStatementNode *methodNode_;
};

class ASTIntegerLiteralExpressionNode : public ASTExpressionNode {
	public:
		ASTIntegerLiteralExpressionNode(int val) : ASTExpressionNode() {
			value_ = val;
		}
		int getValue() const {
			return value_;
		}
		Value *accept(Visitor *) override;

	private:
		int value_;
};

class ASTCharLiteralExpressionNode : public ASTExpressionNode {
	public:
		ASTCharLiteralExpressionNode(char val) : ASTExpressionNode() {
			value_ = val;
		}
		char getValue() const {
			return value_;
		}
		Value *accept(Visitor *) override;

	private:
		char value_;
};

class ASTBoolLiteralExpressionNode : public ASTExpressionNode {
	public:
		ASTBoolLiteralExpressionNode(string val) : ASTExpressionNode() {
			value_ = val;
		}
		string getValue() const {
			return value_;
		}
		Value *accept(Visitor *) override;

	private:
		string value_;
};

class ASTLocationExpressionNode : public ASTExpressionNode {
	public:
		ASTLocationExpressionNode(ASTLocationNode *loc) : ASTExpressionNode() {
			location_ = loc;
		}
		ASTLocationNode *getLocation() const {
			return location_;
		}
		Value *accept(Visitor *) override;

	private:
		ASTLocationNode *location_;
};

class ASTBinaryExpressionNode : public ASTExpressionNode {
	public:
		ASTBinaryExpressionNode(ASTExpressionNode *L, ASTExpressionNode *R, string op) : ASTExpressionNode(L, R) {
			((!op.compare("*")) ? operator_ = _mult :
			((!op.compare("/")) ? operator_ = _div  :
			((!op.compare("+")) ? operator_ = _plus :
			((!op.compare("-")) ? operator_ = _minus :
			((!op.compare("%")) ? operator_ = _mod :
			((!op.compare("==")) ? operator_ = _eq :
			((!op.compare("!=")) ? operator_ = _neq :
			((!op.compare("&&")) ? operator_ = _and :
			((!op.compare("||")) ? operator_ = _or :
			((!op.compare(">")) ? operator_ = _gt :
			((!op.compare("<")) ? operator_ = _lt :
			((!op.compare(">=")) ? operator_ = _gteq :
			((!op.compare("<=")) ? operator_ = _lteq : operator_ = _error)))))))))))));
		}
		const int getOperatorId() const {
			return operator_;
		}
		Value *accept(Visitor *) override;

	private:
		int operator_;
};

class ASTUnaryExpressionNode : public ASTExpressionNode {
	public:
		ASTUnaryExpressionNode(ASTExpressionNode *R, string op) : ASTExpressionNode(NULL, R) {
			((!op.compare("-")) ? operator_ = _unaryminus :
			((!op.compare("!")) ? operator_ = _negate : operator_ = _error));
		}
		const int getOperatorId() const {
			return operator_;
		}
		Value *accept(Visitor *) override;

	private:
		int operator_;
};

/* Visitor Class for performing heterogeneous actions on a ASTNode */
class Visitor {
	public:
		virtual Value *visit(ASTAssignmentStatementNode *) = 0;
		virtual Value *visit(ASTVarLocationNode *) = 0;
		virtual Value *visit(ASTArrayLocationNode *) = 0;
		virtual Value *visit(ASTProgramNode *) = 0;
		virtual Value *visit(ASTBlock *) = 0;
		virtual Value *visit(ASTMethodDeclNode *) = 0;
		virtual Value *visit(ASTSimpleMethodCallNode *) = 0;
		virtual Value *visit(ASTCalloutMethodCallNode *) = 0;
		virtual Value *visit(ASTIfStatementDeclNode *) = 0;
		virtual Value *visit(ASTForStatementDeclNode *) = 0;
		virtual Value *visit(ASTReturnStatementNode *) = 0;
		virtual Value *visit(ASTBreakStatementNode *) = 0;
		virtual Value *visit(ASTContinueStatementNode *) = 0;
		virtual Value *visit(ASTBlockStatementNode *) = 0;
		virtual Value *visit(ASTExpressionCalloutArg *) = 0;
		virtual Value *visit(ASTStringCalloutArg *) = 0;
		virtual Value *visit(ASTMethodCallExpressionNode *) = 0;
		virtual Value *visit(ASTIntegerLiteralExpressionNode *) = 0;
		virtual Value *visit(ASTBoolLiteralExpressionNode *) = 0;
		virtual Value *visit(ASTCharLiteralExpressionNode *) = 0;
		virtual Value *visit(ASTLocationExpressionNode *) = 0;
		virtual Value *visit(ASTBinaryExpressionNode *) = 0;
		virtual Value *visit(ASTUnaryExpressionNode *) = 0;
};

class EvaluateVisitor : public Visitor {
	public:
		Value *visit(ASTAssignmentStatementNode *node);
		Value *visit(ASTVarLocationNode *node);
		Value *visit(ASTArrayLocationNode *node);
		Value *visit(ASTProgramNode *node);
		Value *visit(ASTBlock *node);
		Value *visit(ASTMethodDeclNode *node);
		Value *visit(ASTSimpleMethodCallNode *node);
		Value *visit(ASTCalloutMethodCallNode *node);
		Value *visit(ASTIfStatementDeclNode *node);
		Value *visit(ASTForStatementDeclNode *node);
		Value *visit(ASTReturnStatementNode *node);
		Value *visit(ASTBreakStatementNode *node);
		Value *visit(ASTContinueStatementNode *node);
		Value *visit(ASTBlockStatementNode *node);
		Value *visit(ASTExpressionCalloutArg *node);
		Value *visit(ASTStringCalloutArg *node);
		Value *visit(ASTMethodCallExpressionNode *node);
		Value *visit(ASTIntegerLiteralExpressionNode *node);
		Value *visit(ASTBoolLiteralExpressionNode *node);
		Value *visit(ASTCharLiteralExpressionNode *node) {}
		Value *visit(ASTLocationExpressionNode *node);
		Value *visit(ASTBinaryExpressionNode *node);
		Value *visit(ASTUnaryExpressionNode *node);
};

/* Variable encountered in the FieldDecl rule is stored as a Symbol */
class Symbol {
	public:
		Symbol(string id) : id_(id), literal_(NULL) {}
		Symbol(string id, ASTIntegerLiteralExpressionNode* lit) : id_(id), literal_(lit) {}

		string id_;
		ASTIntegerLiteralExpressionNode *literal_;
};

void annotateSymbolTable(int datatype, list<Symbol *> *variableList);
void BuildIR(ASTProgramNode *root);

#endif
