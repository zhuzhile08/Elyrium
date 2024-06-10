/*************************
 * @file Error.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Error and exception handling classes
 * 
 * @date 2024-06-09
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Core/Common.h>

#include <fmt/core.h>

#include <cstdio>

namespace elyrium {

namespace error {

enum class Type {
	syntaxError
};

enum class Message {
	// synatx errors

	invalidSyntax,
	emptyChar,
	untermChar,
	untermString,
	unclosedParen,
	unclosedBrace,
	unclosedBracket
};

void report(
	std::string_view fileName, 	
	std::string_view text, 
	size_type line,
	size_type column, 
	Type type, 
	Message message
);

} // namespace error

} // namespace elyrium
