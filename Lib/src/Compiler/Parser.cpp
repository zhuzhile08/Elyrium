#include "Elyrium/Compiler/AST.hpp"
#include "Elyrium/Compiler/Token.hpp"
#include "Elyrium/Core/Error.hpp"
#include <Elyrium/Compiler/Parser.hpp>

#include <LSD/UnorderedFlatMap.h>
#include <LSD/UnorderedFlatSet.h>

using namespace lsd::enum_operators;

namespace elyrium {

namespace compiler {

Parser::Parser(lsd::StringView source, lsd::StringView path) :
	m_path(path), m_source(std::move(source)), m_lexer(m_source, m_path), m_current(m_lexer.nextToken()) {
	m_last = m_current;
}

ast::Module Parser::parse() {
	ast::Module module;

	while (m_current.type() != Token::Type::eof)
		module.bindDeclaration(parseDeclaration());

	return module;
}


Token& Parser::next() {
	m_last = m_current;
	return m_current = m_lexer.nextToken();
}

void Parser::consume(bool cond, error::Message message, char expected) {
	verify(cond, message, expected);
	next();
}

void Parser::verify(bool cond, error::Message message, char expected) const {
	if (!cond) {
		std::size_t additionalSpaces { };
		auto source = m_last.lineSource(additionalSpaces);
		throw SyntaxError(m_path, m_last.line(), m_last.column() + additionalSpaces, source, message, expected);
	}
}


// Expressions

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

	if (grouped)
		consume(m_current.type() == Token::Type::parenRight, error::Message::expectedDifferent, ')');

	return expression;
}

ast::expr_ptr Parser::parseUnaryExpression() {
	static constexpr lsd::Array legalTokenTypes = {
		Token::Type::add,
		Token::Type::sub,
		Token::Type::increment,
		Token::Type::decrement,
		Token::Type::deref,
		Token::Type::logicNot,
		Token::Type::bitNot,
		Token::Type::increment,
		Token::Type::decrement,
		Token::Type::identifier,
		Token::Type::integral,
		Token::Type::kTrue,
		Token::Type::kFalse,
		Token::Type::kNull,
		Token::Type::unsignedIntegral,
		Token::Type::character,
		Token::Type::string,
	};

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

	// Check for special expressions first
	switch (m_current.type()) {
		case Token::Type::kFunc:
			parseClosure();

		default:
	}

	bool legal = false;

	for (auto type : legalTokenTypes) {
		if (m_current.type() == type) {
			legal = true;
			break;
		}
	}

	verify(legal, error::Message::expectedExpression);

	auto expression = ast::unary_expr_ptr::create();
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
		baseExpr = ast::atomic_expr_ptr::create(m_current);

		next();
	} else baseExpr = parseExpression();

	auto expression = ast::member_expr_ptr::create(std::move(baseExpr));

	while (m_current.type() != Token::Type::eof) {
		if (m_current.type() == Token::Type::dot) { // Member access
			verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);

			expression->pushMember(m_current); 
		} else if (m_current.type() == Token::Type::parenLeft) { // Function call
			ast::detail::arg_t arguments;

			while (next().type() != Token::Type::parenRight) {
				arguments.emplaceBack(parseExpression());

				if (m_current.type() != Token::Type::comma)
					break;
			}

			verify(m_current.type() == Token::Type::parenRight, error::Message::expectedDifferent, ')');

			expression->pushCall(std::move(arguments));
		} else if (m_current.type() == Token::Type::bracketLeft) { // Subscript
			next();
			expression->pushSubscript(parseExpression());
			
			verify(m_current.type() == Token::Type::bracketRight, error::Message::expectedDifferent, ']');
		} else break;

		next();
	}

	return ast::MemberExpr::simplify(std::move(expression));
}

ast::infix_expr_ptr Parser::parseInfixExpression(ast::expr_ptr&& rightExpr) {
	auto expr = ast::infix_expr_ptr::create(m_current, std::move(rightExpr));
	next();

	return expr;
}

ast::expr_ptr Parser::parseClosure() {
	auto value = ast::closure_expr_ptr::create();

	if (next().type() == Token::Type::braceLeft) {
		while (next().type() != Token::Type::braceRight) {
			value->bindCaptureExpression(parseExpression());
			
			if (m_current.type() != Token::Type::comma)
				break;
		}
	}

	value->bindConstruct(parseFunctionConstruct());
	value->bindBody(parseBlockStatement());

	return value;
}


// Statements

ast::stmt_ptr Parser::parseStatement() {
	switch (m_current.type()) {
		case Token::Type::kIf:
			return parseIfStatement();
		
		case Token::Type::kDo:
		case Token::Type::kFor:
			return parseForStatement();

		case Token::Type::kTry:
			// return parseTryCatchStatement();

		case Token::Type::braceLeft:
			return ast::block_stmt_ptr::create(parseBlockStatement());

		case Token::Type::kYield:
		case Token::Type::kReturn: {
			auto value = ast::jump_stmt_ptr::create(m_current);
			next();
			value->bindExpr(parseExpression());
			
			consume(m_current.type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

			return ast::stmt_ptr(std::move(value));
		}

		case Token::Type::kBreak:
		case Token::Type::kContinue: {
			auto value = ast::jump_stmt_ptr::create(m_current);
			consume(next().type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

			return value;
		}

		default:
			return parseExprStatement();
	}
}

ast::stmt_ptr Parser::parseStatementAndObjDeclaration() {
	auto value = ast::obj_decl_ptr();

	if (!basicParseObjectDeclaration(value))
		return parseStatement();
	
	return value;
}

ast::stmt_ptr Parser::parseExprStatement() {
	if (m_current.type() == Token::Type::semicolon)
		return ast::null_stmt_ptr::create();
	
	auto value = ast::expr_stmt_ptr::create(parseExpression());
	consume(m_current.type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

	return value;

}

ast::stmt_ptr Parser::parseIfStatement() {
	auto value = ast::if_stmt_ptr::create(parseIfConstruct(), parseStatement());

	if (m_current.type() == Token::Type::kElse) {
		if (next().type() == Token::Type::kIf)
			value->bindElseStatement(parseIfStatement());
		else value->bindElseStatement(parseStatement());
	}

	return value;
}

ast::stmt_ptr Parser::parseForStatement() {
	if (m_current.type() == Token::Type::kDo) {
		next();
		auto value = ast::stmt_ptr(ast::for_stmt_ptr::create(parseStatement(), parseForConstruct()));
		consume(m_current.type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

		return value;
	}
		
	return ast::for_stmt_ptr::create(parseForConstruct(), parseStatement());
}

ast::stmt_ptr Parser::parseTryCatchStatement() {
	next();
	auto value = ast::try_catch_stmt_ptr::create(parseStatement());

	consume(m_current.type() == Token::Type::kCatch, error::Message::noCatchBehindTry);
	
	do {
		auto construct = ast::detail::catch_construct_ptr();

		if (m_current.type() == Token::Type::parenLeft) {
			verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);

			construct = ast::detail::catch_construct_ptr::create();
			construct->identifier = m_current;
			
			if (m_current.type() == Token::Type::colon) {
				next();
				construct->type = ast::detail::type_ident_ptr::create(parseTypeIdentifier());
			}
		}

		value->bindCatchBlock(parseStatement(), std::move(construct));
	} while (m_current.type() == Token::Type::kCatch);

	return value;
}


// Declaration parsers

ast::decl_ptr Parser::parseDeclaration() {
	switch (m_current.type()) {
		case Token::Type::semicolon: {
			auto value = ast::null_decl_ptr::create();
			next();

			return value;
		}

		case Token::Type::kNamespace:
			return parseNamespaceDeclaration();

		case Token::Type::kImport:
			return parseImportDeclaration();

		default:
			return parseObjectDeclaration();
	}
}

ast::decl_ptr Parser::parseNamespaceDeclaration() {
	verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);
	auto value = ast::namespace_decl_ptr::create(m_current);

	consume(next().type() == Token::Type::braceLeft, error::Message::expectedDifferent, '{');

	while (m_current.type() != Token::Type::braceRight)
		value->bindDecl(parseDeclaration());

	consume(m_current.type() == Token::Type::braceRight, error::Message::expectedDifferent, '}');

	return value;
}

ast::decl_ptr Parser::parseImportDeclaration() {
	auto value = ast::import_decl_ptr::create();

	while (next().type() != Token::Type::semicolon) {
		verify(m_current.type() == Token::Type::identifier || m_current.type() == Token::Type::string, error::Message::importDeclRequiresStrOrConst);
		value->bindModule(m_current);
		
		if (next().type() != Token::Type::comma)
			break;
	}
	
	consume(m_current.type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

	return value;
}

ast::obj_decl_ptr Parser::parseObjectDeclaration() {
	auto value = ast::obj_decl_ptr();

	verify(basicParseObjectDeclaration(value), error::Message::expectedDeclaration);

	return value;
}

ast::obj_decl_ptr Parser::parseVariableDeclaration(ast::detail::Attributes&& attributes){
	auto value = ast::variable_decl_ptr::create(std::move(attributes));

	while (next().type() != Token::Type::semicolon) {
		value->bindDeclaration(parseIdentifierDeclaration());

		if (m_current.type() != Token::Type::comma)
			break;
	}

	consume(m_current.type() == Token::Type::semicolon, error::Message::expectedDifferent, ';');

	return value;
}

ast::obj_decl_ptr Parser::parseFunctionDeclaration(ast::detail::Attributes&& attributes) {
	verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);
	auto value = ast::function_decl_ptr::create(std::move(attributes), m_current);
	
	next();

	value->bindConstruct(parseFunctionConstruct());
	value->bindBody(parseBlockStatement());

	return value;
}

ast::obj_decl_ptr Parser::parseClassDeclaration(ast::detail::Attributes&& attributes) {
	verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);
	auto value = ast::class_decl_ptr::create(std::move(attributes), m_current);
	
	consume(next().type() == Token::Type::braceLeft, error::Message::expectedDifferent, '{');

	while (m_current.type() != Token::Type::braceRight)
		value->bindDecl(parseObjectDeclaration());
	
	return value;
}

ast::obj_decl_ptr Parser::parseEnumDeclaration(ast::detail::Attributes&& attributes) {
	verify(next().type() == Token::Type::identifier, error::Message::expectedIdentifier);
	auto value = ast::enum_decl_ptr::create(std::move(attributes), m_current);

	if (next().type() == Token::Type::colon) {
		next();
		value->bindType(ast::detail::type_ident_ptr::create(parseTypeIdentifier()));
	}
	
	consume(m_current.type() == Token::Type::braceLeft, error::Message::expectedDifferent, '{');

	while (m_current.type() != Token::Type::braceRight) {
		value->bindValue(parseExpression());

		if (m_current.type() != Token::Type::comma)
			break;
	}

	consume(m_current.type() == Token::Type::braceRight, error::Message::expectedDifferent, '}');
	
	return value;
}


// Base parsers

ast::detail::TypeIdentifier Parser::parseTypeIdentifier() {
	auto value = ast::detail::TypeIdentifier();

	if (m_current.type() == Token::Type::identifier) {
		value.identifier = m_current;
		next();

		if (m_current.type() == Token::Type::bracketLeft) {
			while (next().type() != Token::Type::bracketRight) {
				value.generics.emplaceBack(parseTypeIdentifier());

				if (m_current.type() != Token::Type::comma)
					break;
			}

			consume(m_current.type() == Token::Type::bracketRight, error::Message::expectedDifferent, ']');
		}
	}

	while (m_current.type() == Token::Type::pointer) {
		++value.pointerCount;
		next();
	}

	if (value.identifier.type() == Token::Type::none) verify(value.pointerCount, error::Message::expectedIdentifier);

	return value;
}

ast::detail::IdentifierDecl Parser::parseIdentifierDeclaration() {
	verify(m_current.type() == Token::Type::identifier, error::Message::expectedIdentifier);
	
	auto value = ast::detail::IdentifierDecl();
	value.identifier = m_current;
	next();

	if (m_current.type() == Token::Type::colon) {
		next();
		value.type = ast::detail::type_ident_ptr::create(parseTypeIdentifier());
	}
	
	if (m_current.type() == Token::Type::assign) {
		next();
		value.expression = parseExpression();
	}

	return value;
}

ast::detail::Attributes Parser::parseAttributes() {
	auto value = ast::detail::Attributes();

	do value.attributes.pushBack(m_current);
	while (next().type() == Token::Type::attribute);

	return value;
}

ast::detail::IfConstruct Parser::parseIfConstruct() {
	consume(next().type() == Token::Type::parenLeft, error::Message::expectedDifferent, '(');
	auto value = ast::detail::IfConstruct();

	if (m_current.type() == Token::Type::attribute || m_current.type() == Token::Type::kLet) {
		value.init = parseVariableDeclaration((m_current.type() == Token::Type::attribute) ? parseAttributes() : ast::detail::Attributes());
		value.condition = parseExpression();
	} else {
		auto expr = parseExpression();
		
		if (m_current.type() == Token::Type::semicolon) {
			next();

			value.init = ast::expr_stmt_ptr::create(std::move(expr));
			value.condition = parseExpression();
		} else value.condition = std::move(expr);
	}

	consume(m_current.type() == Token::Type::parenRight, error::Message::expectedDifferent, ')');

	return value;
}

ast::detail::ForConstruct Parser::parseForConstruct() {
	consume(next().type() == Token::Type::parenLeft, error::Message::expectedDifferent, '(');
	auto value = ast::detail::ForConstruct();

	auto expr = ast::expr_ptr();

	if (m_current.type() == Token::Type::attribute || m_current.type() == Token::Type::kLet)
		value.init = parseVariableDeclaration((m_current.type() == Token::Type::attribute) ? parseAttributes() : ast::detail::Attributes());
	else if (m_current.type() == Token::Type::semicolon) {
		value.init = ast::null_stmt_ptr::create();
		if (next().type() == Token::Type::semicolon) {
			next();
			goto forLoopCheckedEnd;
		}
	}

	expr = parseExpression();

	if (m_current.type() == Token::Type::semicolon) {
		next();

		if (value.init) { // The above semicolon is the second semicolon, as the init statement is a variable declaration
			value.condition() = std::move(expr);

			if (m_current.type() == Token::Type::parenRight) {
				next();
				goto forLoopUncheckedEnd;
			} else value.loop().emplaceBack(parseExpression());
		} else { // The above semicolon is the first encountered
			value.init = ast::expr_stmt_ptr::create(std::move(expr)); // If a expression was before the first semicolon, it is always the init statement
			expr = parseExpression(); // This is either the condition, the first item of a range loop or the first expression in the loop expressions

			if (m_current.type() == Token::Type::semicolon) { // This semicolon is the second
				value.condition() = std::move(expr);

				next();
				value.loop().emplaceBack(parseExpression()); // First expression in the loop expressions
			} else value.loop().emplaceBack(std::move(expr));
		}
	}

	while (m_current.type() == Token::Type::comma) { // This is either the loop expression or the items of a range based loop
		if (next(); m_current.type() == Token::Type::colon || m_current.type() == Token::Type::parenRight)
			break;

		value.loop().emplaceBack(parseExpression());
	}

	if (m_current.type() == Token::Type::colon && !value.condition()) { // This handles the range of a range based loop
		next();

		value.rangeBased = true;
		value.range() = parseExpression();
	}
	
forLoopCheckedEnd:
	consume(m_current.type() == Token::Type::parenRight, error::Message::expectedDifferent, ')');

forLoopUncheckedEnd:
	// This can come before the check, but as the check above doesn't require this to have been executed, it may save some time
	if (!value.condition())
		value.condition() = std::move(expr);

	if (expr)
		value.items().emplaceBack(std::move(expr));

	return value;
}

ast::detail::FunctionConstruct Parser::parseFunctionConstruct() {
	auto value = ast::detail::FunctionConstruct();

	consume(m_current.type() == Token::Type::parenLeft, error::Message::expectedDifferent, '(');

	while (m_current.type() != Token::Type::parenRight) {
		value.parameters.emplaceBack(parseIdentifierDeclaration());

		if (m_current.type() != Token::Type::comma)
			break;
	}

	consume(m_current.type() == Token::Type::parenRight, error::Message::expectedDifferent, ')');

	if (m_current.type() == Token::Type::colon) {
		next();
		value.type = ast::detail::type_ident_ptr::create(parseTypeIdentifier());
	}

	return value;
}

ast::BlockStmt Parser::parseBlockStatement() {
	next();

	auto value = ast::BlockStmt();
	while (m_current.type() != Token::Type::eof && m_current.type() != Token::Type::braceRight)
		value.pushStatement(parseStatementAndObjDeclaration());
	
	consume(m_current.type() == Token::Type::braceRight, error::Message::expectedDifferent, '}');

	return value;
}

bool Parser::basicParseObjectDeclaration(ast::obj_decl_ptr& value) {
	ast::detail::Attributes attributes;
	
	if (m_current.type() == Token::Type::attribute)
		attributes = parseAttributes();

	switch (m_current.type()) {
		case Token::Type::kLet:
			value = parseVariableDeclaration(std::move(attributes));

			break;

		case Token::Type::kFunc:
			value = parseFunctionDeclaration(std::move(attributes));

			break;

		case Token::Type::kCoroutine:
			// value = parseCoroutineDeclaration(attribute);

			break;

		case Token::Type::kClass:
			value = parseClassDeclaration(std::move(attributes));

			break;

		case Token::Type::kEnum:
			value = parseEnumDeclaration(std::move(attributes));

			break;
		
		default:
			return false;
	}

	return true;
}

} // namespace compiler

} // namespace elyrium
