using namespace System;
using namespace System::Collections::Generic;

#include "TypeHelper.h"

ref class Stmt abstract { };
ref class Expr abstract { };

public enum Acess
{
	PUBLIC, PRIVATE, PROTECTED, INTERNAL
};

ref class Class
{
public:
	bool fPublic;
	String^ name;
	IList<Stmt^>^ stmts;
};

ref class Modifier
{
public:
	Acess acess;
	bool fStatic;
};

// <modifier> <type> <ident> = <expr>
ref class VarDeclaration : Stmt
{
public:
	Modifier^ modifier;
	AType^ type;
	String^ name;
	Expr^ expr;
};

// <type> <ident>
ref class Argument
{
public:
	AType^ type;
	String^ name;
};

// <modifier> <type> <ident> ( <args> ) { <stmt>* }
ref class FuncDefinition : Stmt
{
public:
	Modifier^ modifier;
	AType^ type;
	String^ name;
	IList<Argument^>^ args;
	IList<Stmt^>^ body;
};

// <ident> = <expr>
ref class Assign : Stmt
{
public:
	String^ name;
	Expr^ expr;
};

// <expr>;
ref class ExprStmt : Stmt
{
public:
	Expr^ expr;
};

// <stmt>*
ref class Compound
{
public:
	IList<Stmt^>^ stmts;
};

ref class Literal : Expr
{
public:
	Object^ value;
};

ref class Variable : Expr
{
public:
	IList<String^>^ name;
};

// <arith_expr> := <expr> ( + | - | * | / ) <expr>
ref class ArithExpr : Expr
{
public:
	Expr^ left;
	Expr^ right;
	String^ op;
};

ref class FunctionCall : Expr
{
public:
	bool fNew;
	IList<String^>^ name;
	IList<Expr^>^ args;
};