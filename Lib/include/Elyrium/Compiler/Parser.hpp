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

namespace elyrium {

namespace compiler {

class Parser {
private:
	enum class Precedence {
		assign,
		assignLeft = assign,
		assignRight,
		logicOr,
		logicAnd,
		bitOr,
		bitXOr,
		bitAnd,
		equality,
		comparison,
		spaceship,
		shift,
		term,
		factor,
		unary,
		member,
		atomic,
	};

public:
	Parser(lsd::StringView source, lsd::StringView path);

	ast::expr_ptr parse() {
		return parseExpression();
	}

	[[nodiscard]] const Token& current() const noexcept {
		return m_current;
	}
	Token& next() {
		return (m_current = m_lexer.nextToken());
	}

private:
	lsd::StringView m_path;
	lsd::StringView m_source;

	Lexer m_lexer;
	Token m_current;


	// Expression parsers

	ast::expr_ptr parseExpression(Precedence precedence = Precedence::atomic, ast::expr_ptr&& expr = nullptr);
	ast::expr_ptr parseUnaryExpression();
	ast::expr_ptr parseAtomicMemberExpression();
	ast::infix_ptr parseInfixExpression(ast::expr_ptr&& rightExpr);


	// Statement parsers
	
	ast::expr_ptr parseStatement();
};

} // namespace compiler

} // namespace elyrium
