/*************************
 * @file AST.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Abstract syntax tree and node representation
 * 
 * @date 2025-03-31
 * @copyright Copyright (c) 2025
 *************************/

#pragma once

#include <LSD/Vector.h>
#include <LSD/UniquePointer.h>

#include <Elyrium/Compiler/Token.hpp>

#include <variant>

namespace elyrium {

namespace compiler {

namespace ast {

// Enums

enum class ExprType {
	none = 1 << 0,
	logical = 1 << 0,
	numerical = 1 << 1,
	member = 1 << 2,
	temporary = 1 << 3
};


// General types

class Statement { };
class ObjectDecl : Statement { };
class Expression { };


// Base pointer aliases

using stmt_ptr = lsd::UniquePointer<Statement>;
using decl_ptr = lsd::UniquePointer<ObjectDecl>;
using expr_ptr = lsd::UniquePointer<Expression>;


namespace detail {

// Utility structures

struct TypeIdentifier {
public:
	Token identifier;
	lsd::Vector<TypeIdentifier> generics;
};

struct IdentifierDecl {
public:
	Token identifier;
	lsd::UniquePointer<TypeIdentifier> type;
	lsd::UniquePointer<expr_ptr> expression;
};

struct OptionalStorageIdentifier {
public:
	lsd::UniquePointer<Token> storageClassSpec;
	IdentifierDecl identifier;
};


// Type aliases

using param_t = lsd::Vector<IdentifierDecl>;
using arg_t = lsd::Vector<expr_ptr>;
using subscript_t = expr_ptr;

} // namespace detail


// Expressions

class AtomicExpr : Expression {
public:
	struct NullAccess { };

	AtomicExpr(const Token& value) : m_value(value.data()) { }

	void pushCall(detail::arg_t&& args);
	void pushSubscript(detail::subscript_t&& expr);
	void pushMember(const Token& identifier);

private:
	lsd::StringView m_value;

	lsd::Vector<std::variant<
		detail::arg_t, // Member call
		detail::subscript_t, // Subscript into array
		Token // Member access
		>> m_chain;
};

class UnaryExpr : Expression {
public:


private:
	lsd::Vector<Token::Type> m_prefix;
	expr_ptr m_expr;
	lsd::Vector<Token::Type> m_postfix;
};

class Factor : Expression {
public:


private:
	expr_ptr m_left;
	Token::Type m_operator;
	expr_ptr m_right;
};

class Term : Expression {
public:


private:
	expr_ptr m_left;
	Token::Type m_operator;
	expr_ptr m_right;
};

class LogicalExpr : Expression {
public:


private:
	expr_ptr m_left;
	Token::Type m_operator;
	expr_ptr m_right;
};

class AssignmentExpr : Expression {
public:


private:
	expr_ptr m_left;
	Token::Type m_operator;
	expr_ptr m_right;
};


// Statements

class BlockStmt : Statement {
public:


private:
	lsd::Vector<stmt_ptr> m_statements;
};


// Declarations

class NamespaceDecl { // Not a object declaration, therefore not tagged
public:


private:
	Token m_identifier;
	lsd::Vector<decl_ptr> m_declarations;
};

class TypeAliasDecl : ObjectDecl {
public:


private:
	Token m_alias;
	detail::TypeIdentifier m_type;
};

class VariableDecl : ObjectDecl {
public:


private:
	Token m_storageSpecifier;
	lsd::Vector<detail::IdentifierDecl> m_identifiers;
};

class FunctionDecl : ObjectDecl {
public:


private:
	Token m_identifier;
	detail::param_t m_parameters;
	BlockStmt m_body;
};

/*
class CoroutineDecl : ObjectDecl {
public:


private:
	Token m_identifier;
	detail::param_t m_parameters;
	BlockExpr m_body;
};
*/

class SpecialFunctionDecl : ObjectDecl {
public:


private:
	Token m_identifier;
	detail::param_t m_parameters;
	BlockStmt m_body;
};

class ClassDecl : ObjectDecl {
public:


private:
	Token m_identifier;
	lsd::Vector<decl_ptr> m_body;
};

class EnumDecl : ObjectDecl {
public:
	struct Value {
	public:

	};

private:
	Token m_identifier;
	lsd::Vector<Value> m_values;
};

} // namespace ast

} // namespace compiler

} // namespace elyrium
