using namespace System;

enum TokenType { SYMB, NAME, LITERAL };

ref class Token
{
public:
	TokenType Type;
	Object^ Value;
	Token(TokenType type, Object^ val)
	{
		Type = type;
		Value = val;
	}
};