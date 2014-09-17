using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

#include "Tokens.h"

ref class Scanner
{
private:
	IList<Token^>^ result;

public:
	property IList<Token^>^ Tokens
	{
		IList<Token^>^ get()
		{
			return result;
		}
	}

private:
	void Scan(String^ src);

public:
	Scanner(TextReader^ reader);
};
