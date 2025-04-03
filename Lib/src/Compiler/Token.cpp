#include <Elyrium/Compiler/Token.hpp>

#include <LSD/UnorderedSparseMap.h>

namespace elyrium {

namespace compiler {

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
