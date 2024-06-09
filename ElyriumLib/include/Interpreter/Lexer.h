/*************************
 * @file Lexer.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Lexer which turns a string into a series of token
 * 
 * @date 2024-06-08
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Common/Common.h>

#include <Interpreter/Token.h>

#include <LSD/Vector.h>

#include <string>

namespace elyrium {

class Lexer {
public:
	Lexer() = default;
	Lexer(const std::string& source);

	std::string stringifyTokens() const;

private:
	std::string m_source;
	std::string::iterator m_iter { };

	size_type m_rowNum { };
	size_type m_colNum { };

	lsd::Vector<Token> m_tokens;

	size_type singleCharTok();
	size_type duplicateCharTok(Token::Type& type, Token::Type singleChar, Token::Type doubleChar);
	size_type appendEqualCharTok(Token::Type& type, Token::Type singleChar, Token::Type appendEqual);
	size_type duplicateAppendEqualCharTok(Token::Type& type, Token::Type singleChar, Token::Type doubleChar, Token::Type appendEqual);

	void skipEmpty();
	void parseToken();
};

} // namespace elyrium
