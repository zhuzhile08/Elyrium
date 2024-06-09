#include <Interpreter/Lexer.h>

namespace elyrium {

Lexer::Lexer(const std::string& source) : m_source(source), m_iter(m_source.begin()) {
	while (m_iter < m_source.end()) parseToken();
}

std::string Lexer::stringifyTokens() const {
	std::string val;

	for (const auto& tok : m_tokens) {
		val.append(tok.stringify()).append(", ");
	}

	val.erase(val.end() - 2, val.end());

	return val;
}

size_type Lexer::singleCharTok() {
	++m_colNum;
	++m_iter;

	return 1;
}

size_type Lexer::duplicateCharTok(Token::Type& type, Token::Type singleChar, Token::Type doubleChar) {
	++m_colNum;

	if (*m_iter == *++m_iter) {
		++m_colNum;
		++m_iter;

		type = doubleChar;

		return 2;
	} else {
		type = singleChar;

		return 1;
	}
}

size_type Lexer::appendEqualCharTok(Token::Type& type, Token::Type singleChar, Token::Type appendEqual) {
	++m_colNum;

	if (*++m_iter == '=') {
		++m_colNum;
		++m_iter;

		type = appendEqual;

		return 2;
	} else {
		type = singleChar;

		return 1;
	}
}

size_type Lexer::duplicateAppendEqualCharTok(Token::Type& type, Token::Type singleChar, Token::Type doubleChar, Token::Type appendEqual) {
	++m_colNum;

	if (*m_iter == *++m_iter) {
		++m_colNum;
		++m_iter;

		type = doubleChar;

		return 2;
	} else if (*m_iter == '=') {
		++m_colNum;
		++m_iter;

		type = appendEqual;

		return 2;
	} else {
		type = singleChar;

		return 1;
	}
}

void Lexer::skipEmpty() {
	for (; m_iter != m_source.end(); m_iter++) { // assumes m_iter starts at an empty character
		switch (*m_iter) {
			case '\n':
				++m_rowNum;
			case '\r':
				m_colNum = 0;
				break;

			case '\t':
				m_colNum += 4;
				break;

			case ' ':
			case '\0':
				++m_colNum;
				break;

			default:
				return; // m_iter is at a non-empty character
		}
	}
}

void Lexer::parseToken() { // expects m_iter to have already started at the next token or empty character
	skipEmpty();

	Token::Type type;

	size_type col = m_colNum, row = m_rowNum;
	size_type pos = m_iter - m_source.begin(), len = 0;

	bool finishedParsing = false; // only needed for string or identifier parsing

	switch (*m_iter) {
		// strings

		case '\"': 
			++m_iter; // m_iter is now at the first character of the string
			pos = m_iter - m_source.begin(); // recalculate position

			for (; m_iter != m_source.end(); m_iter++, m_colNum++, len++) {
				if (*m_iter == '\"') {
					++m_iter; // skip the last quotation mark
					++m_colNum;

					break;
				}
			}

			type = Token::Type::string;

			break;


		// single characters

		case '\'': 
			type = Token::Type::character;

			// assert that the next character is not a '
			len = 1;

			++(++m_iter); // skip to the start of the next token / empty character

			break;


		// numbers

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			++len; // add one to all because we already know that the first character is a number
			++m_iter;
			++m_colNum;

			for (; m_iter != m_source.end() && !finishedParsing; m_iter++, m_colNum++, len++) {
				switch (*m_iter) {
					case '.':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case 'A':
					case 'a':
					case 'B':
					case 'b':
					case 'C':
					case 'c':
					case 'D':
					case 'd':
					case 'E':
					case 'e':
					case 'F':
					case 'f':
						break;

					default:
						finishedParsing = true;

						break;
				}
			}

			type = Token::Type::number;

			--len; // the increments in the for loop will still get evaluated, so everything has to be set back by one
			--m_colNum;
			--m_iter;

			break;
		

		// single character symbols

		case ';':
			type = Token::Type::eol;
			len = singleCharTok();

			break;
		case '.':
			type = Token::Type::dot;
			len = singleCharTok();

			break;
		case ',':
			type = Token::Type::comma;
			len = singleCharTok();

			break;
		case '(':
			type = Token::Type::parenLeft;
			len = singleCharTok();

			break;
		case ')':
			type = Token::Type::parenRight;
			len = singleCharTok();

			break;
		case '{':
			type = Token::Type::braceLeft;
			len = singleCharTok();

			break;
		case '}':
			type = Token::Type::braceRight;
			len = singleCharTok();

			break;
		case '[':
			type = Token::Type::bracketLeft;
			len = singleCharTok();

			break;
		case ']':
			type = Token::Type::bracketRight;
			len = singleCharTok();

			break;


		// possible duplicate symbols

		case ':': 
			len = duplicateCharTok(type, Token::Type::colon, Token::Type::colonColon);

			break;
		case '=':
			len = duplicateCharTok(type, Token::Type::equal, Token::Type::equalEqual);

			break;
		case '|':
			len = duplicateCharTok(type, Token::Type::bitwOr, Token::Type::logicOr);

			break;
		

		// possible appended equal symbols

		case '%':
			len = appendEqualCharTok(type, Token::Type::mod, Token::Type::equalMod);
			
			break;
		case '!':
			len = appendEqualCharTok(type, Token::Type::logicNot, Token::Type::equalLogicNot);
			
			break;
		case '&':
			len = appendEqualCharTok(type, Token::Type::bitwAnd, Token::Type::equalBitwAnd);
			
			break;
		case '~':
			len = appendEqualCharTok(type, Token::Type::bitwNot, Token::Type::equalBitwNot);
			
			break;
		case '^':
			len = appendEqualCharTok(type, Token::Type::bitwXor, Token::Type::equalBitwXor);
			
			break;


		// possible duplicate / appended equal symbols

		case '+': 
			len = duplicateAppendEqualCharTok(type, Token::Type::add, Token::Type::increment, Token::Type::equalAdd);
			
			break;
		case '-':
			len = duplicateAppendEqualCharTok(type, Token::Type::sub, Token::Type::decrement, Token::Type::equalSub);
			
			break;
		case '<':
			len = duplicateAppendEqualCharTok(type, Token::Type::less, Token::Type::shiftLeft, Token::Type::equalLess);
			
			break;
		case '>':
			len = duplicateAppendEqualCharTok(type, Token::Type::greater, Token::Type::shiftRight, Token::Type::equalGreater);
			
			break;

		
		// special comment block comment cases

		case '/': 
			if (*(m_iter + 1) == '*') {
				type = Token::Type::commentBlockLeft;
				len = 2;

				++(++m_iter);
			} else len = appendEqualCharTok(type, Token::Type::div, Token::Type::equalDiv);
			
			break;
		case '*':
			if (*(m_iter + 1) == '/') {
				type = Token::Type::commentBlockRight;
				len = 2;

				++(++m_iter);
			} else len = duplicateAppendEqualCharTok(type, Token::Type::asterisk, Token::Type::rvalue, Token::Type::equalMul);
			
			break;
		

		// identifiers and other tokens

		default: 
			++len; // add one to all because we already know that the first character is a number
			++m_iter;
			++m_colNum;

			for (; m_iter != m_source.end() && !finishedParsing; m_iter++, m_colNum++, len++) {
				switch (*m_iter) {
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
					case '+':
					case '-':
					case '*':
					case '/':
					case '%':
					case '=':
					case '<':
					case '>':
					case '!':
					case '&':
					case '~':
					case '^':
					case '|':
					case '\\':
					case '\n':
					case '\r':
					case '\t':
					case ' ':
					case '\0':
						finishedParsing = true;

						break;
					
					default:
						break;
				}
			}

			type = Token::Type::none;

			--len; // the increments in the for loop will still get evaluated, so everything has to be set back by one
			--m_colNum;
			--m_iter;

			break;
	}
	
	m_tokens.emplaceBack(type, std::string_view(m_source).substr(pos, len), col, row);
}

} // namespace elyrium
