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

#include <cstdio>

namespace elyrium {

namespace error {

enum class Type {
	syntaxError
};

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
};

void report(lsd::StringView filename, size_type lineCount, size_type column, lsd::StringView line, Type type, Message message);

} // namespace error


// Native program internal exception

class ElyriumError : public std::runtime_error {
public:
	ElyriumError(const lsd::String& message) : std::runtime_error(message.data()) {
		m_message.append(message).pushBack('!');
	}
	ElyriumError(const char* message) : std::runtime_error(message) {
		m_message.append(message).pushBack('!');
	}
	ElyriumError(const ElyriumError&) = default;
	ElyriumError(ElyriumError&&) = default;

	ElyriumError& operator=(const ElyriumError&) = default;
	ElyriumError& operator=(ElyriumError&&) = default;

	const char* what() const noexcept override {
		return m_message.cStr();
	}

private:
	lsd::String m_message { "Program terminated with ElyriumError: " };
};


// Syntax errors

class SyntaxError : public ElyriumError {
public:
	SyntaxError(
		lsd::StringView fileName, 
		size_type lineCount, 
		lsd::StringView line, 
		const lsd::String& message);
	SyntaxError(
		lsd::StringView fileName, 
		size_type lineCount, 
		lsd::StringView line, 
		const char* message);
	SyntaxError(const SyntaxError&) = default;
	SyntaxError(SyntaxError&&) = default;

	SyntaxError& operator=(const SyntaxError&) = default;
	SyntaxError& operator=(SyntaxError&&) = default;

private:
	lsd::StringView m_fileName;

	size_type m_lineCount;
	lsd::StringView m_line;
};

} // namespace elyrium
