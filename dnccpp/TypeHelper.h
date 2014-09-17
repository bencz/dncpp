using namespace System;
using namespace System::Collections::Generic;

ref class AType
{
public:
	IList<String^>^ name;
	String^ GetTypeName()
	{
		String^ result = name[0];
		for (int i = 1; i < name->Count; i++)
			result += "." + name[i];
		return result;
	}

	Type^ GetAType()
	{
		String^ result = name[0];
		if (result == "void") return void::typeid;
		if (result == "int") return int::typeid;
		if (result == "float") return float::typeid;
		if (result == "double") return double::typeid;
		if (result == "char") return char::typeid;
		if (result == "string") return String::typeid;
		for (int i = 1; i < name->Count; i++)
			result += "." + name[i];
		return Type::GetType(result);
	}
};