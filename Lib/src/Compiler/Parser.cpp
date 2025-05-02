#include "Elyrium/Compiler/AST.hpp"
#include "Elyrium/Compiler/Token.hpp"
#include <Elyrium/Compiler/Parser.hpp>

#include <Elyrium/Core/Error.hpp>

#include <LSD/UnorderedFlatMap.h>
#include <LSD/UnorderedFlatSet.h>

using namespace lsd::enum_operators;

namespace elyrium {

namespace compiler {

Parser::Parser(lsd::StringView source, lsd::StringView path) :
	m_path(path), m_source(std::move(source)), m_lexer(m_source, m_path), m_current(m_lexer.nextToken())
{

}

ast::expr_ptr Parser::parseExpression(Precedence precedence, ast::expr_ptr&& expr) {
	static const lsd::UnorderedFlatMap<Token::Type, Precedence> infixParsers {
		{ Token::Type::mul, Precedence::factor },
		{ Token::Type::div, Precedence::factor },
		{ Token::Type::mod, Precedence::factor },

		{ Token::Type::add, Precedence::term },
		{ Token::Type::sub, Precedence::term },

		{ Token::Type::shiftLeft, Precedence::shift },
		{ Token::Type::shiftRight, Precedence::shift },

		{ Token::Type::spaceship, Precedence::spaceship },
		{ Token::Type::greater, Precedence::comparison},
		{ Token::Type::less, Precedence::comparison },
		{ Token::Type::greaterEqual, Precedence::comparison },
		{ Token::Type::lessEqual, Precedence::comparison },
		{ Token::Type::equal, Precedence::equality },
		{ Token::Type::notEqual, Precedence::equality },

		{ Token::Type::bitAnd, Precedence::bitAnd },
		{ Token::Type::bitXOr, Precedence::bitXOr },
		{ Token::Type::bitOr, Precedence::bitOr },

		{ Token::Type::logicAnd, Precedence::logicAnd },
		{ Token::Type::logicOr, Precedence::logicOr },

		{ Token::Type::assign, Precedence::assign },
		{ Token::Type::assignMul, Precedence::assign },
		{ Token::Type::assignDiv, Precedence::assign },
		{ Token::Type::assignMod, Precedence::assign },
		{ Token::Type::assignAdd, Precedence::assign },
		{ Token::Type::assignSub, Precedence::assign },
		{ Token::Type::assignShiftLeft, Precedence::assign },
		{ Token::Type::assignShiftRight, Precedence::assign },
		{ Token::Type::assignBitAnd, Precedence::assign },
		{ Token::Type::assignBitXOr, Precedence::assign },
		{ Token::Type::assignBitOr, Precedence::assign },
		{ Token::Type::assignBitNot, Precedence::assign },
	};

	bool grouped = m_current.type() == Token::Type::parenLeft;
	if (grouped) { // When parsing a grouped expression, the precedence should be reset to parse the entire group
		precedence = Precedence::atomic;

		next();
	}

	auto expression = expr ? std::move(expr) : parseUnaryExpression();
	ast::expr_ptr rightExpr { };

	while (m_current.type() != Token::Type::eof) {
		auto parserIt = infixParsers.find(m_current.type());
		if (parserIt == infixParsers.end())
			break;

		if (precedence == Precedence::assignRight) // Simulate right-associativity with assignment
			precedence = Precedence::assignLeft;


		if (auto newPrec = parserIt->second; newPrec <= precedence) { // Continue parsing in cases where binding power decreases
			precedence = newPrec;

			if (rightExpr) expression->bindRight(std::move(rightExpr));

			expression = parseInfixExpression(std::move(expression));
			rightExpr = parseUnaryExpression();
		} else {
			expression->bindRight(parseExpression(newPrec, std::move(rightExpr)));
		}
	}
	
	if (rightExpr) expression->bindRight(std::move(rightExpr));

	if (grouped) {
		if (m_current.type() != Token::Type::parenRight) {
			throw SyntaxError(m_path, m_current.line(), m_current.column(), m_current.lineSource(), error::Message::unclosedParen);

			return nullptr;
		}

		next();
	}

	return ast::expr_ptr(std::move(expression));
}

ast::expr_ptr Parser::parseUnaryExpression() {
	static const lsd::UnorderedFlatSet<Token::Type> prefixOperators {
		Token::Type::add,
		Token::Type::sub,
		Token::Type::increment,
		Token::Type::decrement,
		Token::Type::deref,
		Token::Type::logicNot,
		Token::Type::bitNot,
	};

	static const lsd::UnorderedFlatSet<Token::Type> postfixOperators {
		Token::Type::increment,
		Token::Type::decrement,
	};


	if (m_current.type() == Token::Type::eof) {
		throw SyntaxError(m_path, m_current.line(), m_current.column(), m_current.lineSource(), error::Message::expectedExpression);

		return nullptr;
	}
 

	auto expression = ast::unary_ptr::create();
	while (m_current.type() != Token::Type::eof) {
		if (auto op = prefixOperators.find(m_current.type()); op == prefixOperators.end()) {
			expression->bindExpr(parseAtomicMemberExpression());

			break;
		}

		expression->pushPrefix(m_current);

		next();
	}

	if (auto op = postfixOperators.find(m_current.type()); op != postfixOperators.end()) {
		expression->setPostfix(m_current); // Guarantees right associativity with postfix operators

		next();
	}

	return ast::UnaryExpr::simplify(std::move(expression));
}

ast::expr_ptr Parser::parseAtomicMemberExpression() {
	ast::expr_ptr baseExpr;
	if (m_current.type() != Token::Type::parenLeft) {
		baseExpr = ast::atomic_ptr::create(m_current);

		next();
	} else baseExpr = parseExpression();

	auto expression = ast::member_ptr::create(std::move(baseExpr));

	while (m_current.type() != Token::Type::eof) {
		if (m_current.type() == Token::Type::dot) { // Member access
			if (next().type() != Token::Type::identifier) {
				throw SyntaxError(m_path, m_current.line(), m_current.column(), m_current.lineSource(), error::Message::expectedIdentifier);

				return nullptr;
			}

			expression->pushMember(m_current); 
			next();
		} else if (m_current.type() == Token::Type::parenLeft) { // Function call
			ast::detail::arg_t arguments;

			while (next().type() != Token::Type::parenRight) {
				arguments.emplaceBack(parseExpression());

				if (m_current.type() != Token::Type::comma)
					break;
			}

			if (m_current.type() != Token::Type::parenRight) {
				throw SyntaxError(m_path, m_current.line(), m_current.column(), m_current.lineSource(), error::Message::unclosedParen);

				return nullptr;
			}
			
			expression->pushCall(std::move(arguments));
			next();
		} else if (m_current.type() == Token::Type::bracketLeft) { // Subscript
			next();
			expression->pushSubscript(parseExpression());

			if (m_current.type() != Token::Type::bracketRight) {
				throw SyntaxError(m_path, m_current.line(), m_current.column(), m_current.lineSource(), error::Message::unclosedBracket);

				return nullptr;
			}

			next();
		} else break;
	}

	return ast::MemberExpr::simplify(std::move(expression));
}

ast::infix_ptr Parser::parseInfixExpression(ast::expr_ptr&& rightExpr) {
	auto expr = ast::infix_ptr::create(m_current, std::move(rightExpr));
	next();

	return std::move(expr);
}

} // namespace compiler

} // namespace elyrium
