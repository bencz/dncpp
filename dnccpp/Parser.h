using namespace System;
using namespace System::Collections::Generic;

#include "ast.h"
#include "Scanner.h"

ref class Parser
{
private:
	IList<Class^>^ result;
	IList<Token^>^ tokens;
	int ix;
public:
	property IList<Class^>^ Result
	{
		IList<Class^>^ get()
		{
			return result;
		}
	}

public:
	Parser(IList<Token^>^);

private:
	void Skip(TokenType,String^);
	void Skip(String^);
	String^ GetName();
	String^ GetSymbol();
	bool GetKeyword(String^);
	bool Is(String^);
	IList<Stmt^>^ ParseCompoundStmt();
	IList<String^>^ ParseName();
	AType^ ParseType();
	IList<Argument^>^ ParseArguments();
	Class^ ParseClass();
	Modifier^ ParseModifier();
	Stmt^ ParseStmt();
	FunctionCall^ ParseFuncCall(bool, IList<String^>^);
	Expr^ ParseExpr();
	Expr^ ParseAddExpr();
	Expr^ ParseMulExpr();
	Expr^ ParsePrimExpr();
};