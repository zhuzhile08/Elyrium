#include <Core/Error.h>

#include <LSD/UnorderedSparseMap.h>

#include <string>

namespace elyrium {

namespace error {

void report(std::string_view fileName, std::string_view text, size_type line, size_type column, Type type, Message message) {
	static lsd::UnorderedSparseMap<Type, const char*> errorType({
		{ Type::syntaxError, "Syntax Error" }
	});

	static lsd::UnorderedSparseMap<Message, const char*> errorMsg({
		{ Message::invalidSyntax, "Invalid syntax" },
		{ Message::emptyChar, "Character can't be empty" },
		{ Message::untermChar, "Character wasn't terminated" },
		{ Message::untermString, "String wasn't terminated" },
		{ Message::unclosedParen, "Parenthesies weren't closed" },
		{ Message::unclosedBrace, "Braces weren't closed" },
		{ Message::unclosedBracket, "Brackets weren't closed" }
	});

	fmt::println(stderr, "File \"{}\", line {}:\n\t{}\n\t{}^\n{}: {}", fileName, line, text, std::string(column, ' '), errorType.at(type), errorMsg.at(message));
}

} // namespace error

} // namespace elyrium