#include "Scanner.h"

void Scanner::Scan(String^ src)
{
	int ix, ixBgn;
	for (ix = 0; ix < src->Length;)
	{
		if (ix + 1 < src->Length && src[ix] == '/' && src[ix + 1] == '/')
		{
			for (ix += 2; ix < src->Length && src[ix] != '\n';)
				ix++;
		}
		else if (Char::IsWhiteSpace(src[ix]))
		{
			ix++;
		}
		else if (Char::IsLetter(src[ix]) || src[ix] == '_')
		{
			for (ixBgn = ix; ix < src->Length &&
				(Char::IsLetter(src[ix]) || Char::IsDigit(src[ix]) || src[ix] == '_');)
			{
				ix++;
			}
			auto ident = src->Substring(ixBgn, ix - ixBgn);
			result->Add(gcnew Token(TokenType::NAME, ident));
		}
		else if (src[ix] == '"')
		{
			ixBgn = ++ix;
			while (ix < src->Length && src[ix] != '"')
				ix++;
			if (ix >= src->Length || src[ix] != '"')
				throw gcnew Exception("unterminated string literal");

			result->Add(gcnew Token(TokenType::LITERAL, src->Substring(ixBgn, ix - ixBgn)));
			ix++;
		}
		else if (src[ix] == '\'')
		{
			ix++; // skip the '
			if (ix + 1 >= src->Length || src[ix + 1] != '\'')
				throw gcnew Exception("unterminated character literal");
			result->Add(gcnew Token(TokenType::LITERAL, src[ix++]));
			ix++;
		}
		else if (Char::IsDigit(src[ix]))
		{
			int nDot = 0;
			for (ixBgn = ix; ix < src->Length && (Char::IsDigit(src[ix]) || src[ix] == '.'); ix++)
			{
				if (src[ix] == '.')
					nDot++;
			}
			if (nDot == 0)
				result->Add(gcnew Token(TokenType::LITERAL, Int32::Parse(src->Substring(ixBgn, ix - ixBgn))));
			else if (nDot == 1)
				result->Add(gcnew Token(TokenType::LITERAL, Double::Parse(src->Substring(ixBgn, ix - ixBgn))));
			else
				throw gcnew Exception("Scanner: found more then one dot in number");
		}
		else
		{
			String^ k = "+-*/=;()[]{}.,";
			int x = k->IndexOf(src[ix]);
			if (x >= 0)
				result->Add(gcnew Token(TokenType::SYMB, src->Substring(ix, 1)));
			else
				throw gcnew Exception("Scanner: unrecognized character '" + src[ix] + "'");
			ix++;
		}
	}
}

Scanner::Scanner(TextReader^ reader)
{
	result = gcnew List<Token^>();
	String^ source = reader->ReadToEnd();
	Scan(source);
}