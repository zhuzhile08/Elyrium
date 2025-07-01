#include "Elyrium/Compiler/Token.hpp"
#include <Elyrium/Compiler/AST.hpp>
#include <system_error>

#define ELYRIUM_PRINT_INDENT(indent) for (auto i = 0; i < indent; i++) std::printf("    ")
#define ELYRIUM_PRINT_INDENTED_AST(format, indent, ...) \
ELYRIUM_PRINT_INDENT(indent); \
std::printf(format __VA_OPT__(,) __VA_ARGS__)

namespace elyrium {

namespace compiler {

namespace ast {

namespace detail {

// Type identifier

void TypeIdentifier::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Type -> %.*s", level, static_cast<int>(identifier.data().size()), identifier.data().data());

	if (pointerCount > 0)
		std::printf(" | Pointer indirection count -> %zu", pointerCount);
	
	if (++level; !generics.empty()) {
		std::printf(" | Generics:\n");
		
		for (const auto& generic: generics)
			generic.print(level);
	} else std::printf("\n");
}


// Identifier declarations

void IdentifierDecl::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Identifier declaration:\n", level);
	++level;

	ELYRIUM_PRINT_INDENTED_AST("Identifier -> %.*s\n", level, static_cast<int>(identifier.data().size()), identifier.data().data());
	
	if (type)
		type->print(level);

	if (expression) {
		ELYRIUM_PRINT_INDENTED_AST("Expression -> ", level);
		expression->print(level);
	}
}


// Attributes

void Attributes::print(int level) const {
	if (!attributes.empty()) {
		ELYRIUM_PRINT_INDENTED_AST("Attributes -> ", level);

		for (std::size_t i = 0; i < attributes.size(); i++) {
			auto attr = attributes[i].data();
			std::printf("%.*s", static_cast<int>(attr.size()), attr.data());

			if (i != attributes.size() - 1)
				std::putchar(',');
		}

		std::putchar('\n');
	}
}


// For constructs

void ForConstruct::print(int level) const {
	if (init) {
		ELYRIUM_PRINT_INDENTED_AST("Init statement -> ", level);
		init->print(level);
	}

	if (rangeBased) {
		if (!m_loopOrItems.empty()) {
			ELYRIUM_PRINT_INDENTED_AST("Items:\n", level);

			++level;
			for (const auto& loop : m_loopOrItems) {
				ELYRIUM_PRINT_INDENT(level);
				loop->print(level);
			}
			--level;
		}

		ELYRIUM_PRINT_INDENTED_AST("Range -> ", level);
		m_conditionOrRange->print(level);
	} else {
		if (m_conditionOrRange) {
			ELYRIUM_PRINT_INDENTED_AST("Condition -> ", level);
			m_conditionOrRange->print(level);
		}

		if (!m_loopOrItems.empty()) {
			ELYRIUM_PRINT_INDENTED_AST("Loop expression:\n", level);

			++level;
			for (const auto& loop : m_loopOrItems) {
				ELYRIUM_PRINT_INDENT(level);
				loop->print(level);
			}
		}
	}
}

void CatchConstruct::print(int level) const {
	ELYRIUM_PRINT_INDENTED_AST("Catched exception:", level);

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Identifier -> %.*s\n", level, static_cast<int>(identifier.data().size()), identifier.data().data());
	
	if (type) type->print(level);
}


// Function constructs

void FunctionConstruct::print(int level) const {
	if (parameters.size() > 0) {
		ELYRIUM_PRINT_INDENTED_AST("Parameters:\n", level);
		for (const auto& param : parameters)
			param.print(level + 1);
	}

	if (type) type->print(level);
}

} // namespace detail


// Module

void Module::bindDeclaration(decl_ptr&& decl) {
	m_declarations.emplaceBack(std::move(decl));
}

void Module::print() const {
	for (const auto& decl : m_declarations)
		decl->print();
}


// Expressions

void Expression::bindRight(expr_ptr&&) {
	assert(false && "elyrium::compiler::ast::Expression::bindRight(): Attempted to bind an expression to the right of another unbindable expression, aborting!");
}


// Atomic expression

void AtomicExpr::print(int) const {
	std::printf("Atomic expression: [ %.*s | %zu ]\n",
				static_cast<int>(m_value.data().size()),
				m_value.data().data(),
				std::to_underlying<Token::Type>(m_value.type()));
}


// Member expression

expr_ptr MemberExpr::simplify(member_expr_ptr&& expr) {
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
	std::printf("Member expression:\n");

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Base expression -> ", level);
	m_value->print(level);
	
	for (const auto& chain : m_chain) {
		std::visit([level](auto&& element) {
			using Ty = std::decay_t<decltype(element)>;

			auto currLevel = level;
			if constexpr (std::is_same_v<Ty, detail::arg_t>) {
				++currLevel;
				ELYRIUM_PRINT_INDENTED_AST("Function call", currLevel);

				if (!element.empty()) {
					std::printf(" arguments:\n");
					++currLevel;

					for (const auto& arg : element) {
						ELYRIUM_PRINT_INDENT(currLevel);
						arg->print(currLevel);
					}
				} else std::putchar('\n');
			} else if constexpr (std::is_same_v<Ty, detail::subscript_t>) {
				++currLevel;
				ELYRIUM_PRINT_INDENTED_AST("Subscript index -> ", currLevel);
				element->print(currLevel);
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

expr_ptr UnaryExpr::simplify(unary_expr_ptr&& expr) {
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
	std::printf("Unary expression:\n");

	++level;
	for (const auto& op : m_prefix) {
		ELYRIUM_PRINT_INDENTED_AST("Prefix operator: [ %.*s | %zu ]\n",
								   level,
								   static_cast<int>(op.data().size()),
								   op.data().data(),
								   std::to_underlying<Token::Type>(op.type()));
	}
	
	ELYRIUM_PRINT_INDENT(level);
	m_expr->print(level);

	if (m_postfix.type() != Token::Type::none) {
		ELYRIUM_PRINT_INDENTED_AST("Postfix operator: [ %.*s | %zu ]\n",
								   level,
								   static_cast<int>(m_postfix.data().size()),
								   m_postfix.data().data(),
								   std::to_underlying<Token::Type>(m_postfix.type()));
	}
}


// Infix expression

void InfixExpr::bindRight(expr_ptr&& expr) {
	m_right = std::move(expr);
}

void InfixExpr::print(int level) const {
	std::printf("Infix expression:\n");

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Operator: [ %.*s | %zu ]\n",
							   level,
							   static_cast<int>(m_operator.data().size()),
							   m_operator.data().data(),
							   std::to_underlying<Token::Type>(m_operator.type()));
	ELYRIUM_PRINT_INDENTED_AST("Left expression -> ", level);
	m_left->print(level);
	ELYRIUM_PRINT_INDENTED_AST("Right expression -> ", level);
	m_right->print(level);
}



// Statements

void NullStmt::print(int) const {
	std::printf("Null-statement\n");
}


// Expression statements

void ExprStmt::print(int level) const {
	std::printf("Expression statement -> ");
	m_expr->print(level);
}


// Jump statements

void JumpStmt::bindExpr(expr_ptr&& expr) {
	m_expr = std::move(expr);
}

void JumpStmt::print(int level) const {
	std::printf("Jump statement [ %.*s | %zu ]",
				static_cast<int>(m_keyword.data().size()),
				m_keyword.data().data(),
				std::to_underlying<Token::Type>(m_keyword.type()));

	if (m_expr) {
		std::printf(" -> ");
		m_expr->print(level);
	} else std::printf("\n");
}


// Block statements

void BlockStmt::pushStatement(stmt_ptr&& stmt) {
	m_statements.emplaceBack(std::move(stmt));
}

void BlockStmt::print(int level) const {
	std::printf("Block statement:\n");

	++level;
	for (const auto& stmt : m_statements) {
		ELYRIUM_PRINT_INDENT(level);
		stmt->print(level);
	}
}


// If statements

void IfStmt::bindElseStatement(stmt_ptr&& chain) {
	m_chain = std::move(chain);
}

void IfStmt::print(int level) const {
	std::printf("If statement:\n");

	++level;
	if (m_construct.init) {
		ELYRIUM_PRINT_INDENTED_AST("Init statement -> ", level);
		m_construct.init->print(level);
	}

	ELYRIUM_PRINT_INDENTED_AST("Condition -> ", level);
	m_construct.condition->print(level);

	ELYRIUM_PRINT_INDENTED_AST("Statement -> ", level);
	m_statement->print(level);

	if (m_chain) {
		--level;
		ELYRIUM_PRINT_INDENTED_AST("Else-Statement -> ", level);
		m_chain->print(level);
	}
}


// For statement

void ForStmt::print(int level) const {
	std::printf("For statement:\n");
	
	m_construct.print(++level);

	ELYRIUM_PRINT_INDENTED_AST("Statement -> ", level);
	m_statement->print(level);
}


// Try-catch statements

void TryCatchStmt::bindCatchBlock(stmt_ptr&& stmt, detail::catch_construct_ptr&& construct) {
	m_catchBlocks.emplaceBack(std::move(stmt), std::move(construct));
}

void TryCatchStmt::print(int level) const {
	std::printf("Try-catch statement:\n");

	++level;
	ELYRIUM_PRINT_INDENTED_AST("Try-block -> ", level);
	m_tryBlock->print(level);

	ELYRIUM_PRINT_INDENTED_AST("Catch-blocks:\n", level);

	++level;
	for (const auto& block : m_catchBlocks) {
		if (block.second)
			block.second->print(level);
		
		ELYRIUM_PRINT_INDENTED_AST("Statement -> ", level);
		block.first->print(level);
	}
}



// Declarations

void NullDecl::print(int) const {
	std::printf("Null declaration\n");
}


// Namespace declarations

void NamespaceDecl::bindDecl(decl_ptr&& decl) {
	m_declarations.emplaceBack(std::move(decl));
}

void NamespaceDecl::print(int level) const {
	std::printf("Namespace declaration -> %.*s:\n", static_cast<int>(m_identifier.data().size()), m_identifier.data().data());

	++level;
	for (const auto& decl : m_declarations) {
		ELYRIUM_PRINT_INDENT(level);
		decl->print(level);
	}
}


// Import declarations

void ImportDecl::bindModule(const Token& module) {
	m_modules.pushBack(module);
}

void ImportDecl::print(int level) const {
	std::printf("Import declaration:\n");

	++level;
	for (const auto& module : m_modules) {
		ELYRIUM_PRINT_INDENTED_AST("Module name ", level);

		if (module.type() == Token::Type::string)
			std::printf("(string) -> \"%.*s\"\n", static_cast<int>(module.data().size()), module.data().data());
		else std::printf("(variable) -> %.*s\n", static_cast<int>(module.data().size()), module.data().data());
	}
}


// Variable declarations

void VariableDecl::bindDeclaration(detail::IdentifierDecl&& decl) {
	m_identifiers.emplaceBack(std::move(decl));
}

void VariableDecl::print(int level) const {
	std::printf("Variable declaration:\n");

	++level;
	m_attributes.print(level);

	for (const auto& decl : m_identifiers)
		decl.print(level);
}


// Function declarations

void FunctionDecl::bindConstruct(detail::FunctionConstruct&& construct) {
	m_construct = std::move(construct);
}

void FunctionDecl::bindBody(BlockStmt&& body) {
	m_body = std::move(body);
}

void FunctionDecl::print(int level) const {
	std::printf("Function declaration -> %.*s:\n", static_cast<int>(m_identifier.data().size()), m_identifier.data().data());
	
	++level;
	m_attributes.print(level);
	m_construct.print(level);

	ELYRIUM_PRINT_INDENTED_AST("Function body -> ", level);
	m_body.print(level);

}


// Class declarations

void ClassDecl::bindDecl(decl_ptr&& decl) {
	m_body.emplaceBack(std::move(decl));
}

void ClassDecl::print(int level) const {
	std::printf("Class declaration -> %.*s:\n", static_cast<int>(m_identifier.data().size()), m_identifier.data().data());

	++level;
	m_attributes.print(level);

	ELYRIUM_PRINT_INDENTED_AST("Declarations:\n", level);

	++level;
	for (const auto& decl : m_body) {
		ELYRIUM_PRINT_INDENT(level);
		decl->print(level);
	}
}


// Enum declarations

void EnumDecl::bindType(detail::type_ident_ptr&& type) {
	m_type = std::move(type);
}

void EnumDecl::bindValue(expr_ptr&& expr) {
	m_values.emplaceBack(std::move(expr));
}

void EnumDecl::print(int level) const {
	std::printf("Enum declaration -> %.*s:\n", static_cast<int>(m_identifier.data().size()), m_identifier.data().data());

	++level;
	m_attributes.print(level);
	if (m_type) m_type->print(level);

	ELYRIUM_PRINT_INDENTED_AST("Values:\n", level);

	++level;
	for (const auto& value : m_values) {
		ELYRIUM_PRINT_INDENT(level);
		value->print(level);
	}
}



// Special expressions

// Closure expressions

void ClosureExpr::bindCaptureExpression(expr_ptr&& expr) {
	m_captures.emplaceBack(std::move(expr));
}

void ClosureExpr::bindConstruct(detail::FunctionConstruct&& construct) {
	m_construct = std::move(construct);
}

void ClosureExpr::bindBody(BlockStmt&& body) {
	m_body = std::move(body);
}

void ClosureExpr::print(int level) const {
	std::printf("Closure expression:\n");
	
	++level;
	if (m_captures.size() > 0) {
		ELYRIUM_PRINT_INDENTED_AST("Captures:\n", level);
		for (const auto& capture : m_captures)
			capture->print(level + 1);
	}

	m_construct.print();

	ELYRIUM_PRINT_INDENTED_AST("Closure body -> ", level);
	m_body.print(level);
}

} // namespace ast

} // namespace compiler

} // namespace elyrium
