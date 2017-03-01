%{
#include "head.h"           // Include some C++ headers
#include "ast.h"            // AST's interfaces
int yyerror(string s);
int yylex(void);
using namespace std;

ASTProgramNode *root;
%}

%union
{
    int     intVal;
    string  *strVal;

    ASTProgramNode* prog;
    ASTStatementDeclNode* stmt;
    ASTMethodCallStatementNode *meth1;
    ASTMethodDeclNode *method;
    ASTBlock *bl;
    ASTParameterDecl *param;
    list<ASTParameterDecl *> *paramList;
    list<ASTMethodDeclNode *> *methList;
    list<ASTStatementDeclNode*> *stmtlist;
    Symbol* sym;
    list<Symbol*> *symlist;
    ASTIntegerLiteralExpressionNode* intLit;
    ASTLocationNode* loc;
    ASTExpressionNode* expr;
    list<ASTExpressionNode *> *exprList;
    ASTBinaryExpressionNode* binexpr;
    list<ASTCalloutArg *> *carglist;
    ASTCalloutArg* carg;
}


%start              Program

%token              CHAR_LITERAL STRING_LITERAL TYPES BOOL_LITERAL
%token              ID DEC_LITERAL HEX_LITERAL INTEGER BOOLEAN
%token              HEADER CALLOUT VOID IF ELSE CONTINUE BREAK RETURN FOR
%token              AND OR MINUS PLUS MULT DIV MOD
%token              ASSIGN PLUSASSIGN MINUSASSIGN
%token              EQ GT LT GTEQ LTEQ NEQ BING UNARY

%left               EQ NEQ GT LT GTEQ LTEQ
%left               AND OR
%left               MINUS PLUS
%left               MULT DIV MOD
%left               BING
%left               UNARY

%type               <strVal>                            ID
%type               <strVal>                            TYPES
%type               <intVal>                            DEC_LITERAL
%type               <intVal>                            HEX_LITERAL
%type               <strVal>                            STRING_LITERAL
%type               <strVal>                            ASSIGN
%type               <strVal>                            PLUSASSIGN
%type               <strVal>                            MINUSASSIGN
%type               <strVal>                            CHAR_LITERAL
%type               <strVal>                            BOOL_LITERAL
%type               <strVal>                            MINUS
%type               <strVal>                            BING
%type               <strVal>                            MULT
%type               <strVal>                            DIV
%type               <strVal>                            PLUS
%type               <strVal>                            MOD
%type               <strVal>                            AND
%type               <strVal>                            OR
%type               <strVal>                            EQ
%type               <strVal>                            NEQ
%type               <strVal>                            GT
%type               <strVal>                            LT
%type               <strVal>                            GTEQ
%type               <strVal>                            LTEQ

%type               <prog>                              Program
%type               <stmt>                              StatementDecl
%type               <stmtlist>                          StatementDeclList
%type               <symlist>                           VariableList
%type               <sym>                               Variable
%type               <intVal>                            Type
%type               <intLit>                            IntegerLiteral
%type               <strVal>                            AssignOp
%type               <loc>                               Location
%type               <expr>                              Expr
%type               <binexpr>                           BinaryExpr
%type               <carglist>                          CalloutArgList
%type               <carg>                              CalloutArg
%type               <method>                            MethodDecl
%type               <methList>                          MethodDeclList
%type               <methList>                          nonEmptyMethodDeclList
%type               <meth1>                             MethodCall
%type               <bl>                                Block
%type               <param>                             ParameterDecl
%type               <paramList>                         ParameterDeclList
%type               <paramList>                         nonEmptyParameterDeclList
%type               <exprList>                          ExprList
%type               <exprList>                          nonEmptyExprList

%%

/* Grammar for the Decaf Programming Language */

Program:            HEADER '{' FieldDeclList MethodDeclList '}' { $$ = new ASTProgramNode($4); root = $$; BuildIR(root); }
                    ;

MethodDecl:         Type ID '(' ParameterDeclList ')' Block { $$ = new ASTMethodDeclNode($1, *($2), $4, $6); }
                    | VOID ID '(' ParameterDeclList ')' Block { $$ = new ASTMethodDeclNode(3, *($2), $4, $6); }
                    ;

MethodDeclList:     /* empty */ { $$ = new list<ASTMethodDeclNode *>(); }
                    | nonEmptyMethodDeclList { $$ = $1; }
                    ;

nonEmptyMethodDeclList: MethodDecl { $$ = new list<ASTMethodDeclNode *>(); $$->push_back($1); }
                        | nonEmptyMethodDeclList MethodDecl { $$ = $1; $$->push_back($2); }
                        ;

FieldDeclList:      /* empty */
                    | FieldDeclList FieldDecl
                    ;

StatementDeclList:  /* empty */ { $$ = new list<ASTStatementDeclNode *>(); }
                    | StatementDeclList StatementDecl { $$ = $1; $$->push_back($2); }
                    ;

FieldDecl:          Type VariableList ';' { annotateSymbolTable($1, $2); }
                    ;

VariableList:       Variable { $$ = new list<Symbol *>(); $$->push_back($1); }
                    | VariableList ',' Variable { $$ = $1; $$->push_back($3); }
                    ;

Variable:           ID { $$ = new Symbol(*($1)); }
                    | ID '[' IntegerLiteral ']' { $$ = new Symbol(*($1), $3); }
                    ;

Type:               TYPES { if(!($1->compare("int"))) $$ = _int_; else $$ = _bool_; }
                    ;

IntegerLiteral:     DEC_LITERAL { $$ = new ASTIntegerLiteralExpressionNode($1); }
                    | HEX_LITERAL { $$ = new ASTIntegerLiteralExpressionNode($1); }
                    ;

StatementDecl:      Location AssignOp Expr ';' { $$ = new ASTAssignmentStatementNode($1, *($2), $3); }
                    | MethodCall ';' { $$ = $1; }
                    | IF '(' Expr ')' Block { $$ = new ASTIfStatementDeclNode($3, $5, nullptr); }
                    | IF '(' Expr ')' Block ELSE Block { $$ = new ASTIfStatementDeclNode($3, $5, $7); }
                    | FOR ID AssignOp Expr ',' Expr Block { $$ = new ASTForStatementDeclNode(*($2), $4, $6, $7); }
                    | RETURN Expr ';' { $$ = new ASTReturnStatementNode($2); }
                    | BREAK ';' { $$ = new ASTBreakStatementNode(); }
                    | CONTINUE ';' { $$ = new ASTContinueStatementNode(); }
                    | Block { $$ = new ASTBlockStatementNode($1); }
                    ;

MethodCall:         ID '(' ExprList ')' { $$ = new ASTSimpleMethodCallNode(*($1), $3); }
                    | CALLOUT '(' STRING_LITERAL ',' CalloutArgList ')' { $$ = new ASTCalloutMethodCallNode(*($3), $5); }
                    ;

ParameterDeclList:  /* empty */ { $$ = new list<ASTParameterDecl *>(); }
                    | nonEmptyParameterDeclList { $$ = $1; }
                    ;

nonEmptyParameterDeclList: ParameterDecl { $$ = new list<ASTParameterDecl *>(); $$->push_back($1); }
                        | nonEmptyParameterDeclList ',' ParameterDecl { $$ = $1; $$->push_back($3); }
                        ;

ParameterDecl:      Type ID { $$ = new ASTParameterDecl($1, *($2), false); }
                    | Type ID '[' ']' { $$ = new ASTParameterDecl($1, *($2), true); }
                    ;

Block:              '{' FieldDeclList StatementDeclList '}' { $$ = new ASTBlock($3); }
                    ;

AssignOp:           ASSIGN { $$ = $1; }
                    | PLUSASSIGN { $$ = $1; }
                    | MINUSASSIGN { $$ = $1; }
                    ;

Location:           ID { $$ = new ASTVarLocationNode(*($1)); }
                    | ID '[' Expr ']' { $$ = new ASTArrayLocationNode(*($1), $3); }
                    ;

ExprList:           /* empty */ { $$ = new list<ASTExpressionNode *>(); }
                    | nonEmptyExprList { $$ = $1; }
                    ;

nonEmptyExprList:   Expr { $$ = new list<ASTExpressionNode *>(); $$->push_back($1); }
                    | nonEmptyExprList ',' Expr { $$ = $1; $$->push_back($3); }
                    ;

Expr:               Location { $$ = new ASTLocationExpressionNode($1); }
                    | MethodCall { $$ = new ASTMethodCallExpressionNode($1); }
                    | IntegerLiteral { $$ = $1; }
                    | MINUS Expr %prec UNARY { $$ = new ASTUnaryExpressionNode($2, *($1)); }
                    | BING Expr { $$ = new ASTUnaryExpressionNode($2, *($1)); }
                    | CHAR_LITERAL { $$ = new ASTCharLiteralExpressionNode((*$1)[0]); }
                    | BOOL_LITERAL { $$ = new ASTBoolLiteralExpressionNode(*($1)); }
                    | '(' Expr ')' { $$ = $2; }
                    | BinaryExpr { $$ = $1; }
                    ;

BinaryExpr:         Expr MULT Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr DIV Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr PLUS Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr MINUS Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr MOD Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr AND Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr OR Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr EQ Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr NEQ Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr GT Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr LT Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr GTEQ Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    | Expr LTEQ Expr { $$ = new ASTBinaryExpressionNode($1, $3, *($2)); }
                    ;

CalloutArgList:     CalloutArg { $$ = new list<ASTCalloutArg *>(); $$->push_back($1); }
                    | CalloutArgList ',' CalloutArg { $$ = $1; $$->push_back($3); }
                    ;

CalloutArg:         Expr { $$ = new ASTExpressionCalloutArg($1); }
                    | STRING_LITERAL { $$ = new ASTStringCalloutArg(*$1); }
                    ;

%%

int yyerror(string s)
{
    extern int yylineno;	// defined and maintained in lex.c
    extern char *yytext;	// defined and maintained in lex.c

    cout << "ERROR: " << s << " at symbol \"" << yytext;
    cout << "\" on line " << yylineno << endl;
    exit(1);
}

int yyerror(char *s)
{
    return yyerror(string(s));
}
