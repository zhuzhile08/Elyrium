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

	invalidEscape,
	unescapedChar,
	emptyChar,

	unclosedBlockComment,

	expectedDifferent,
	expectedIdentifier,
	expectedExpression,
	expectedDeclaration,

	noCatchBehindTry,
	importDeclRequiresStrOrConst,
};

} // namespace error


// Native program internal exception

class Exception : public std::exception {
public:
	Exception() = default;
	Exception(lsd::String&& message) {
		m_message.append(message).pushBack('!');
	}
	Exception(const Exception&) = default;
	Exception(Exception&&) = default;

	Exception& operator=(const Exception&) = default;
	Exception& operator=(Exception&&) = default;

	const char* what() const noexcept override {
		return m_message.cStr();
	}

protected:
	lsd::String m_message;
};


// Syntax errors

class SyntaxError : public Exception {
public:
	SyntaxError(
		lsd::StringView fileName, 
		size_type line,
		size_type column,
		lsd::StringView lineSource, 
		error::Message message,
		char expected = '\0');
};

} // namespace elyrium
