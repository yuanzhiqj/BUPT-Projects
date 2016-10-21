
#include <exception>
#include <string>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <memory>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

//#define PRINTCOMMENT
//#define PRINTPREPROCESS

namespace BOT_LexParser
{
	namespace BOT_LexParser_Impl
	{
		constexpr const char *Keywords[]
		{
			"auto", "break", "case", "char", "const", "continue",
			"default", "do", "double", "else", "enum", "extern",
			"float", "for", "goto", "if", "inline", "int", "long",
			"register", "restrict", "return", "short", "signed",
			"sizeof", "static", "struct", "switch", "typedef",
			"union", "unsigned", "void", "volatile", "while",
			"_Bool", "_Complex", "_Imaginary"
		};

		constexpr const char *Punctuators[]
		{
			"[", "]", "(", ")", "{", "}", ".", "->",
			"++", "--", "&", "*", "+", "-", "~", "!",
			"/", "%", "<<", ">>", "<", ">", "<=", ">=",
			"==", "!=", "^", "|", "&&", "||",
			"?", ":", ",", ";",
			"=", "*=", "/=", "%=", "+=", "-=",
			"<<=", ">>=", "&=", "^=", "|="
		};

		constexpr const char *Escapes[]
		{
			"\\'", "\\\"", "\\?", "\\\\",
			"\\a", "\\b", "\\f", "\\n",
			"\\r", "\\t", "\\v"
		};

		bool isKeyword (const std::string &candi)
		{
			static std::unordered_set<std::string> set;
			if (set.empty ())
			{
				auto length = sizeof (Keywords)
					/ sizeof (Keywords[0]);
				for (size_t i = 0; i < length; i++)
					set.emplace (Keywords[i]);
			}
			if (set.find (candi) != set.end ())
				return true;
			return false;
		}

		bool isPunctuator (const std::string &candi)
		{
			static std::unordered_set<std::string> set;
			if (set.empty ())
			{
				auto length = sizeof (Punctuators)
					/ sizeof (Punctuators[0]);
				for (size_t i = 0; i < length; i++)
					set.emplace (Punctuators[i]);
			}
			if (set.find (candi) != set.end ())
				return true;
			return false;
		}

		bool isEscape (const std::string &candi)
		{
			static std::unordered_set<std::string> set;
			if (set.empty ())
			{
				auto length = sizeof (Escapes)
					/ sizeof (Escapes[0]);
				for (size_t i = 0; i < length; i++)
					set.emplace (Escapes[i]);
			}
			if (set.find (candi) != set.end ())
				return true;
			return false;
		}

		bool isWhite (char ch)
		{
			return (ch == ' ' || ch == '\t' || ch == '\v'
					|| ch == '\r' || ch == '\n');
		}
	}

	void LexParsing (std::istream &is,
					 std::ostream &os)
	{
		using namespace BOT_LexParser_Impl;

		size_t cLine = 1;
		size_t cChar = 0;
		std::unordered_map<std::string, size_t> wordCount;
		std::set<std::string> symbols;
		std::vector<std::pair<std::string, std::string>> tokens;
		std::vector<std::pair<std::string, size_t>> errors;

		// Assume Not Mac :-(
		const char endOfLine = '\n';

		// Eat Line
		auto eatLine = [&is, &endOfLine, &cLine, &cChar] (std::string &seg)
		{
			char ch;
			while (true)
			{
				if (!is.get (ch))
					break;
				cChar++;
				if (ch == endOfLine)
					break;
				seg += ch;
			}
			cLine++;
		};

		while (true)
		{
			std::string seg;
			char ch;

			if (!is.get (ch))
				// Eof
				break;
			cChar++;
			seg += ch;
			int nch = is.peek ();

			// Automaton

			// Blank
			if (isWhite (ch))
			{
				if (ch == endOfLine)
					cLine++;
			}
			// Identifier
			else if (isalpha (ch) || ch == '_')
			{
				while (true)
				{
					char ch = is.peek ();
					if (!isalpha (ch) && !isdigit (ch) && ch != '_')
						break;
					seg += (char) is.get ();
					cChar++;
				}

				if (isKeyword (seg))
				{
					tokens.emplace_back ("keyword", seg);
					wordCount["keyword"]++;
				}
				else
				{
					symbols.emplace (seg);
					tokens.emplace_back ("identifier", seg);
					wordCount["identifier"]++;
				}
			}
			// Number or Punc .
			else if (ch == '.' || isdigit (ch))
			{
				// Ignore this Part :-)
				auto _ = 0.0;
				_ = _ = 0;
				_ = .0, 0.;
				_ = 0.F;

				// Decimal
				if (ch == '.' || ch != '0' || (ch == '0' && nch == '.'))
				{
					try
					{
						auto state = 0;
						if (ch == '.')
							state = 2;

						while (true)
						{
							auto isEnd = false;
							char nch = is.peek ();
							switch (state)
							{
							case 0:
								if (isdigit (nch))
									;
								else if (nch == '.')
									//state = 1;
									state = 2;
								else if (nch == 'e' || nch == 'E')
									state = 3;
								else
									isEnd = true;
								break;
								//case 1:
								//	if (isdigit (nch))
								//		state = 2;
								//	else
								//		throw std::runtime_error (
								//			"Bad Fraction: "
								//			+ seg);
								//	break;
							case 2:
								if (isdigit (nch))
									;
								else if (nch == 'e' || nch == 'E')
									state = 3;
								else
									isEnd = true;
								break;
							case 3:
								if (nch == '+' || nch == '-')
									state = 4;
								else if (isdigit (nch))
									state = 5;
								else
									throw std::runtime_error (
										"Bad Exponential: "
										+ seg);
								break;
							case 4:
								if (isdigit (nch))
									state = 5;
								else
									throw std::runtime_error (
										"Bad Exponential: "
										+ seg);
								break;
							case 5:
								if (isdigit (nch))
									;
								else
									isEnd = true;
								break;
							default:
								throw std::runtime_error (
									"Intra Automaton Error");
								break;
							}
							if (isEnd) break;

							// Get Char
							seg += (char) is.get ();
							cChar++;
						}

						if (seg == ".")
							tokens.emplace_back ("punctuator", seg);
						else
							tokens.emplace_back ("constant", seg);
					}
					catch (const std::exception &ex)
					{
						errors.emplace_back (ex.what (), cLine);
					}
				}
				// Hexadecimal
				else if (nch == 'X' || nch == 'x')
				{
					// Eat X
					seg += (char) is.get ();
					cChar++;

					while (true)
					{
						char nch = is.peek ();
						if (!isdigit (nch)
							&& !(nch >= 'A' && nch <= 'F')
							&& !(nch >= 'a' && nch <= 'f'))
							break;

						// Get Char
						seg += (char) is.get ();
						cChar++;
					}

					if (seg.size () > 2)
						tokens.emplace_back ("constant", seg);
					else
						errors.emplace_back ("Invalid 0x", cLine);
				}
				// Octal
				else
				{
					while (true)
					{
						char nch = is.peek ();
						if (nch < '0' || nch > '7')
							break;

						// Get Char
						seg += (char) is.get ();
						cChar++;
					}
					tokens.emplace_back ("constant", seg);
				}
				wordCount["constant"]++;
			}
			// Char or String Literal
			else if (ch == '\'' || ch == '"')
			{
				auto chType = ch;
				try
				{
					while (true)
					{
						if (!is.get (ch))
							break;
						seg += ch;
						cChar++;

						if (ch == chType)
							break;

						if (ch == endOfLine)
						{
							seg.pop_back ();
							cLine++;
							throw std::runtime_error (
								"No End Bracket: " + seg);
						}

						if (ch == '\\')
						{
							std::string tmp (1, ch);
							is.get (ch);
							tmp += ch;
							cChar++;

							if (ch == endOfLine)
							{
								// Cross Line String
								seg.pop_back ();
								cLine++;
							}
							else if (isEscape (tmp))
								seg += ch;
							else
							{
								// Skip this Error
								seg.pop_back ();
								errors.emplace_back (
									"Invalid Escape Char: " + tmp,
									cLine);
							}
						}
					}
					if (chType == '\'')
					{
						if (seg.size () <= 2)
							throw std::runtime_error (
								"Too few chars inside Pair ''");
						tokens.emplace_back ("constant", seg);
						wordCount["constant"]++;
					}
					else
					{
						tokens.emplace_back ("string", seg);
						wordCount["string"]++;
					}
				}
				catch (const std::exception &ex)
				{
					errors.emplace_back (ex.what (), cLine);
				}
			}
			// Block Comment
			else if (ch == '/' && nch == '*')
			{
				while (true)
				{
					if (!is.get (ch))
						break;
					cChar++;

					// Cross Line Comment
					if (ch == endOfLine)
					{
						cLine++;
						seg += "\\n";
					}
					else
						seg += ch;

					if (ch == '*' && is.peek () == '/')
					{
						seg += (char) is.get ();
						cChar++;
						break;
					}
				}
#ifdef PRINTCOMMENT
				tokens.emplace_back ("comment", seg);
#endif // PRINTCOMMENT
					}
			// Line Comment
			else if (ch == '/' && nch == '/')
			{
				eatLine (seg);
#ifdef PRINTCOMMENT
				tokens.emplace_back ("comment", seg);
#endif // PRINTCOMMENT
				}
			// PreProcessing
			else if (ch == '#')
			{
				eatLine (seg);
#ifdef PRINTPREPROCESS
				tokens.emplace_back ("preprocess", seg);
#endif // PRINTPREPROCESS
			}
			// Punc or Err
			else
			{
				bool isPunc = isPunctuator (seg);
				while (true)
				{
					// Peek following
					char nch = is.peek ();
					if (isWhite (nch) || isalnum (nch) || nch == EOF)
						break;

					// Some Punc can be the prefix of other Punc
					if (!isPunctuator (seg + nch))
						break;

					seg += (char) is.get ();
					cChar++;
				}

				if (isPunc)
					tokens.emplace_back ("punctuator", seg);
				else
					errors.emplace_back (
						"Invalid Punctuator: " + seg, cLine);
			}
		}  // end of While

		// Output
		os << "Total Lines: " << cLine << std::endl
			<< "Total Chars: " << cChar << std::endl;

		os << "\nTotal Words:\n";
		for (auto &word : wordCount)
			os << "[" << std::setw (10) << std::right
			<< word.first << "]:\t"
			<< word.second << std::endl;

		os << "\nSymbols:\n";
		for (auto &symbol : symbols)
			os << symbol << std::endl;

		os << "\nTokens:\n";
		for (auto &token : tokens)
			os << "<" << std::setw (10) << std::right
			<< token.first << ">\t"
			<< token.second << std::endl;

		if (!errors.empty ())
		{
			os << "\nErrors:\n";
			for (auto &error : errors)
				os << "[" << error.second << "]\t"
				<< error.first << std::endl;
		}
	}
}

int main (int argc, char *argv[])
{
	try
	{
		using namespace BOT_LexParser;

		if (argc < 2)
			throw std::runtime_error ("Too few Arguments");

		std::ifstream ifs (argv[1]);
		std::ofstream ofs (argv[1] + std::string (".output.txt"));

		if (!ifs.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1]);
		if (!ofs.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1] + ".output.txt");

		// Parse ifs into ofs :-)
		LexParsing (ifs, ofs);
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what () << std::endl;
		return 1;
	}
	return 0;
}