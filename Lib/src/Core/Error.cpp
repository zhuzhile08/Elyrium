#include <Elyrium/Core/Error.hpp>

#include <LSD/UnorderedSparseMap.h>
#include <LSD/String.h>

#include <cstdio>

namespace elyrium {

namespace error {

void report(lsd::StringView filename, size_type lineCount, size_type column, lsd::StringView line, Type type, Message message) {
	static lsd::UnorderedSparseMap<Type, const char*> errorType({
		{ Type::syntaxError, "Syntax Error" }
	});

	static lsd::UnorderedSparseMap<Message, const char*> errorMsg({
		{ Message::invalidSyntax, "Invalid syntax" },
		{ Message::invalidNumericLiteral, "Invalid numeric literal" },
		{ Message::disallowedChar, "Disallowed character in code" },
		{ Message::unescapedChar, "Unescaped special character" },
		{ Message::invalidEscape, "Invalid escape sequence" },
		{ Message::emptyChar, "Character can't be empty" },
		{ Message::unclosedCharQuotes, "Character quotes weren't closed" },
		{ Message::unclosedStringQuotes, "String quotes weren't closed" },
		{ Message::unclosedParen, "Parenthesies weren't closed" },
		{ Message::unclosedBrace, "Braces weren't closed" },
		{ Message::unclosedBracket, "Brackets weren't closed" },
		{ Message::unclosedBlockComment, "Block comment wasn't closed" }
	});

	std::fprintf(stderr, "File \"%s\", line %zu:%zu:\n   |%s\n\t%*c\n%s: %s!", 
		filename.data(), 
		lineCount,
		column,
		line.data(),
		static_cast<int>(column),
		'^',
		errorType.at(type),
		errorMsg.at(message));
}

} // namespace error


// Internal system exceptions

SyntaxError::SyntaxError(
	lsd::StringView fileName, 
	size_type lineCount, 
	lsd::StringView line, 
	const lsd::String& message) : ElyriumError(message) {
	
}
SyntaxError::SyntaxError(
	lsd::StringView fileName, 
	size_type lineCount, 
	lsd::StringView line, 
	const char* message) : ElyriumError(message) {
	
}

} // namespace elyrium
