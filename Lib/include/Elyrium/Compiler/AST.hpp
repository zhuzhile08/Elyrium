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

#include <cassert>
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


// General types and forward declarations

class Statement {
public:
	Statement() = default;
	virtual ~Statement() { };

	virtual void print(int = 0) const = 0;
};

class NullStmt : public Statement {
public:
	void print(int = 0) const;
};

class ExprStmt;
class JumpStmt;
class BlockStmt;
class IfStmt;
class ForStmt;
class TryCatchStmt;

using stmt_ptr = lsd::UniquePointer<Statement>;
using null_stmt_ptr = lsd::UniquePointer<NullStmt>;
using expr_stmt_ptr = lsd::UniquePointer<ExprStmt>;
using jump_stmt_ptr = lsd::UniquePointer<JumpStmt>;
using block_stmt_ptr = lsd::UniquePointer<BlockStmt>;
using if_stmt_ptr = lsd::UniquePointer<IfStmt>;
using for_stmt_ptr = lsd::UniquePointer<ForStmt>;
using try_catch_stmt_ptr = lsd::UniquePointer<TryCatchStmt>;


class Declaration : public Statement { };
class ObjectDecl : public Declaration { };

class NullDecl : public Declaration {
public:
	void print(int = 0) const;
};

class NamespaceDecl;
class ImportDecl;
class VariableDecl;
class FunctionDecl;
class SpecialFunctionDecl;	
class CoroutineDecl;
class ClassDecl;
class EnumDecl;

using decl_ptr = lsd::UniquePointer<Declaration>;
using obj_decl_ptr = lsd::UniquePointer<ObjectDecl>;
using null_decl_ptr = lsd::UniquePointer<NullDecl>;
using namespace_decl_ptr = lsd::UniquePointer<NamespaceDecl>;
using import_decl_ptr = lsd::UniquePointer<ImportDecl>;
using variable_decl_ptr = lsd::UniquePointer<VariableDecl>;
using function_decl_ptr = lsd::UniquePointer<FunctionDecl>;
using special_function_decl_ptr = lsd::UniquePointer<SpecialFunctionDecl>;
using coroutine_decl_ptr = lsd::UniquePointer<CoroutineDecl>;
using class_decl_ptr = lsd::UniquePointer<ClassDecl>;
using enum_decl_ptr = lsd::UniquePointer<EnumDecl>;


class Expression {
public:
	Expression() = default;
	virtual ~Expression() { }

	virtual void bindRight(lsd::UniquePointer<Expression>&&);
	virtual void print(int = 0) const = 0;
};

class AtomicExpr;
class MemberExpr;
class UnaryExpr;
class InfixExpr;
class ClosureExpr;
class CoclosureExpr;
class BlockExpr;
class IfExpr;
class ForExpr;
class TryCatchExpr;

using expr_ptr = lsd::UniquePointer<Expression>;
using atomic_expr_ptr = lsd::UniquePointer<AtomicExpr>;
using member_expr_ptr = lsd::UniquePointer<MemberExpr>;
using unary_expr_ptr = lsd::UniquePointer<UnaryExpr>;
using infix_expr_ptr = lsd::UniquePointer<InfixExpr>;
using closure_expr_ptr = lsd::UniquePointer<ClosureExpr>;
using coclosure_expr_ptr = lsd::UniquePointer<CoclosureExpr>;
using block_expr_ptr = lsd::UniquePointer<BlockExpr>;
using if_expr_ptr = lsd::UniquePointer<IfExpr>;
using for_expr_ptr = lsd::UniquePointer<ForExpr>;
using try_catch_expr = lsd::UniquePointer<TryCatchExpr>;


namespace detail {

// Utility structures

struct TypeIdentifier {
public:
	Token identifier;
	lsd::Vector<TypeIdentifier> generics;

	size_type pointerCount = 0;

	void print(int level = 0) const;
};

using type_ident_ptr = lsd::UniquePointer<TypeIdentifier>;


struct IdentifierDecl {
public:
	Token identifier;
	type_ident_ptr type;
	expr_ptr expression;

	void print(int level = 0) const;
};

struct Attributes {
public:
	lsd::Vector<Token> attributes;

	void print(int level = 0) const;
};

struct IfConstruct {
public:
	stmt_ptr init;
	expr_ptr condition;
	
	void print(int level = 0) const;
};

class ForConstruct {
public:
	stmt_ptr init;
	bool rangeBased = false;

	void print(int level = 0) const;

	expr_ptr& condition() noexcept {
		return m_conditionOrRange;
	}
	expr_ptr& range() noexcept {
		return m_conditionOrRange;
	}

	lsd::Vector<expr_ptr>& loop() noexcept {
		return m_loopOrItems;
	}
	lsd::Vector<expr_ptr>& items() noexcept {
		return m_loopOrItems;
	}

private:
	expr_ptr m_conditionOrRange;
	lsd::Vector<expr_ptr> m_loopOrItems;
};

struct CatchConstruct {
public:
	Token identifier;
	type_ident_ptr type;

	void print(int level = 0) const;
};


// Type aliases

using param_t = lsd::Vector<IdentifierDecl>;
using arg_t = lsd::Vector<expr_ptr>;
using subscript_t = expr_ptr;

using catch_construct_ptr = lsd::UniquePointer<CatchConstruct>;


// Advanced utility classes

class FunctionConstruct {
public:
	detail::param_t parameters;
	detail::type_ident_ptr type;

	void print(int level = 0) const;
};

} // namespace detail


// Module

class Module {
public:
	Module() = default;

	void bindDeclaration(decl_ptr&& decl);
	void print() const;

private:
	lsd::Vector<decl_ptr> m_declarations;
};



// Expressions

class AtomicExpr : public Expression {
public:
	AtomicExpr(const Token& identifier) : m_value(identifier) { }

	void print(int level = 0) const;

private:
	Token m_value;
};

class MemberExpr : public Expression {
public:
	struct NullAccess { };

	MemberExpr(expr_ptr&& expr) : m_value(std::move(expr)) { }

	static expr_ptr simplify(member_expr_ptr&& expr);

	void pushCall(detail::arg_t&& args);
	void pushSubscript(detail::subscript_t&& expr);
	void pushMember(const Token& identifier);

	void print(int level = 0) const;

private:
	expr_ptr m_value;

	lsd::Vector<std::variant<
		detail::arg_t, // Member call
		detail::subscript_t, // Subscript into array
		Token // Member access
		>> m_chain;
};

class UnaryExpr : public Expression {
public:
	UnaryExpr() = default;

	static expr_ptr simplify(unary_expr_ptr&& expr);

	void pushPrefix(const Token& op);
	void bindExpr(expr_ptr&& expr);
	void setPostfix(const Token& op);

	void print(int level = 0) const;

private:
	lsd::Vector<Token> m_prefix;
	expr_ptr m_expr;
	Token m_postfix;
};

class InfixExpr : public Expression {
public:
	InfixExpr(const Token& op, expr_ptr&& expr) : m_left(std::move(expr)), m_operator(op) { }

	void bindRight(expr_ptr&& expr);
	void print(int level = 0) const;

private:
	expr_ptr m_left;
	Token m_operator;
	expr_ptr m_right;
};

class StmtExpr : public Expression {
public:
	StmtExpr(stmt_ptr&& stmt) : m_stmt(std::move(stmt)) { }

	void bindExpr(expr_ptr&& expr);
	void print(int level) const;

private:
	stmt_ptr m_stmt;
	expr_ptr m_expr;
};



// Statements

class ExprStmt : public Statement {
public:
	ExprStmt(expr_ptr&& expr) : m_expr(std::move(expr)) { }

	void print(int level = 0) const;

private:
	expr_ptr m_expr;
};

class JumpStmt : public Statement {
public:
	JumpStmt(const Token& keyword) : m_keyword(keyword) { }

	void bindExpr(expr_ptr&& ptr);
	void print(int level = 0) const;

private:
	Token m_keyword;
	expr_ptr m_expr;
};

class BlockStmt : public Statement {
public:
	BlockStmt() = default;

	void pushStatement(stmt_ptr&& stmt);
	void print(int level = 0) const;

private:
	lsd::Vector<stmt_ptr> m_statements;
};

class IfStmt : public Statement {
public:
	IfStmt(detail::IfConstruct&& construct, stmt_ptr&& stmt) : m_construct(std::move(construct)), m_statement(std::move(stmt)) { }

	void bindElseStatement(stmt_ptr&& stmt);
	void print(int level = 0) const;

private:
	detail::IfConstruct m_construct;
	stmt_ptr m_statement;

	stmt_ptr m_chain;
};

class ForStmt : public Statement {
public:
	ForStmt(detail::ForConstruct&& construct, stmt_ptr&& stmt) : 
		m_construct(std::move(construct)), m_statement(std::move(stmt)), m_doBlock(false) { }
	ForStmt(stmt_ptr&& stmt, detail::ForConstruct&& construct) : 
		m_construct(std::move(construct)), m_statement(std::move(stmt)), m_doBlock(false) { }

	void print(int level = 0) const;

private:
	detail::ForConstruct m_construct;
	stmt_ptr m_statement;

	bool m_doBlock;
};

class TryCatchStmt : public Statement {
public:
	TryCatchStmt(stmt_ptr&& stmt) : m_tryBlock(std::move(stmt)) { }

	void bindCatchBlock(stmt_ptr&& stmt, detail::catch_construct_ptr&& construct = nullptr);

	void print(int level = 0) const;

private:
	stmt_ptr m_tryBlock;
	lsd::Vector<std::pair<stmt_ptr, detail::catch_construct_ptr>> m_catchBlocks;
};


// Declarations

class NamespaceDecl : public Declaration {
public:
	NamespaceDecl(const Token& ident) : m_identifier(ident) { }

	void bindDecl(decl_ptr&& decl);
	void print(int level = 0) const;

private:
	Token m_identifier;
	lsd::Vector<decl_ptr> m_declarations;
};

class ImportDecl : public Declaration {
public:
	ImportDecl() = default;

	void bindModule(const Token& module);
	void print(int level = 0) const;

private:
	lsd::Vector<Token> m_modules;
};

class VariableDecl : public ObjectDecl {
public:
	VariableDecl(detail::Attributes&& attributes) : m_attributes(std::move(attributes)) { }

	void bindDeclaration(detail::IdentifierDecl&& decl);
	void print(int level = 0) const;

private:
	detail::Attributes m_attributes;
	lsd::Vector<detail::IdentifierDecl> m_identifiers;
};

class FunctionDecl : public ObjectDecl {
public:
	FunctionDecl(detail::Attributes&& attributes, const Token& ident) : m_attributes(std::move(attributes)), m_identifier(ident) { }

	void bindConstruct(detail::FunctionConstruct&& construct);
	void bindBody(BlockStmt&& stmt);

	void print(int level = 0) const;

private:
	detail::Attributes m_attributes;
	Token m_identifier;
	detail::FunctionConstruct m_construct;

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

class OperatorFunctionDecl : public ObjectDecl {
public:
	OperatorFunctionDecl(detail::Attributes&& attributes, const Token& op) : m_attributes(std::move(attributes)), m_operator(op) { }

	void bindConstruct(detail::FunctionConstruct&& construct);
	void bindBody(BlockStmt&& stmt);

	void print(int level = 0) const;

private:
	detail::Attributes m_attributes;
	Token m_operator;
	detail::param_t m_parameters;
	BlockStmt m_body;
};

class ClassDecl : public ObjectDecl {
public:
	ClassDecl(detail::Attributes&& attributes, const Token& ident) : m_attributes(std::move(attributes)), m_identifier(ident) { }

	void bindDecl(decl_ptr&& decl);
	void print(int level = 0) const;

private:
	detail::Attributes m_attributes;
	Token m_identifier;
	lsd::Vector<decl_ptr> m_body;
};

class EnumDecl : public ObjectDecl {
public:
	EnumDecl(detail::Attributes&& attributes, const Token& ident) : m_attributes(std::move(attributes)), m_identifier(ident) { }

	void bindType(detail::type_ident_ptr&& type);
	void bindValue(expr_ptr&& expr);

	void print(int level = 0) const;

private:
	detail::Attributes m_attributes;
	Token m_identifier;
	detail::type_ident_ptr m_type;
	lsd::Vector<expr_ptr> m_values;
};


// Special expressions

class ClosureExpr : public Expression {
public:
	ClosureExpr() = default;

	void bindCaptureExpression(expr_ptr&& expr);
	void bindConstruct(detail::FunctionConstruct&& construct);
	void bindBody(BlockStmt&& stmt);

	void print(int level = 0) const;

private:
	lsd::Vector<expr_ptr> m_captures;
	detail::FunctionConstruct m_construct;

	BlockStmt m_body;
};

} // namespace ast

} // namespace compiler

} // namespace elyrium
