#include <Elyrium/Core/Error.hpp>

#include <LSD/UnorderedDenseMap.h>
#include <LSD/String.h>

#include <cstdio>

#define ELYRIUM_ERROR_MSG(str) "File \"%s\", line %zu:%zu:\n   |%s\n\t%*c\n"": %s!\n"

namespace elyrium {

SyntaxError::SyntaxError(
	lsd::StringView fileName, 
	size_type line, 
	size_type column,
	lsd::StringView lineSource, 
	error::Message message) {
	static const lsd::UnorderedDenseMap<error::Message, const char*> errorMsg({
		{ error::Message::invalidSyntax, "Invalid syntax" },
		{ error::Message::invalidNumericLiteral, "Invalid numeric literal" },
		{ error::Message::disallowedChar, "Disallowed character in code" },
		{ error::Message::unescapedChar, "Unescaped special character" },
		{ error::Message::invalidEscape, "Invalid escape sequence" },
		{ error::Message::emptyChar, "Character can't be empty" },
		{ error::Message::unclosedCharQuotes, "Character quotes weren't closed" },
		{ error::Message::unclosedStringQuotes, "String quotes weren't closed" },
		{ error::Message::unclosedParen, "Parenthesies weren't closed" },
		{ error::Message::unclosedBrace, "Braces weren't closed" },
		{ error::Message::unclosedBracket, "Brackets weren't closed" },
		{ error::Message::unclosedBlockComment, "Block comment wasn't closed" },
		{ error::Message::expectedExpression, "Expected expression" },
		{ error::Message::expectedIdentifier, "Expected indentifier" },
	});

	size_type len = std::snprintf(nullptr, 0, ELYRIUM_ERROR_MSG("Syntax error"), 
								  fileName.data(),
								  line,
								  column,
								  lineSource.data(),
								  static_cast<int>(column),
								  '^',
								  errorMsg.at(message));

	m_message.resize(len);
	std::snprintf(m_message.data(), len, ELYRIUM_ERROR_MSG("Syntax error"),
				  fileName.data(),
				  line,
				  column,
				  lineSource.data(),
				  static_cast<int>(column),
				  '^',
				  errorMsg.at(message));
}

} // namespace elyrium
