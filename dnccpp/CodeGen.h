using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Reflection;
using namespace System::Reflection::Emit;

#include "Parser.h"

ref class CodeGenerator
{
private:
	Dictionary<String^, TypeBuilder^>^ typeTable;
	Dictionary<String^, ConstructorBuilder^>^ ctorTable;
	Dictionary<String^, MethodBuilder^>^ funcTable;
	Dictionary<String^, Object^>^ symbolTable;
	bool fStaticFunc;

public:
	CodeGenerator(IList<Class^>^, String^);

private:
	void GenStmt(Stmt^, ILGenerator^, array<Type^>^);
	void GenExpr(Expr^, Type^, ILGenerator^, array<Type^>^);
	Type^ TypeOfExpr(Expr^, array<Type^>^);
	void Store(String^, Type^, ILGenerator^);
};