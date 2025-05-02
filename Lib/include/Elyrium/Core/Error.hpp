/*************************
 * @file Error.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Error and exception handling classes
 * 
 * @date 2024-06-09
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Elyrium/Core/Common.hpp>
#include <Elyrium/Core/File.hpp>
#include <exception>

namespace elyrium {

namespace error {

enum class Message {
	// Syntax errors

	invalidSyntax,
	invalidNumericLiteral,
	disallowedChar,
	unescapedChar,
	invalidEscape,
	emptyChar,
	unclosedCharQuotes,
	unclosedStringQuotes,
	unclosedParen,
	unclosedBrace,
	unclosedBracket,
	unclosedBlockComment,
	expectedExpression,
	expectedIdentifier,
};

} // namespace error


// Native program internal exception

class ElyriumError : public std::exception {
public:
	ElyriumError() = default;
	ElyriumError(lsd::String&& message) {
		m_message.append(message).pushBack('!');
	}
	ElyriumError(const ElyriumError&) = default;
	ElyriumError(ElyriumError&&) = default;

	ElyriumError& operator=(const ElyriumError&) = default;
	ElyriumError& operator=(ElyriumError&&) = default;

	const char* what() const noexcept override {
		return m_message.cStr();
	}

protected:
	lsd::String m_message;
};


// Syntax errors

class SyntaxError : public ElyriumError {
public:
	SyntaxError(
		lsd::StringView fileName, 
		size_type line,
		size_type column,
		lsd::StringView lineSource, 
		error::Message message);
};

} // namespace elyrium
