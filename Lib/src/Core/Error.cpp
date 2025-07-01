#include "LSD/StringView.h"
#include <Elyrium/Core/Error.hpp>

#include <LSD/UnorderedDenseMap.h>
#include <LSD/String.h>

#include <cstdio>

#define ELYRIUM_CUSTOM_ERROR_MSG(str, fmt) "File \"%s\", line %zu:%zu\n   | %.*s\n     %*c " str ": " fmt "!\n"
#define ELYRIUM_ERROR_MSG(str) ELYRIUM_CUSTOM_ERROR_MSG(str, "%s") 

namespace elyrium {

SyntaxError::SyntaxError(
	lsd::StringView fileName, 
	size_type line, 
	size_type column,
	lsd::StringView lineSource, 
	error::Message message,
	char expected) {
	static const lsd::UnorderedDenseMap<error::Message, const char*> errorMsg({
		{ error::Message::invalidSyntax, "Invalid syntax" },
		{ error::Message::invalidNumericLiteral, "Invalid numeric literal" },
		{ error::Message::disallowedChar, "Disallowed character in code" },
		{ error::Message::unescapedChar, "Unescaped special character" },
		{ error::Message::invalidEscape, "Invalid escape sequence" },
		{ error::Message::emptyChar, "Character can't be empty" },
		{ error::Message::unclosedBlockComment, "Expected \"*/\"" },
		{ error::Message::expectedExpression, "Expected expression" },
		{ error::Message::expectedIdentifier, "Expected indentifier" },
		{ error::Message::expectedDeclaration, "Expected declaration" },
		{ error::Message::noCatchBehindTry, "No catch-block was found behind a try-block"},
		{ error::Message::importDeclRequiresStrOrConst, "Import declaration requires string or a constant string variable" },
	});

	
	if (message == error::Message::expectedDifferent) {
		size_type len = std::snprintf(nullptr, 0, ELYRIUM_CUSTOM_ERROR_MSG("Syntax error", "Expected '%c'"),
									  fileName.data(),
									  line + 1,
									  column,
									  static_cast<int>(lineSource.size()),
									  lineSource.data(),
									  static_cast<int>(column) + 1,
									  '^',
									  expected);
		
		m_message.resize(len);
		std::snprintf(m_message.data(), len + 1, ELYRIUM_CUSTOM_ERROR_MSG("Syntax error", "Expected '%c'"),
					  fileName.data(),
					  line + 1,
					  column,
					  static_cast<int>(lineSource.size()),
					  lineSource.data(),
					  static_cast<int>(column) + 1,
					  '^',
					  expected);
	} else {
		size_type len = std::snprintf(nullptr, 0, ELYRIUM_ERROR_MSG("Syntax error"), 
									  fileName.data(),
									  line + 1,
									  column,
									  static_cast<int>(lineSource.size()),
									  lineSource.data(),
									  static_cast<int>(column) + 1,
									  '^',
									  errorMsg.at(message));

		m_message.resize(len);
		std::snprintf(m_message.data(), len + 1, ELYRIUM_ERROR_MSG("Syntax error"),
					  fileName.data(),
					  line + 1,
					  column,
					  static_cast<int>(lineSource.size()),
					  lineSource.data(),
					  static_cast<int>(column) + 1,
					  '^',
					  errorMsg.at(message));
	}
}

} // namespace elyrium
