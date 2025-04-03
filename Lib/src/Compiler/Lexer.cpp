#include <Elyrium/Compiler/Lexer.hpp>

#include <Elyrium/Core/Error.hpp>

#include <LSD/UnorderedSparseMap.h>


#define REPORT_INV_NUM { \
	error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::invalidNumericLiteral); \
	m_good = false; \
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

Token Lexer::nextToken() {
	if (m_iter == m_source.end()) return Token(Token::Type::eof);

	if (!m_good || !skipEmpty())
		return Token();

	switch (*m_iter) {
		// Strings
		case '"': {
			bool finishedParsing = false;
			auto line = m_line, col = m_column;

			auto begin = m_iter;

			// Skip the quotation mark
			auto c = next();

			while ((c = next()) != '\0') {
				switch (c) {
					case '"':
						finishedParsing = true;

						// Skip the last quotation mark
						m_column += 2;

						continue;

					case '\n':
						m_column = 0;
						++m_line;

						continue;

					case '\t':
						m_column += 4;

						continue;

					case '\\':
						if (!verifyEscapeSequence()) break;

					default:
						m_column += 1;
				}

				break;
			}

			if (!finishedParsing) {
				error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unclosedStringQuotes);
				m_good = false;

				break;
			}

			return Token(Token::Type::string, { begin, m_iter++ }, line, col); // m_iter++ skips the last quotation mark which couldn't be skipped above
		}


		// Single characters
		case '\'': {
			auto col = m_column;
			auto c = next();
			auto begin = m_iter;

			switch (c) {
				case '\0':
					error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unclosedCharQuotes);
					m_good = false;

					break;

				case '\'':
					error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::emptyChar);
					m_good = false;

					break;

				case '\n':
				case '\t':
					error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unescapedChar);
					m_good = false;

					break;

				case '\\':
					if (!verifyEscapeSequence()) break;

				default:
					if (auto c = next(); c == '\0' || c == '\'') {
						error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unclosedCharQuotes);
						m_good = false;

						break;
					}

					return Token(Token::Type::character, { begin, m_iter++ }, m_line, col);
			}

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
			return singleCharEqualTok(Token::Type::bitAnd, Token::Type::assignBitwAnd);

		case '~':
			return singleCharEqualTok(Token::Type::bitNot, Token::Type::assignBitwNot);

		case '^':
			return singleCharEqualTok(Token::Type::bitXor, Token::Type::assignBitwXor);


		// More complicated cases

		case '+':
			return singleCharEqualOrDoubleCharTok(Token::Type::add, Token::Type::increment, Token::Type::assignAdd);

		case '-':
			return singleCharEqualOrDoubleCharTok(Token::Type::sub, Token::Type::decrement, Token::Type::assignSub);

		case '<':
			return singleCharEqualOrDoubleCharEqualTok(Token::Type::less, Token::Type::shiftLeft, Token::Type::lessEqual, Token::Type::assignShiftLeft);

		case '>':
			return singleCharEqualOrDoubleCharEqualTok(Token::Type::greater, Token::Type::shiftRight, Token::Type::lessEqual, Token::Type::assignShiftRight);


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
	auto col = m_column++;

	if (auto c = *m_iter, n = next(); n != '\0' && n == c) {
		next();
		
		return Token(doubleChar, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}

Token Lexer::singleCharEqualTok(Token::Type singleChar, Token::Type equal) {
	auto begin = m_iter;
	auto col = m_column++;

	if (auto n = next(); n != '\0' && n == '=') {
		next();

		return Token(equal, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}

Token Lexer::singleCharEqualOrDoubleCharTok(Token::Type singleChar, Token::Type doubleChar, Token::Type equal) {
	auto begin = m_iter;
	auto col = m_column++;

	if (auto c = *m_iter, n = next(); n != '\0' && n == c)
		return Token(doubleChar, { begin, m_iter }, m_line, col);
	else if (n == '=') {
		next();

		return Token(equal, { begin, m_iter }, m_line, col);
	}

	return Token(singleChar, { begin, m_iter }, m_line, col);
}

Token Lexer::singleCharEqualOrDoubleCharEqualTok(Token::Type singleChar, Token::Type doubleChar, Token::Type singleEqual, Token::Type doubleEqual) {
	auto begin = m_iter;
	auto col = m_column++;

	if (auto c = *m_iter, n = next(); n != '\0' && n == c) {
		if (auto n = next(); n != '\0' && n == '=') {
			next();

			return Token(doubleEqual, { begin, m_iter }, m_line, col);
		}

		return Token(doubleChar, { begin, m_iter }, m_line, col);
	} else if (n == '=') {
		next();

		return Token(singleEqual, { begin, m_iter }, m_line, col);
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
	static const lsd::UnorderedSparseMap<lsd::StringView, Token::Type> lookup {
		{ "null", Token::Type::kNull },
		{ "move", Token::Type::kMove },
		{ "true", Token::Type::kTrue },
		{ "false", Token::Type::kFalse },
		{ "if", Token::Type::kIf },
		{ "else", Token::Type::kElse },
		{ "for", Token::Type::kFor },
		{ "while", Token::Type::kWhile },
		{ "do", Token::Type::kDo },
		{ "return", Token::Type::kReturn },
		{ "yield", Token::Type::kYield },
		{ "raise", Token::Type::kRaise },
		{ "try", Token::Type::kTry },
		{ "catch", Token::Type::kCatch },
		{ "this", Token::Type::kThis },
		{ "let", Token::Type::kLet },
		{ "const", Token::Type::kConst },
		{ "global", Token::Type::kGlobal },
		{ "ptr", Token::Type::kPtr },
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

bool Lexer::verifyEscapeSequence() {
	switch (next()) {
		default:
			error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::invalidEscape);

			return m_good = false;

		case '\0':
			error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unclosedCharQuotes);

			return m_good = false;

		case 'x':
		case 'u':
		case 'U':
			if (std::isxdigit(next())) {
				return true;
			}

			return m_good = false;

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
			return true;
	}
}


bool Lexer::skipEmpty() {
	bool blockCommentMode = false;

	char c = *m_iter;
	do {
		switch (c) {
			case '\n':
				++m_line;
				m_column = 0;

				break;

			case '\t':
				m_column += 3; // Add extra spaces to the column

			case '\r': case '\f': case '\v': case ' ':
				break;

			case '/': {
				if (blockCommentMode) break;

				auto it = m_iter + 1;
				if (it == m_source.end()) break;

				if (c = *it; c == '/') {
					++m_line;
					m_column = 0;

					for (m_iter += 2; m_iter != m_source.end() && *m_iter != '\n'; m_iter++)
						;

					break;
				} else if (c == '*') {
					next();
					blockCommentMode = true;

					break;
				}

				return true;
			}

			case '*':
				if (blockCommentMode) {
					auto it = m_iter + 1;
					if (it == m_source.end()) break;

					if (*it == '/') {
						next();
						next();

						blockCommentMode = false;
					}

					break;
				}

				return true;

			default:
				if (blockCommentMode) break;

				return true;
		}
	} while ((c = next()) != '\0');

	if (blockCommentMode) {
		error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::unclosedBlockComment);
		return m_good = false;
	}

	return true;
}

char Lexer::next() {
	if (++m_iter == m_source.end()) return '\0';

	if (*m_iter == '\0') {
		error::report(m_path, m_line, m_column, currentLine(), error::Type::syntaxError, error::Message::disallowedChar);
		m_good = false;

		return '\0';
	}

	++m_column;
	return *m_iter;
}

lsd::String Lexer::currentLine() {
	lsd::String str;

	auto it = m_iter - 1;
	while (it != m_source.begin() && *it != '\n') --it;

	for (auto c = *it; it >= m_source.end() && c != '\n'; ++it) {
		switch (c) {
			case '\t':
				str.append(' ', 4);

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
