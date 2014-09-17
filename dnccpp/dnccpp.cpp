using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

#include "CodeGen.h"

int main(array<System::String ^> ^args)
{
	auto input = File::OpenText("test.txt");
	Scanner^ scanner = gcnew Scanner(input);
	Parser^ parser = gcnew Parser(scanner->Tokens);
	gcnew CodeGenerator(parser->Result, Path::GetFileNameWithoutExtension("test.txt"));
    return 0;
}
