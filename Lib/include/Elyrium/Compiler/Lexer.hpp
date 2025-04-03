/*************************
 * @file Lexer.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Lexer which turns a string into a series of token
 * 
 * @date 2024-06-08
 * 
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Elyrium/Core/Common.hpp>

#include <Elyrium/Compiler/Token.hpp>

#include <LSD/Vector.h>
#include <LSD/String.h>

namespace elyrium {

namespace compiler {

class Lexer {
public:
	Lexer(lsd::StringView source, lsd::StringView path) : m_path(path), m_source(source), m_iter(m_source.begin()) { }

	Token nextToken();

	[[nodiscard]] bool good() const noexcept {
		return m_good;
	}

private:
	lsd::StringView m_path;

	lsd::StringView m_source;
	lsd::StringView::iterator m_iter;

	size_type m_line { };
	size_type m_column { };

	bool m_good = true;

	Token singleCharTok(Token::Type type);
	Token doubleCharTok(Token::Type singleChar, Token::Type doubleChar);
	Token singleCharEqualTok(Token::Type singleChar, Token::Type equal);
	Token singleCharEqualOrDoubleCharTok(Token::Type singleChar, Token::Type doubleChar, Token::Type equal);
	Token singleCharEqualOrDoubleCharEqualTok(Token::Type singleChar, Token::Type doubleChar, Token::Type singleEqual, Token::Type doubleEqual);

	Token numericLiteral(bool guaranteedFloat);
	Token keywordOrIdentifier();

	bool verifyEscapeSequence();

	bool skipEmpty();
	char next();

	lsd::String currentLine();
};

} // namespace compiler

} // namespace elyrium
