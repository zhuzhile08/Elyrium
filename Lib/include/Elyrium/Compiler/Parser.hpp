/*************************
 * @file Parser.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Parsing phase of the compiler
 * 
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 *************************/

#pragma once

#include <Elyrium/Compiler/Lexer.hpp>

#include <Elyrium/Compiler/Token.hpp>
#include <Elyrium/Compiler/AST.hpp>

#include <LSD/String.h>

#include <initializer_list>

namespace elyrium {

namespace compiler {

class Parser {
private:
	enum class Precedence {
		base,
		assign = base,
		logicOr,
		logicAnd,
		bitXor,
		bitOr,
		bitAnd,
		equality,
		comparison,
		shift,
		term,
		factor,
		unary,
		member,
		atomic,
		brace
	};

public:
	Parser(lsd::String&& source, lsd::String&& path);

private:
	lsd::String m_path;
	lsd::String m_source;

	Lexer m_lexer;
	Token m_current;

	ast::expr_ptr parseExpression(Precedence precedence = Precedence::base);
};

} // namespace compiler

} // namespace elyrium
