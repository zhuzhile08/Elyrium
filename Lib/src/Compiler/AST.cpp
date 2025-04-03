#include <Elyrium/Compiler/AST.hpp>

namespace elyrium {

namespace compiler {

namespace ast {

void AtomicExpr::pushCall(detail::arg_t&& args) {
	m_chain.emplaceBack(std::move(args));
}

void AtomicExpr::pushSubscript(detail::subscript_t&& expr) {
	m_chain.emplaceBack(std::move(expr));
}

void AtomicExpr::pushMember(const Token& identifier) {
	m_chain.pushBack(identifier);
}

} // namespace ast

} // namespace compiler

} // namespace elyrium
