#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include <stack>
#include <list>
#include "ast.h"
#include "stdllvm.h"
using namespace std;
using namespace llvm;

/* Loop Structure for Break and Continue Statements */
typedef struct Loop {
	BasicBlock *entryBB;
	BasicBlock *afterBB;
	PHINode *var_;
	ASTExpressionNode *endExp;
}loop;

Module *DecafToLLVM = new Module("DecafToLLVM", getGlobalContext());
IRBuilder<> *Builder = new IRBuilder<>(getGlobalContext());
static map<string, Value *> symTable;
static stack<BasicBlock *> Blocks;
static stack<loop *> loops;
static bool declStarted = false;
string var;

/* The Driver function to start building the IR */
void BuildIR(ASTProgramNode *root) {
	EvaluateVisitor v;
	root->accept(&v);
}

void Error(const char *S) {
	cout << S << endl;
}
Value *ErrorV(const char *Str) {
  Error(Str);
  return nullptr;
}

/*
 * When the location is just a variable of type int or boolean,
 * we return it's AllocaInst. If it's an array, we create a Global
 * Element Pointer for required index, that is used to load the
 * contents at that index.
 */
Value *EvaluateVisitor::visit(ASTVarLocationNode *node) {
	var = node->getVar();
	return symTable[var];
}

Value *EvaluateVisitor::visit(ASTArrayLocationNode *node) {
	var = node->getVar();
	Value *ret = symTable[var];
	Value *size = node->getExpression()->accept(this);
	if(size->getType()->isPointerTy()) {
		size = Builder->CreateLoad(size, "tmp");
	}
	vector<Value*> v;
	v.push_back(Builder->getInt64(0));
	v.push_back(Builder->CreateSExt(size, Builder->getInt64Ty(), "zext"));
	ArrayRef<Value *> a = ArrayRef<Value *>(v);
	return Builder->CreateInBoundsGEP(ret, v, "getptr");
}

/*
 * Simple assignment statements. Supports '=', '+=' and '-='.
 */
Value *EvaluateVisitor::visit(ASTAssignmentStatementNode *node) {
	Value *ptr = node->getLocation()->accept(this);
	Value *val = node->getExpression()->accept(this);
	string op = node->getAssignmentOperator();
	Value *v;
	if(val->getType()->isPointerTy())
		val = Builder->CreateLoad(val, "tmp");
	switch(op[0]) {
		case '=':
			return Builder->CreateStore(val, ptr, false);
		case '+':
			v = Builder->CreateLoad(ptr, "ptr");
			val = Builder->CreateAdd(v, val, "ADD");
			return Builder->CreateStore(val, ptr, false);
		case '-':
			v = Builder->CreateLoad(ptr, "ptr");
			val = Builder->CreateSub(v, val, "ADD");
			return Builder->CreateStore(val, ptr, false);
	}
}

Value *EvaluateVisitor::visit(ASTIntegerLiteralExpressionNode *node) {
	return Builder->getInt32(node->getValue());
}
Value *EvaluateVisitor::visit(ASTBoolLiteralExpressionNode *node) {
	string val = node->getValue();
	int v = 0;
	if(!(val.compare("true"))) {
		v = 1;
	}
	return Builder->getInt1(v);
}

Value *EvaluateVisitor::visit(ASTLocationExpressionNode *node) {
	ASTLocationNode *loc = node->getLocation();
	Value *v = loc->accept(this);
	return v;
}

/*
 * Unary and Binary expressions are handled here.
 */
Value *EvaluateVisitor::visit(ASTUnaryExpressionNode *node) {
	Value *R = node->right->accept(this);
	if(!R) {
		return nullptr;
	}
	if(R->getType()->isPointerTy()) {
		R = Builder->CreateLoad(R, "tmp");
	}
	int op = node->getOperatorId();
	switch(op) {
		case _unaryminus:
			return Builder->CreateNeg(R, "NEG");
		case _negate:
			return Builder->CreateNot(R, "NOT");
		default:
			return ErrorV("Invalid Unary operator");
	}
}

Value *EvaluateVisitor::visit(ASTBinaryExpressionNode *node) {
	Value *L = node->left->accept(this);
	Value *R = node->right->accept(this);
	if (!L || !R) {
		return nullptr;
	}
	if(L->getType()->isPointerTy()) {
		L = Builder->CreateLoad(L, "tmp");
	}
	if(R->getType()->isPointerTy()) {
		R = Builder->CreateLoad(R, "tmp");
	}
	int op = node->getOperatorId();
	switch(op) {
		case _plus:
			return Builder->CreateAdd(L, R, "ADD");
		case _minus:
			return Builder->CreateSub(L, R, "SUB");
		case _mult:
			return Builder->CreateMul(L, R, "MUL");
		case _div:
			return Builder->CreateUDiv(L, R, "DIV");
		case _mod:
			return Builder->CreateURem(L, R, "MOD");
		case _and:
			return Builder->CreateAnd(L, R, "AND");
		case _or:
			return Builder->CreateOr(L, R, "OR");
		case _eq:
			return Builder->CreateICmpEQ(L, R, "EQ");
		case _neq:
			return Builder->CreateICmpNE(L, R, "NEQ");
		case _lt:
			return Builder->CreateICmpSLT(L, R, "LT");
		case _gt:
			return Builder->CreateICmpSGT(L, R, "GT");
		case _lteq:
			return Builder->CreateICmpSLE(L, R, "LTEQ");
		case _gteq:
			return Builder->CreateICmpSGE(L, R, "GTEQ");
		default:
			return ErrorV("Invalid Binary Operator");
	}
}

/*
 * Function to get the LLVM Datatype from the decafType that we
 * give as input.
 */
Type *getLLVMType(int decafTy) {
	switch(decafTy) {
		case _int_:
			return Builder->getInt32Ty();
		case _bool_:
			return Builder->getInt1Ty();
		default:
			runtime_error("Unknown Datatype");
	}
}

Value *EvaluateVisitor::visit(ASTMethodCallExpressionNode *node) {
	ASTMethodCallStatementNode *meth = node->getMethodCallStatement();
	return meth->accept(this);
}

/*
 * In the break statements, we get the loop that is running
 * right now, from the loop stack. We then insert a break
 * instruction at that point that points to the Basic block
 * just after the loop. We then skip writing the instructions
 * following the break statement in the present BB. Hence, return nullptr
 * to prevent writing the following instructions.
 */
Value *EvaluateVisitor::visit(ASTBreakStatementNode *node) {
	loop *thisLoop = loops.top();
	BasicBlock *BB = thisLoop->afterBB;
	Builder->CreateBr(BB);
	return nullptr;
}

/*
 * In continue statements, we get the present loop's entry and exit blocks.
 * We then write the loop's iterator increment and check condition instructions
 * in the present basic block and then return a nullptr so that any further
 * instructions in the present BB are not written in the bitcode.
 */
Value *EvaluateVisitor::visit(ASTContinueStatementNode *node) {
	loop* thisLoop = loops.top();
	BasicBlock *inBB = thisLoop->entryBB;
	BasicBlock *outBB = thisLoop->afterBB;
	BasicBlock *cond = Builder->GetInsertBlock();
	PHINode *v = thisLoop->var_;
	Value *out = Builder->CreateAdd(v, Builder->getInt32(1), "ADD");
	ASTExpressionNode *end = thisLoop->endExp;
	Value *check = end->accept(this);
	check = Builder->CreateICmpEQ(check, Builder->getInt1(1), "loopcond");
	Builder->CreateCondBr(check, inBB, outBB);

	v->addIncoming(out, cond);
	return nullptr;
}

/*
 * For loop. We have two sections for any for loop. The Loop Basic
 * Block and the Basic Block after the loop. We write the PHINode for
 * the loop variable and then check for loop body to see how many inputs
 * are there to put in the PHINode. After this is done, we set the
 * instructions insert point as the after loop Basic Block.
 */
Value *EvaluateVisitor::visit(ASTForStatementDeclNode *node) {
	ASTExpressionNode *start = node->getInitExpression();
	ASTExpressionNode *end = node->getFinalExpression();
	string it = node->getIterVarName();
	ASTBlock *body = node->getForBody();

	Value *init = start->accept(this);
	Function *F = Builder->GetInsertBlock()->getParent();

	BasicBlock *LoopBB = BasicBlock::Create(getGlobalContext(), "loop", F);
	BasicBlock *AfterBB = BasicBlock::Create(getGlobalContext(), "afterloop", F);

	BasicBlock *PreHeaderBB = Builder->GetInsertBlock();
	Builder->CreateBr(LoopBB);

	Builder->SetInsertPoint(LoopBB);
	PHINode *var = Builder->CreatePHI(Type::getInt32Ty(getGlobalContext()), 2, it.c_str());

	loop *thisLoop = (loop *)malloc(sizeof(loop));
	thisLoop->entryBB = LoopBB;
	thisLoop->afterBB = AfterBB;
	thisLoop->var_ = var;
	thisLoop->endExp = end;
	loops.push(thisLoop);

	var->addIncoming(init, PreHeaderBB);

	symTable[it] = var;

	Value *bodyVal = body->accept(this);
	if(bodyVal != nullptr) {
		Value *NextVar = Builder->CreateAdd(var, Builder->getInt32(1), "ADD");

		Value *endVal = end->accept(this);
		endVal = Builder->CreateICmpEQ(endVal, Builder->getInt1(1), "loopcond");
		BasicBlock *LoopEndBB = Builder->GetInsertBlock();

		Builder->CreateCondBr(endVal, LoopBB, AfterBB);

		var->addIncoming(NextVar, LoopEndBB);
	}
	Builder->SetInsertPoint(AfterBB);
	loops.pop();

	return Builder->getInt32(0);
}

/*
 * If else statement. This statement has three parts. The IfBlock,
 * Else Block and the IfCont Block. IfCont is required only if there
 * are any instructions after the if and else statements are completed,
 * in the present basic block. If any of the IF or ELSE have break / continue
 * statements, it will be handled by the respective code for the statements.
 * One special case is when there is no else part.
 */
Value *EvaluateVisitor::visit(ASTIfStatementDeclNode *node) {
	ASTExpressionNode *ifExp = node->getIfExpression();
	ASTBlock *ifBlock = node->getIfBlock();
	ASTBlock *elseBlock = node->getElseBlock();
	Value *v = ifExp->accept(this);
	Value *ifVal = Builder->getInt32(1);
	Value *elseVal = Builder->getInt32(1);
	if(!v) {
		return nullptr;
	}
	if(v->getType()->isIntegerTy()) {
		if(v->getType()->isIntegerTy(32)) {
			v = Builder->CreateICmpEQ(v, Builder->getInt32(1), "cmp");
		}
		else {
			v = Builder->CreateICmpEQ(v, Builder->getInt1(1), "cmp");
		}
		Function *F = Builder->GetInsertBlock()->getParent();
		BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", F);
		if(elseBlock) {
			 BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
			 BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
			 Builder->CreateCondBr(v, ThenBB, ElseBB);
			 Builder->SetInsertPoint(ThenBB);

	 		ifVal = ifBlock->accept(this);
	 		if(ifVal) {
	 			Builder->CreateBr(MergeBB);
	 		}
	 		ThenBB = Builder->GetInsertBlock();

			F->getBasicBlockList().push_back(ElseBB);
			Builder->SetInsertPoint(ElseBB);

			elseVal = elseBlock->accept(this);
			if(elseVal) {
				Builder->CreateBr(MergeBB);
			}
			ElseBB = Builder->GetInsertBlock();

			if(!ifVal && !elseVal) {
				return Builder->getInt32(0);
			}
			else {
				F->getBasicBlockList().push_back(MergeBB);
				Builder->SetInsertPoint(MergeBB);
				return Builder->getInt32(0);
			}
		}
		else {
			BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
			Builder->CreateCondBr(v, ThenBB, MergeBB);
			Builder->SetInsertPoint(ThenBB);

			ifVal = ifBlock->accept(this);
			if(ifVal) {
				Builder->CreateBr(MergeBB);
			}
			ThenBB = Builder->GetInsertBlock();

			F->getBasicBlockList().push_back(MergeBB);
			Builder->SetInsertPoint(MergeBB);

			return Builder->getInt32(0);
		}
	}
}

/*
 * Return Statements are also such control flow statements that
 * once encountered, there's no need to write the instructions in the
 * present Basic Block, hence returning a nullptr.
 */
Value *EvaluateVisitor::visit(ASTReturnStatementNode *node) {
	ASTExpressionNode *expr = node->getReturnExpression();
	Value *v = expr->accept(this);
	if(v->getType()->isPointerTy()) {
		v = Builder->CreateLoad(v, "tmp");
	}
	Builder->CreateRet(v);
	return nullptr;
}

/*
 * Different method call statements. Simple Method Call statements
 * are just like calling a user-defined function in C. callout is
 * a special function Structure, used to call "printf" from C's
 * standard library.
 */
Value *EvaluateVisitor::visit(ASTSimpleMethodCallNode *node) {
	string methName = node->getMethodName();
	list<ASTExpressionNode *> *exprList = node->getExpressionList();
	list<ASTExpressionNode *>::iterator it;
	vector<Value *> args;

	for(it = exprList->begin(); it != exprList->end(); it++) {
		Value *v = (*it)->accept(this);
		if(v->getType()->isPointerTy())
			v = Builder->CreateLoad(v, "tmp");
		args.push_back(v);
	}
	Function *callMe = DecafToLLVM->getFunction(methName);
	return Builder->CreateCall(callMe, args, "calltmp");
}

Value *EvaluateVisitor::visit(ASTCalloutMethodCallNode *node) {
	string funcName = node->getFuncName();
	list<ASTCalloutArg *> *args = node->getArgumentList();
	list<ASTCalloutArg *>::iterator it;
	vector<Value *> argsV;
	Value *fmt;
	for(it = args->begin(); it != args->end(); it++) {
		Value *v = (*it)->accept(this);
		if(it != args->begin()) {
			if(v->getType()->isPointerTy()) {
				v = Builder->CreateLoad(v, "loadarg");
			}
		}
		argsV.push_back(v);
	}
	string func;
	for(int i = 1; i < funcName.length() - 1; i++) {
		func.push_back(funcName[i]);
	}
	vector<Type *> argTypes;
	argTypes.push_back(Type::getInt8PtrTy(getGlobalContext()));
	FunctionType *FT = FunctionType::get(Builder->getInt32Ty(), argTypes, true);
	Constant *printFunc = DecafToLLVM->getOrInsertFunction(func, FT);

	return Builder->CreateCall(printFunc, argsV, "print");
}

Value *EvaluateVisitor::visit(ASTBlockStatementNode *node) {
	ASTBlock *block = node->getBlock();
	return block->accept(this);
}

/*
 * callout arguments. String arguments are the format strings,
 * which are used for printf calls. Eg : "%d%d\n". Expression
 * arguments are variable's values. Actually, locations are provided
 * which are loaded and then provided as arguments.
 */
Value *EvaluateVisitor::visit(ASTStringCalloutArg *node) {
	string out;
	string in = node->getString();
	for(int i = 1; i < in.length() - 1; i++) {
		if(in[i] == '\\' && in[i+1] == 'n') {
			out.push_back('\n');
			i++;
		}
		else {
			out.push_back(in[i]);
		}
	}
	Value *formatStr = Builder->CreateGlobalStringPtr(out.c_str(), "fmt");
	return formatStr;
}

Value *EvaluateVisitor::visit(ASTExpressionCalloutArg *node) {
	Value *v = node->getExpression()->accept(this);
	return v;
}

Value *EvaluateVisitor::visit(ASTBlock *node) {
	list<ASTStatementDeclNode *> *s = node->getStatementList();
	list<ASTStatementDeclNode *>::iterator it;

	Value *v;
	for(it = s->begin(); it != s->end(); it++) {
		v = (*it)->accept(this);
		if(!v) {					// If the return value is nullptr, then
			break;					// do not process further statements.
		}
	}
	return v;
}

static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, Type *ty,
                                          const string &VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(ty, 0,
                           VarName.c_str());
}

/*
 * Function to create method declarations. Simple Enough.
 */
Value *EvaluateVisitor::visit(ASTMethodDeclNode *node) {
	declStarted = true;
	string name = node->getMethodName();
	int val = node->getType();
	Type *retType;
	if(val == 1) {
		retType = Builder->getInt32Ty();
	}
	else if (val == 2) {
		retType = Builder->getInt1Ty();
	}
	else {
		retType = Builder->getVoidTy();
	}
	vector<Type *> paramTypes;
	vector<string> paramNames;
	list<ASTParameterDecl *> *params = node->getParamList();
	list<ASTParameterDecl *>::iterator it;
	for(it = params->begin(); it != params->end(); it++) {
		int t = (*it)->getType();
		if(t == 1) {
			paramTypes.push_back(Builder->getInt32Ty());
		}
		else {
			paramTypes.push_back(Builder->getInt1Ty());
		}
		paramNames.push_back((*it)->getVarName());
	}
	Function *F = Function::Create(FunctionType::get(retType, paramTypes, false),
								Function::ExternalLinkage, name, DecafToLLVM);
	symTable.insert(make_pair(name, F));
	BasicBlock *BBlock = BasicBlock::Create(getGlobalContext(), name+"_1", F);
	Builder->SetInsertPoint(BBlock);

	Function::arg_iterator args = F->arg_begin();
	for(int i = 0; i < paramNames.size(); i++) {
		AllocaInst *alloca = CreateEntryBlockAlloca(F, paramTypes[i], paramNames[i]);
		Builder->CreateStore(args, alloca);

		symTable[paramNames[i]] = alloca;
		Value *x = args++;
		x->setName(paramNames[i]);
	}
	ASTBlock *block;
	block = node->getBlock();
	block->accept(this);
	return F;
}

Value *EvaluateVisitor::visit(ASTProgramNode *node) {
	list<ASTMethodDeclNode *> *s = node->getMethodDeclList();
	list<ASTMethodDeclNode *>::iterator iter;
	for(iter = s->begin(); iter != s->end(); iter++) {
		Value *v = (*iter)->accept(this);
	}
	return nullptr;
}

AllocaInst *defineVariable(Type *llvmTy, Value *v, string id) {
	Value *arraySize = 0;
	int align = 4;
	if(v) {
		arraySize = v;
		align = 16;
	}
	AllocaInst *Alloca = Builder->CreateAlloca(llvmTy, arraySize, id.c_str());
	Alloca->setAlignment(align);
	return Alloca;
}

/*
 * Function to declare and store the global variables / allocas in the
 * symbol Table.
 */
void annotateSymbolTable(int datatype, list<Symbol*> *variableList) {
	if(!declStarted) {
		list<Symbol*>::iterator iter;
		EvaluateVisitor v_;
		bool isArray = false;
		for(iter = variableList->begin(); iter != variableList->end(); iter++) {
			Symbol *sym = *iter;
			Value *v = nullptr;
			if(sym->literal_ != 0) {
				v = sym->literal_->accept(&v_);
				isArray = true;
			}
			ConstantInt *c;
			if(v) {
				c = dyn_cast<ConstantInt>(v);
			}
			Type *ty = getLLVMType(datatype);
			GlobalVariable *var;
			if(!isArray) {
				var = new GlobalVariable(*DecafToLLVM, ty, false, GlobalValue::CommonLinkage,
														0, sym->id_.c_str());
				var->setAlignment(4);
				if(ty->isIntegerTy(32)) {
					var->setInitializer(Builder->getInt32(0));
				}
				else {
					var->setInitializer(Builder->getInt1(0));
				}
			}
			else {
				ArrayType* ArrayTy_0 = ArrayType::get(ty, c->getSExtValue());
				PointerType* PointerTy_1 = PointerType::get(ArrayTy_0, 0);
				var = new GlobalVariable(*DecafToLLVM, ArrayTy_0, false, GlobalValue::CommonLinkage,
														0, sym->id_.c_str());
				var->setAlignment(16);
				ConstantAggregateZero* const_array_2 = ConstantAggregateZero::get(ArrayTy_0);
				var->setInitializer(const_array_2);
			}
			symTable.insert(make_pair(sym->id_, var));
		}
	}
	else {
		list<Symbol*>::iterator iter;
		EvaluateVisitor v_;
		for(iter = variableList->begin(); iter != variableList->end(); iter++) {
			Symbol *sym = *iter;
			Value *v = nullptr;
			if(sym->literal_ != 0) {
				v = sym->literal_->accept(&v_);
			}
			Type *ty = getLLVMType(datatype);
			AllocaInst *alloca = defineVariable(ty, v, sym->id_);
			symTable.insert(make_pair(sym->id_, alloca));
		}
	}
}

/* Accept Functions (Dispatch #1) for the Visitor Design Pattern */
Value * ASTProgramNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTAssignmentStatementNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTVarLocationNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTArrayLocationNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTExpressionCalloutArg::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTStringCalloutArg::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTIntegerLiteralExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTBoolLiteralExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTCharLiteralExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTLocationExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTBinaryExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTUnaryExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTMethodDeclNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTIfStatementDeclNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTContinueStatementNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTBreakStatementNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTReturnStatementNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTForStatementDeclNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTSimpleMethodCallNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTBlockStatementNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTCalloutMethodCallNode::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTBlock::accept(Visitor *v) {
	return v->visit(this);
}
Value * ASTMethodCallExpressionNode::accept(Visitor *v) {
	return v->visit(this);
}
