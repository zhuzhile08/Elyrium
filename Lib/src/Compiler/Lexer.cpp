#include "LSD/StringView.h"
#include <Elyrium/Compiler/Lexer.hpp>

#include <LSD/UnorderedFlatMap.h>


#define REPORT_INV_NUM { \
	throwSyntaxError(error::Message::invalidNumericLiteral); \
	return Token(); \
}

#define VALIDATE_NUM(check) \
do { \
	c = next(); \
	if (check) { \
		prevUnderscore = false; \
		continue; \
	} if (c == '_') { \
		if (prevUnderscore) REPORT_INV_NUM; \
		prevUnderscore = true; \
		continue; \
	} break; \
} while (true);


namespace elyrium {

namespace compiler {

void Lexer::throwSyntaxError(error::Message message, char expected) {
	std::size_t additionalSpaces { };
	auto source = currentLine(additionalSpaces);

	if (expected != '\0')
		throw SyntaxError(m_path, m_line, m_column + expected, source, message, expected); \
	else throw SyntaxError(m_path, m_line, m_column + expected, source, message); \
}

Token Lexer::nextToken() {
	if (m_iter == m_source.end()) return Token(Token::Type::eof);
	skipEmpty();
	if (m_iter == m_source.end()) return Token(Token::Type::eof);

	switch (*m_iter) {
		// Strings
		case '"': {
			auto line = m_line, col = m_column;
			auto c = next();
			auto begin = m_iter;

			bool finishedParsing = false;
			while ((c = next()) != '\0') {
				switch (c) {	
					case '\n':
						m_column = -1;
						++m_line;

						continue;

					case '\\':
						verifyEscapeSequence();

					default:
						m_column += 1;

						continue;

					case '"':
						finishedParsing = true;
				}

				break;
			}

			if (!finishedParsing) {
				throwSyntaxError(error::Message::expectedDifferent, '"');

				break;
			}

			++m_column;
			return Token(Token::Type::string, { begin, m_iter++ }, line, col); // m_iter++ skips the last quotation mark which couldn't be skipped above
		}


		// Single characters
		case '\'': {
			auto col = m_column;
			auto c = next();
			auto begin = m_iter;

			switch (c) {
				case '\0':
					throwSyntaxError(error::Message::expectedDifferent, '\'');

					break;

				case '\'':
					throwSyntaxError(error::Message::emptyChar);

					break;

				case '\n':
				case '\t':
					throwSyntaxError(error::Message::unescapedChar);

					break;

				case '\\':
					verifyEscapeSequence();
			}

			if (next() != '\'')
				throwSyntaxError(error::Message::expectedDifferent, '\'');
	
			++m_column;
			return Token(Token::Type::character, { begin, m_iter++ }, m_line, col);

			break;
		}


		// Single character tokens

		case ';':
			return singleCharTok(Token::Type::semicolon);

		case ',':
			return singleCharTok(Token::Type::comma);

		case '(':
			return singleCharTok(Token::Type::parenLeft);

		case ')':
			return singleCharTok(Token::Type::parenRight);

		case '{':
			return singleCharTok(Token::Type::braceLeft);

		case '}':
			return singleCharTok(Token::Type::braceRight);

		case '[':
			return singleCharTok(Token::Type::bracketLeft);

		case ']':
			return singleCharTok(Token::Type::bracketRight);

		case ':':
			return singleCharTok(Token::Type::colon);



		// Double character tokens

		case '=':
			return doubleCharTok(Token::Type::assign, Token::Type::equal);

		case '|':
			return doubleCharTok(Token::Type::bitOr, Token::Type::logicOr);


		// Tokens with an equal appended


		case '/':
			return singleCharEqualTok(Token::Type::div, Token::Type::assignDiv);

		case '*':
			return singleCharEqualTok(Token::Type::mul, Token::Type::assignMul);

		case '%':
			return singleCharEqualTok(Token::Type::mod, Token::Type::assignMod);

		case '!':
			return singleCharEqualTok(Token::Type::logicNot, Token::Type::notEqual);

		case '&':
			return singleCharEqualTok(Token::Type::bitAnd, Token::Type::assignBitAnd);

		case '~':
			return singleCharEqualTok(Token::Type::bitNot, Token::Type::assignBitNot);

		case '^':
			return singleCharEqualTok(Token::Type::bitXOr, Token::Type::assignBitXOr);


		// More complicated cases

		case '+':
			return singleCharEqualOrDoubleCharTok(Token::Type::add, Token::Type::increment, Token::Type::assignAdd);

		case '-':
			return singleCharEqualOrDoubleCharTok(Token::Type::sub, Token::Type::decrement, Token::Type::assignSub);


		case '>': {
			auto begin = m_iter;
			auto col = m_column++;

			if (auto c = *m_iter, n = next(); n == c) {
				if (auto n = next(); n == '=') {
					next();

					return Token(Token::Type::assignShiftRight, { begin, m_iter }, m_line, col);
				}

				return Token(Token::Type::shiftRight, { begin, m_iter }, m_line, col);
			} else if (n == '=') {
				next();

				return Token(Token::Type::greaterEqual, { begin, m_iter }, m_line, col);
			}

			return Token(Token::Type::greater, { begin, m_iter }, m_line, col);
		}

		case '<': { // Most complex operator case, as it may parse to <, <=, <=>, <<, <<=
			auto begin = m_iter;
			auto col = m_column++;

			if (auto c = *m_iter, n = next(); n == c) {
				if (auto n = next(); n == '=') {
					next();

					return Token(Token::Type::assignShiftRight, { begin, m_iter }, m_line, col);
				}

				return Token(Token::Type::shiftRight, { begin, m_iter }, m_line, col);
			} else if (n == '=') {
				if (auto n = next(); n == '>') {
					next();

					return Token(Token::Type::spaceship, { begin, m_iter }, m_line, col);
				}

				return Token(Token::Type::greaterEqual, { begin, m_iter }, m_line, col);
			}

			return Token(Token::Type::greater, { begin, m_iter }, m_line, col);
		}


		// Attributes

		case '@': {
			auto begin = ++m_iter;
			auto col = m_column++;
			
			auto c = *m_iter;
			while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
				c = next();

			return Token(Token::Type::attribute, { begin, m_iter}, m_line, col);
		}


		// Remaining literals, keywords and identifiers

		case '.':
			if (auto c = *(m_iter + 1); c >= '0' && c <= '9')
				return numericLiteral(true);

			return singleCharTok(Token::Type::dot);

		default:
			if (auto c = *m_iter; c >= '0' && c <= '9')
				return numericLiteral(false);

			return keywordOrIdentifier();
	}

	return Token();
}


Token Lexer::singleCharTok(Token::Type type) {
	auto begin = m_iter++;

	return Token(type, { begin, m_iter }, m_line, m_column++);
}

Token Lexer::doubleCharTok(Token::Type singleChar, Token::Type doubleChar) {
	auto begin = m_iter;
	auto col = m_column;

	if (auto c = *m_iter, n = next(); n == c) {
		next();
		
		return Token(doubleChar, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}

Token Lexer::singleCharEqualTok(Token::Type singleChar, Token::Type equal) {
	auto begin = m_iter;
	auto col = m_column;

	if (auto n = next(); n == '=') {
		next();

		return Token(equal, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}

Token Lexer::singleCharEqualOrDoubleCharTok(Token::Type singleChar, Token::Type doubleChar, Token::Type equal) {
	auto begin = m_iter;
	auto col = m_column;

	if (auto c = *m_iter, n = next(); n == c) {
		next();

		return Token(doubleChar, { begin, m_iter }, m_line, col);
	} else if (n == '=') {
		next();

		return Token(equal, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}


Token Lexer::numericLiteral(bool guaranteedFloat) {
	auto col = m_column;
	auto begin = m_iter;

	bool prevUnderscore = true;
	auto c = *m_iter;

	auto type = Token::Type::integral;
	if (guaranteedFloat) goto parseFloatBehindDecPoint;

	if (c == '0') {
		switch (c = next()) {
			case 'B': case 'b':
				VALIDATE_NUM(c == '0' || c == '1');

				break;

			case 'O': case 'o':
				VALIDATE_NUM(c >= '0' && c <= '7');

				break;

			case 'X': case 'x':
				VALIDATE_NUM(std::isxdigit(c));

				break;

			default:
				if (std::isdigit(c))
					goto parseRegularNumber;

				break;
		}
	} else {
	parseRegularNumber:
		VALIDATE_NUM(std::isdigit(c));

		if (c == '.') {
			prevUnderscore = true;

		parseFloatBehindDecPoint:
			type = Token::Type::floating;

			auto currCol = m_column;
			VALIDATE_NUM(std::isdigit(c));

			if (c == 'e') {
				if (currCol - m_column == 0) REPORT_INV_NUM;

				prevUnderscore = true;
				if (c = *(m_iter + 1); c == '+' || c == '-') next();

				currCol = m_column;
				VALIDATE_NUM(std::isdigit(c));
				if (currCol - m_column == 0) REPORT_INV_NUM;
			} else if (guaranteedFloat && currCol - m_column == 0) REPORT_INV_NUM;
		}
	}

	if (c == '_') REPORT_INV_NUM;
	if (c == 'U' || c == 'u') {
		if (type == Token::Type::floating) REPORT_INV_NUM;
		type = Token::Type::unsignedIntegral;
	} else if (c == 'f') type = Token::Type::floating;

	return Token(type, { begin, m_iter }, m_line, col);
}

Token Lexer::keywordOrIdentifier() {
	static const lsd::UnorderedFlatMap<lsd::StringView, Token::Type> lookup {
		{ "null", Token::Type::kNull },

		{ "true", Token::Type::kTrue },
		{ "false", Token::Type::kFalse },

		{ "move", Token::Type::kMove },
	
		{ "if", Token::Type::kIf },
		{ "else", Token::Type::kElse },
		{ "for", Token::Type::kFor },
		{ "do", Token::Type::kDo },
		{ "break", Token::Type::kBreak },
		{ "continue", Token::Type::kContinue },
		{ "return", Token::Type::kReturn },
		{ "yield", Token::Type::kYield },

		{ "raise", Token::Type::kRaise },
		{ "try", Token::Type::kTry },
		{ "catch", Token::Type::kCatch },

		{ "this", Token::Type::kThis },

		{ "let", Token::Type::kLet },

		{ "func", Token::Type::kFunc },
		{ "coroutine", Token::Type::kCoroutine },
		{ "enum", Token::Type::kEnum },
		{ "class", Token::Type::kClass },
		{ "type", Token::Type::kType },
		{ "namespace", Token::Type::kNamespace },
		{ "import", Token::Type::kImport }
	};

	auto begin = m_iter;
	auto col = m_column;

	while (true) {
		switch (next()) {
			default:
				continue;

			case ';':
			case '.':
			case ',':
			case ':':
			case '(':
			case ')':
			case '{':
			case '}':
			case '[':
			case ']':
			case '^':
			case '~':
			case '|':
			case '&':
			case '+':
			case '-':
			case '*':
			case '/':
			case '%':
			case '=':
			case '>':
			case '<':
			case '!':
			case ' ':
			case '@':
			case '\t':
			case '\n':
			case '\r':
			case '\f':
			case '\v':
			case '\'':
			case '"':
			case '\\':
			case '\b':
			case '\0':
		}

		break;
	}

	lsd::StringView value(begin, m_iter);

	if (auto it = lookup.find(value); it != lookup.end())
		return Token(it->second, value, m_line, col);

	return Token(Token::Type::identifier, value, m_line, col);
}

void Lexer::verifyEscapeSequence() {
	switch (next()) {
		default:
			throwSyntaxError(error::Message::invalidEscape);

		case '\0':
			throwSyntaxError(error::Message::expectedDifferent, '\'');

		case 'x':
		case 'u':
		case 'U':
			if (std::isxdigit(next())) {
				return;
			}

		case 'b':
		case 'e':
		case 'f':
		case 'n':
		case 'r':
		case 't':
		case 'v':
		case '\\':
		case '\'':
		case '"':
			return;
	}
}


void Lexer::skipEmpty() {
	size_type blockCommentMode = 0;

	char c = *m_iter;
	do {
		switch (c) {
			case '\n':
				++m_line;
				m_column = -1;

				break;

			case '\r': case '\f': case '\v': case ' ': case '\t':
				break;

			case '*':
				if (blockCommentMode) {
					auto it = m_iter + 1;
					if (it == m_source.end()) break;

					if (*it == '/') {
						next();

						--blockCommentMode;
					}

					break;
				}

				return;

			case '/': {
				auto it = m_iter + 1;
				if (it == m_source.end()) break;

				if (c = *it; c == '/') {
					++m_line;
					m_column = -1;

					for (m_iter += 2; m_iter != m_source.end() && *m_iter != '\n'; m_iter++)
						;

					break;
				} else if (c == '*') {
					blockCommentMode += 1;

					break;
				}
			}

			default:
				if (blockCommentMode) break;

				return;
		}
	} while ((c = next()) != '\0');

	if (blockCommentMode != 0)
		throwSyntaxError(error::Message::unclosedBlockComment);
}

char Lexer::next() {
	if (++m_iter == m_source.end()) return '\0';

	if (*m_iter == '\0') {
		throwSyntaxError(error::Message::disallowedChar);

		return '\0';
	}

	++m_column;
	return *m_iter;
}

lsd::String Lexer::currentLine(std::size_t& additionalSpaces) {
	lsd::String str;

	auto it = m_iter;
	auto begin = it - 1;
	do {
		--begin;
		
		if (*begin == '\n')
			break;

		it = begin;
	} while (it != m_source.begin());

	for (auto c = *it; it >= m_source.end() && c != '\n'; ++it, c = *it) {
		switch (c) {
			case '\t':
				str.append(' ', 4);
				additionalSpaces += 3;

			case '\r':
			case '\f':
			case '\v':
				break;

			default:
				str.pushBack(c);
		}
	}

	return str;
}

} // namespace compiler

} // namespace elyrium
