#include "CodeGen.h"

CodeGenerator::CodeGenerator(IList<Class^>^ classes, String^ moduleName)
{
	AssemblyName^ name = gcnew AssemblyName(moduleName);
	AssemblyBuilder^ asmb = AppDomain::CurrentDomain->DefineDynamicAssembly(name, AssemblyBuilderAccess::Save);
	ModuleBuilder^ moduleBuilder = asmb->DefineDynamicModule(moduleName + ".exe");

	typeTable = gcnew Dictionary<String^, TypeBuilder^>();
	ctorTable = gcnew Dictionary<String^, ConstructorBuilder^>();
	funcTable = gcnew Dictionary<String^, MethodBuilder^>();

	for each (auto cls in classes)
	{
		TypeBuilder^ typeBuilder = moduleBuilder->DefineType(cls->name, TypeAttributes::Class);
		ConstructorBuilder^ ctorBuilder = typeBuilder->DefineDefaultConstructor(MethodAttributes::Public);

		typeTable[cls->name] = typeBuilder;
		ctorTable[cls->name] = ctorBuilder;

		for each (auto stmt in cls->stmts)
		{
			if (dynamic_cast<VarDeclaration^>(stmt) != nullptr)
			{
				Console::WriteLine("VarDecl: todo");
				auto var = (VarDeclaration^)stmt;
				Console::WriteLine(var->name);
			}
			else if (dynamic_cast<FuncDefinition^>(stmt) != nullptr)
			{
				symbolTable = gcnew Dictionary<String^, Object^>();
				FuncDefinition^ func = (FuncDefinition^)stmt;
				MethodAttributes attr = func->modifier->acess == Acess::PUBLIC
					? MethodAttributes::Public : MethodAttributes::Private;

				if (func->modifier->fStatic)
					attr = attr | MethodAttributes::Static;
				fStaticFunc = func->modifier->fStatic;

				IList<Argument^>^ args = func->args;
				array<Type^>^ types = gcnew array<Type^>(args->Count);
				for (int i = 0; i < args->Count; i++)
				{
					types[i] = args[i]->type->GetAType();
					symbolTable[args[i]->name] = i;
				}

				MethodBuilder^ funcBuilder = typeBuilder->DefineMethod(func->name, attr, func->type->GetAType(), types);

				funcTable[func->name] = funcBuilder;
				ILGenerator^ il = funcBuilder->GetILGenerator();
				for each (auto stmt1 in func->body)
					GenStmt(stmt1, il, types);

				il->Emit(OpCodes::Ret);

				if (func->name == "Main" || func->name == "main")
					asmb->SetEntryPoint(funcBuilder);
			}
			else
				throw gcnew Exception("Code generator: stmt is " + stmt);
		}
		typeBuilder->CreateType();
	}
	moduleBuilder->CreateGlobalFunctions();
	asmb->Save(moduleName + ".exe");
}

void CodeGenerator::GenStmt(Stmt^ stmt, ILGenerator^ il, array<Type^>^ argTypes)
{
	if (dynamic_cast<VarDeclaration^>(stmt) != nullptr)
	{
		VarDeclaration^ var = (VarDeclaration^)stmt;
		String^ tName = var->type->GetTypeName();
		if (typeTable->ContainsKey(tName))
		{
			TypeBuilder^ typeBuilder = typeTable[tName];
			ConstructorBuilder^ ctorBuilder = ctorTable[tName];
			LocalBuilder^ localBuilder = il->DeclareLocal(typeBuilder);
			symbolTable[var->name] = localBuilder;
			il->Emit(OpCodes::Newobj, ctorBuilder);
			il->Emit(OpCodes::Stloc, localBuilder);
		}
		else
		{
			Type^ vType = var->type->GetAType();
			symbolTable[var->name] = il->DeclareLocal(vType);
			GenExpr(var->expr, TypeOfExpr(var->expr, argTypes), il, argTypes);
			Store(var->name, TypeOfExpr(var->expr, argTypes), il);
		}
	}
	else if (dynamic_cast<Assign^>(stmt) != nullptr)
	{
		Assign^ assign = (Assign^)stmt;
		GenExpr(assign->expr, TypeOfExpr(assign->expr, argTypes), il, argTypes);
		Store(assign->name, TypeOfExpr(assign->expr, argTypes), il);
	}
	else if (dynamic_cast<ExprStmt^>(stmt) != nullptr)
	{
		Expr^ expr = ((ExprStmt^)stmt)->expr;
		
		if (dynamic_cast<FunctionCall^>(expr) != nullptr)
		{
			FunctionCall^ funcCall = (FunctionCall^)expr;
			if (funcCall->name->Count > 1 && funcCall->name[0] != "System")
			{
				if (symbolTable->ContainsKey(funcCall->name[0]))
				{
					LocalBuilder^ localBuilder = (LocalBuilder^)symbolTable[funcCall->name[0]];
					il->Emit(OpCodes::Ldloc, localBuilder);
				}
			}

			array<Type^>^ typeArgs = gcnew array<Type^>(funcCall->args->Count);
			for (int i = 0; i < funcCall->args->Count; i++)
			{
				typeArgs[i] = TypeOfExpr(funcCall->args[i], argTypes);
				GenExpr(funcCall->args[i], typeArgs[i], il, argTypes);
			}

			String^ strFunc = funcCall->name[funcCall->name->Count - 1];
			if (funcCall->name[0] == "System")
			{
				String^ strType = funcCall->name[0];
				for (int i = 1; i<funcCall->name->Count - 1; i++)
					strType += "." + funcCall->name[i];

				Type^ typeFunc = Type::GetType(strType);
				il->Emit(OpCodes::Call, typeFunc->GetMethod(strFunc, typeArgs));
			}
			else if (funcCall->name->Count > 1)
			{
				MethodBuilder^ methodBuilder = funcTable[strFunc];
				il->EmitCall(OpCodes::Call, methodBuilder, nullptr);
			}
			else
			{
				if (!funcTable->ContainsKey(strFunc))
					throw gcnew Exception("undeclared function '" + strFunc + "'");

				MethodBuilder^ funcBuilder = funcTable[strFunc];
				il->EmitCall(OpCodes::Call, funcBuilder, nullptr);
			}
		}
	}
}

void CodeGenerator::GenExpr(Expr^ expr, Type^ expectedType, ILGenerator^ il, array<Type^>^ argTypes)
{
	Type^ deliveredType;

	if (dynamic_cast<Literal^>(expr) != nullptr)
	{
		Object^ val = ((Literal^)expr)->value;
		deliveredType = val->GetType();
		if (dynamic_cast<String^>(val) != nullptr)
			il->Emit(OpCodes::Ldstr, (String^)val);
		else if (dynamic_cast<Int32^>(val) != nullptr)
			il->Emit(OpCodes::Ldc_I4, (int)val);
		else if (dynamic_cast<Double^>(val) != nullptr)
			il->Emit(OpCodes::Ldc_R8, (double)val);
	}

	else if (dynamic_cast<Variable^>(expr) != nullptr)
	{
		String^ ident = ((Variable^)expr)->name[0];
		deliveredType = TypeOfExpr(expr, argTypes);
		if (!symbolTable->ContainsKey(ident))
			throw gcnew Exception("undeclared variable '" + ident + "'");

		Object^ var = symbolTable[ident];
		if (dynamic_cast<LocalBuilder^>(var) != nullptr)
			il->Emit(OpCodes::Ldloc, (LocalBuilder^)var);
		else if (dynamic_cast<Int32^>(var) != nullptr)
			il->Emit(OpCodes::Ldarg, (int)var + (fStaticFunc ? 0 : 1));
		else
			throw gcnew Exception("invalid: " + var);
	}

	else if (dynamic_cast<ArithExpr^>(expr) != nullptr)
	{
		ArithExpr^ arithExpr = (ArithExpr^)expr;
		GenExpr(arithExpr->left, TypeOfExpr(arithExpr->left, argTypes), il, argTypes);
		GenExpr(arithExpr->right, TypeOfExpr(arithExpr->right, argTypes), il, argTypes);

		if (arithExpr->op == "+")
			il->Emit(OpCodes::Add);
		else if (arithExpr->op == "-")
			il->Emit(OpCodes::Sub);
		else if (arithExpr->op == "*")
			il->Emit(OpCodes::Mul);
		else if (arithExpr->op == "/")
			il->Emit(OpCodes::Div);
		else
			throw gcnew Exception("unsuported operator '" + arithExpr->op + "'");

		deliveredType = TypeOfExpr(arithExpr, argTypes);
	}

	else
		throw gcnew Exception("don't know how to generate " + expr->GetType()->Name);

	if (deliveredType != expectedType)
	{
		if (deliveredType == int::typeid && expectedType == String::typeid)
		{
			il->Emit(OpCodes::Box, int::typeid);
			il->Emit(OpCodes::Callvirt, (Object::typeid)->GetMethod("ToString"));
		}
		else
			throw gcnew Exception("can't coerce a " + deliveredType->Name + " to a " + expectedType);
	}
}

Type^ CodeGenerator::TypeOfExpr(Expr^ expr, array<Type^>^ argTypes)
{
	if (dynamic_cast<Literal^>(expr) != nullptr)
		return ((Literal^)expr)->value->GetType();
	else if (dynamic_cast<Variable^>(expr) != nullptr)
	{
		Variable^ var = (Variable^)expr;
		String^ vName = var->name[0];
		if (symbolTable->ContainsKey(vName))
		{
			if (dynamic_cast<LocalBuilder^>(symbolTable[vName]) != nullptr)
				return ((LocalBuilder^)symbolTable[vName])->LocalType;
			else
			{
				int ixArg = (int)symbolTable[vName];
				return argTypes[ixArg];
			}
		}
		else
			throw gcnew Exception("undeclared variable '" + vName + "'");
	}
	else if (dynamic_cast<ArithExpr^>(expr) != nullptr)
		return TypeOfExpr(((ArithExpr^)expr)->left, argTypes);
	else
		throw gcnew Exception("don't know how to calculate the type of " + expr->GetType()->Name);
}

void CodeGenerator::Store(String^ name, Type^ type, ILGenerator^ il)
{
	if (symbolTable->ContainsKey(name))
	{
		LocalBuilder^ locb = (LocalBuilder^)symbolTable[name];
		if (locb->LocalType == type || (locb->LocalType == double::typeid && type == int::typeid))
			il->Emit(OpCodes::Stloc, locb);
		else
			throw gcnew Exception("'" + name + "' is of type " + locb->LocalType->Name +
					" but attempted to store value of type " + type->Name);
	}
	else
		throw gcnew Exception("undeclared variable '" + name + "'");
}