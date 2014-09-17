#include "Parser.h"

Parser::Parser(IList<Token^>^ in_tokens)
{
	tokens = in_tokens;
	result = gcnew List<Class^>();
	for (ix = 0; ix < tokens->Count;)
		result->Add(ParseClass());
}


void Parser::Skip(TokenType type, String^ str)
{
	if (ix == tokens->Count)
		throw gcnew Exception("expected '" + str + "', got EOF");
	else if (tokens[ix]->Type != type || (String^)tokens[ix]->Value != str)
		throw gcnew Exception("expected '" + str + "', got '" + tokens[ix]->Value + "'");
	ix++;
}

void Parser::Skip(String^ str)
{
	Skip(TokenType::SYMB, str);
}

String^ Parser::GetName()
{
	if (ix == tokens->Count || tokens[ix]->Type != TokenType::NAME)
		return nullptr;

	return (String^)tokens[ix++]->Value;
}

String^ Parser::GetSymbol()
{
	if (ix == tokens->Count || tokens[ix]->Type != TokenType::SYMB)
		return nullptr;

	return (String^)tokens[ix++]->Value;
}

bool Parser::GetKeyword(String^ keyword)
{
	if (ix == tokens->Count || tokens[ix]->Type != TokenType::NAME)
		return false;
	if ((String^)tokens[ix]->Value == keyword)
	{
		ix++;
		return true;
	}
	return false;
}

bool Parser::Is(String^ is)
{
	if (ix == tokens->Count)
		return false;
	if (tokens[ix]->Type != TokenType::SYMB && tokens[ix]->Type != TokenType::NAME)
		return false;
	return ((String^)tokens[ix]->Value == is);
}

// { <stmt>* }
IList<Stmt^>^ Parser::ParseCompoundStmt()
{
	IList<Stmt^>^ stmts = gcnew List<Stmt^>();
	Skip("{");
	while (ix < tokens->Count && !Is("}"))
	{
		stmts->Add(ParseStmt());
		if (Is((";")))
			ix++;
	}
	Skip("}");
	return stmts;
}

// <ident> (. <ident>)*
IList<String^>^ Parser::ParseName()
{
	IList<String^>^ name = gcnew List<String^>();
	while (ix < tokens->Count && tokens[ix]->Type == TokenType::NAME)
	{
		name->Add((String^)tokens[ix++]->Value);
		if (!Is("."))
			break;
		Skip(".");
	}
	return name;
}

AType^ Parser::ParseType()
{
	AType^ type = gcnew AType();
	type->name = ParseName();
	return type;
}

// ( <args>* )
IList<Argument^>^ Parser::ParseArguments()
{
	IList<Argument^>^ args = gcnew List<Argument^>();
	while (ix < tokens->Count && GetSymbol() != ")")
	{
		Argument^ arg = gcnew Argument();
		arg->type = ParseType();
		arg->name = GetName();
		args->Add(arg);
	}
	return args;
}

Class^ Parser::ParseClass()
{
	Class^ cls = gcnew Class();
	cls->fPublic = GetKeyword("public");
	Skip(TokenType::NAME, "class");
	cls->name = GetName();
	cls->stmts = ParseCompoundStmt();
	return cls;
}

Modifier^ Parser::ParseModifier()
{
	bool fPublic = GetKeyword("public");
	bool fPrivate = GetKeyword("private");
	bool fStatic = GetKeyword("static");
	if (!fPublic && !fPrivate)
		fPrivate = true;
	Modifier^ mod = gcnew Modifier();
	if (fPublic)
		mod->acess = fPublic ? Acess::PUBLIC : Acess::PRIVATE;
	mod->fStatic = fStatic;
	return mod;
}

Stmt^ Parser::ParseStmt()
{
	Modifier^ modifier = ParseModifier();
	IList<String^>^ name1 = ParseName(); // type
	IList<String^>^ name2 = ParseName(); // variable

	String^ symb = GetSymbol();
	if (name1->Count > 0 && name2->Count > 0 && symb == "(")
	{
		FuncDefinition^ result = gcnew FuncDefinition();
		result->modifier = modifier;
		result->type = gcnew AType();
		result->type->name = name1;
		result->name = name2[0];
		result->args = ParseArguments();
		result->body = ParseCompoundStmt();
		return result;
	}
	else if (name1->Count > 0 && name2->Count > 0 && symb == "=")
	{
		VarDeclaration^ var = gcnew VarDeclaration();
		var->type = gcnew AType();
		var->type->name = name1;
		var->name = name2[0];
		var->expr = ParseExpr();
		Skip(";");
		return var;
	}
	else if (name1->Count > 0 && name2->Count == 0 && symb == "=")
	{
		VarDeclaration^ result = gcnew VarDeclaration();
		return result;
	}
	else if (name1->Count > 0 && name2->Count == 0 && symb == "(")
	{ // hello.Print(str);
		ExprStmt^ stmt = gcnew ExprStmt();
		stmt->expr = ParseFuncCall(false, name1);
		return stmt;
	}
	else
	{
		ExprStmt^ stmt = gcnew ExprStmt();
		stmt->expr = ParseExpr();
		return stmt;
	}
}

FunctionCall^ Parser::ParseFuncCall(bool fNew, IList<String^>^ name)
{
	FunctionCall^ funcCall = gcnew FunctionCall();
	funcCall->fNew = fNew;
	funcCall->name = name;
	funcCall->args = gcnew List<Expr^>();
	while (ix < tokens->Count && !Is(")"))
	{
		funcCall->args->Add(ParseExpr());
		if (Is(","))
			ix++;
	}
	Skip(")");
	return funcCall;
}

Expr^ Parser::ParseExpr()
{
	return ParseAddExpr();
}

Expr^ Parser::ParseAddExpr()
{
	Expr^ left = ParseMulExpr();
	if (!Is("+") && !Is("-"))
		return left;
	ArithExpr^ expr = gcnew ArithExpr();
	expr->op = (String^)tokens[ix++]->Value;
	expr->left = left;
	expr->right = ParseMulExpr();
	return expr;
}

Expr^ Parser::ParseMulExpr()
{
	Expr^ left = ParsePrimExpr();
	if (!Is("*") && !Is("/"))
		return left;
	ArithExpr^ expr = gcnew ArithExpr();
	expr->op = (String^)tokens[ix++]->Value;
	expr->left = left;
	expr->right = ParsePrimExpr();
	return expr;
}

Expr^ Parser::ParsePrimExpr()
{
	if (ix == tokens->Count)
		throw gcnew Exception("expected expression, got EOF");
	if (tokens[ix]->Type == TokenType::LITERAL)
	{
		Literal^ literal = gcnew Literal();
		literal->value = tokens[ix++]->Value;
		return literal;
	}
	else if (tokens[ix]->Type == TokenType::NAME)
	{
		bool fNew = false;
		if (Is("new"))
		{
			ix++;
			fNew = true;
		}

		IList<String^>^ name = ParseName();
		if (Is("("))
		{
			Skip("(");
			return ParseFuncCall(fNew, name);
		}
		else
		{
			Variable^ var = gcnew Variable();
			var->name = name;
			return var;
		}
	}
	else
		throw gcnew Exception("expected literal or variable, got " + tokens[ix]->Value);
}