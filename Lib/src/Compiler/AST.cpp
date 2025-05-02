#include "Elyrium/Compiler/Token.hpp"
#include <Elyrium/Compiler/AST.hpp>

#define ELYRIUM_PRINT_INDENTED_AST(format, indent, ...) \
for (auto i = 0; i < indent; i++) putchar('\t'); \
std::printf(format __VA_OPT__(,) __VA_ARGS__)

namespace elyrium {

namespace compiler {

namespace ast {

void Expression::bindRight(expr_ptr&&) {
	assert(false && "elyrium::compiler::ast::Expression::bindRight(): Attempted to bind an expression to the right of another unbindable expression, aborting!");
}


// Atomic expression

void AtomicExpr::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Atomic expression: [ %.*s | %zu ]\n",
							   static_cast<int>(level),
							   static_cast<int>(m_value.data().size()),
							   m_value.data().data(),
							   std::to_underlying<Token::Type>(m_value.type()));
}


// Member expression

expr_ptr MemberExpr::simplify(member_ptr&& expr) {
	if (expr && expr->m_chain.empty())
		return std::move(expr->m_value);

	return expr_ptr(std::move(expr));
}

void MemberExpr::pushCall(detail::arg_t&& args) {
	m_chain.emplaceBack(std::move(args));
}

void MemberExpr::pushSubscript(detail::subscript_t&& expr) {
	m_chain.emplaceBack(std::move(expr));
}

void MemberExpr::pushMember(const Token& identifier) {
	m_chain.pushBack(identifier);
}

void MemberExpr::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Member expression:\n", level);

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Base expression:\n", level);
	m_value->print(level + 1);
	
	for (const auto& chain : m_chain) {
		std::visit([level](auto&& element) {
			using Ty = std::decay_t<decltype(element)>;

			auto currLevel = level;
			if constexpr (std::is_same_v<Ty, detail::arg_t>) {
				ELYRIUM_PRINT_INDENTED_AST("Function call arguments:\n", currLevel);
				++currLevel;

				for (const auto& arg : element)
					arg->print(currLevel);
			} else if constexpr (std::is_same_v<Ty, detail::subscript_t>) {
				ELYRIUM_PRINT_INDENTED_AST("Subscript index:\n", currLevel);
				element->print(currLevel + 1);
			} else if constexpr (std::is_same_v<Ty, Token>) {
				ELYRIUM_PRINT_INDENTED_AST("Class member: %.*s\n",
										   currLevel,
										   static_cast<int>(element.data().size()),
										   element.data().data());
			} else static_assert("elyrium::compiler::ast::AtomicExpr::print(): Invalid type in chain, aborting!");
		}, chain);
	}
}


// Unary expression

expr_ptr UnaryExpr::simplify(unary_ptr&& expr) {
	if (expr->m_prefix.empty() && expr->m_postfix.type() == Token::Type::none)
		return std::move(expr->m_expr);

	return expr_ptr(std::move(expr));
}

void UnaryExpr::pushPrefix(const Token& op) {
	m_prefix.pushBack(op);
}

void UnaryExpr::bindExpr(expr_ptr&& expr) {
	m_expr = std::move(expr);
}

void UnaryExpr::setPostfix(const Token& op) {
	m_postfix = op;
}

void UnaryExpr::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Unary expression:\n", level);

	for (const auto& op : m_prefix) {
		++level;
		ELYRIUM_PRINT_INDENTED_AST("Prefix operator: [ %.*s | %zu ]\n",
								   level,
								   static_cast<int>(op.data().size()),
								   op.data().data(),
								   std::to_underlying<Token::Type>(op.type()));
	}
	
	m_expr->print(++level);

	if (m_postfix.type() != Token::Type::none) {
		ELYRIUM_PRINT_INDENTED_AST("Postfix operator: [ %.*s | %zu ]\n",
								   level + 1,
								   static_cast<int>(m_postfix.data().size()),
								   m_postfix.data().data(),
								   std::to_underlying<Token::Type>(m_postfix.type()));
	}
}


// Infix expression

InfixExpr::InfixExpr(const Token& op, expr_ptr&& expr) : m_left(std::move(expr)), m_operator(op) { }

void InfixExpr::bindRight(expr_ptr&& expr) {
	m_right = std::move(expr);
}

void InfixExpr::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Infix expression:\n", level);

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Operator: [ %.*s | %zu ]\n",
							   level,
							   static_cast<int>(m_operator.data().size()),
							   m_operator.data().data(),
							   std::to_underlying<Token::Type>(m_operator.type()));
	ELYRIUM_PRINT_INDENTED_AST("Left expression:\n", level);
	m_left->print(level + 1);
	ELYRIUM_PRINT_INDENTED_AST("Right expression:\n", level);
	m_right->print(level + 1);
}

} // namespace ast

} // namespace compiler

} // namespace elyrium
