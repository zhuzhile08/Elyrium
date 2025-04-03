#include <Elyrium/Compiler/Parser.hpp>

#include <LSD/UnorderedSparseMap.h>

using namespace lsd::enum_operators;

namespace elyrium {

namespace compiler {

namespace {



} // namespace


Parser::Parser(lsd::String&& source, lsd::String&& path) :
	m_path(path), m_source(std::move(source)), m_lexer(m_source, m_path), m_current(m_lexer.nextToken())
{

}

ast::expr_ptr Parser::parseExpression(Precedence precedence) {
	static lsd::UnorderedSparseMap<lsd::StringView, ast::expr_ptr_t (Parser::*) ()> prefixParsers {
	};

	ast::expr_ptr expr;

	ast::ExprType expected = ast::ExprType::none;

	bool terminated = true;

	do {
		m_current = m_lexer.nextToken();

		switch (m_current.type()) {
			case Token::Type::logicNot: case Token::Type::add: case Token::Type::sub: case Token::Type::bitNot:
			case Token::Type::deref: case Token::Type::increment: case Token::Type::decrement:


		}
	} while (m_current.type() != Token::Type::none);

	if (!terminated) {

	}

	return expr;
}

} // namespace compiler

} // namespace elyrium
