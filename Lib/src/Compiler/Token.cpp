#include "LSD/StringView.h"
#include <Elyrium/Compiler/Token.hpp>

#include <LSD/UnorderedFlatMap.h>

namespace elyrium {

namespace compiler {

lsd::StringView Token::lineSource() const noexcept {
	if (m_data.empty()) return { };

	auto end = m_data.end();
	while (*end != '\n' || *end != '\0') // This should in theory be safe, as \0 can't appear but is guaranteed to appear at the end of the lsd::String
		++end;

	return lsd::StringView { m_data.begin() - m_column, end };
}

lsd::String Token::stringify() const {
	size_type length = std::snprintf(nullptr, 0, "[%i: \"%.*s\" %zu, %zu]",
									 static_cast<type_tag>(m_type),
									 static_cast<int>(m_data.length()),
									 m_data.data(),
									 m_line, m_column) + 1;
	lsd::String output(length, '\0');
	std::snprintf(output.data(), length, "[%i: \"%.*s\" %zu, %zu]",
				  static_cast<type_tag>(m_type),
				  static_cast<int>(m_data.length()),
				  m_data.data(),
				  m_line, m_column);

	return output;
}

} // namespace compiler

} // namespace elyrium
