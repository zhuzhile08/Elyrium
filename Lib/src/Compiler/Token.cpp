#include <Elyrium/Compiler/Token.hpp>

#include <LSD/UnorderedFlatMap.h>

namespace elyrium {

namespace compiler {

lsd::String Token::lineSource(std::size_t& additionalSpaces) const noexcept {
	if (m_data.empty()) return { };

	lsd::String str;

	auto end = m_data.end();
	while (*end != '\n' && *end != '\0') // This should in theory be safe, as \0 can't appear but is guaranteed to appear at the end of the lsd::String
		++end;

	for (auto begin = m_data.begin() - m_column; begin != end; begin++) {
		switch (auto c = *begin; c) {
			case '\t':
				str.append("    ", 4);
				additionalSpaces += 3;

			case '\r':
			case '\f':
			case '\v':
				break;

			default:
				str.pushBack(c);
		}
	}

	return str;
}

lsd::String Token::stringify() const {
	size_type length = std::snprintf(nullptr, 0, "[%zu: \"%.*s\" %zu, %zu]",
									 static_cast<type_tag>(m_type),
									 static_cast<int>(m_data.length()),
									 m_data.data(),
									 m_line, m_column) + 1;
	lsd::String output(length, '\0');
	std::snprintf(output.data(), length, "[%zu: \"%.*s\" %zu, %zu]",
				  static_cast<type_tag>(m_type),
				  static_cast<int>(m_data.length()),
				  m_data.data(),
				  m_line, m_column);

	return output;
}

} // namespace compiler

} // namespace elyrium
