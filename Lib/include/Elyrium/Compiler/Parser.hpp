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

#include <Elyrium/Core/Error.hpp>

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

	ast::Module parse();

private:
	lsd::StringView m_path;
	lsd::StringView m_source;

	Lexer m_lexer;
	Token m_current;
	Token m_last;

	Token& next();
	void consume(bool type, error::Message message, char expected = '\0');
	void verify(bool type, error::Message message, char expected = '\0') const;

	// Expression parser

	ast::expr_ptr parseExpression(Precedence precedence = Precedence::atomic, ast::expr_ptr&& expr = nullptr);
	ast::expr_ptr parseUnaryExpression();
	ast::expr_ptr parseAtomicMemberExpression();
	ast::infix_expr_ptr parseInfixExpression(ast::expr_ptr&& rightExpr);

	ast::expr_ptr parseClosure();

	// Statement parsers
	
	ast::stmt_ptr parseStatement();
	ast::stmt_ptr parseStatementAndObjDeclaration();
	ast::stmt_ptr parseExprStatement();
	ast::stmt_ptr parseIfStatement();
	ast::stmt_ptr parseForStatement();
	ast::stmt_ptr parseTryCatchStatement();
	
	// Declaration parsers
	
	ast::decl_ptr parseDeclaration();
	ast::decl_ptr parseNamespaceDeclaration();
	ast::decl_ptr parseImportDeclaration();
	ast::obj_decl_ptr parseObjectDeclaration();
	ast::obj_decl_ptr parseVariableDeclaration(ast::detail::Attributes&& attributes);
	ast::obj_decl_ptr parseFunctionDeclaration(ast::detail::Attributes&& attributes);
	ast::obj_decl_ptr parseClassDeclaration(ast::detail::Attributes&& attributes);
	ast::obj_decl_ptr parseEnumDeclaration(ast::detail::Attributes&& attributes);

	// Base parsers

	ast::detail::TypeIdentifier parseTypeIdentifier();
	ast::detail::IdentifierDecl parseIdentifierDeclaration();
	ast::detail::Attributes parseAttributes();
	ast::detail::IfConstruct parseIfConstruct();
	ast::detail::ForConstruct parseForConstruct();
	ast::detail::FunctionConstruct parseFunctionConstruct();

	ast::BlockStmt parseBlockStatement();
	
	bool basicParseObjectDeclaration(ast::obj_decl_ptr& value);
};

} // namespace compiler

} // namespace elyrium
