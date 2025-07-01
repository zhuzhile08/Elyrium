#include <cstdio>
#include <chrono>
#include <filesystem>

#include <Elyrium/Core/Config.hpp>
#include <Elyrium/Core/Error.hpp>

#include "Config.hpp"

#include <Elyrium/Compiler/Lexer.hpp>
#include <Elyrium/Compiler/Parser.hpp>

namespace {

inline constexpr lsd::StringView code = " \
/** \n\
 * Brainfuck interpreter in ELyrium \n\
 * \n\
 * Serves as a early syntax and VM test \n\
 * \n\
 * /* Nested comments */ \n\
 * // Nested comments \n\
 */ \n\
// ALso demonstrates some trailing comma placement \n\
\n\
import \"io\"; \n\
\n\
@const let SUCCESS = 0; \n\
@const let FAILURE = -1; \n\
\n\
@const let MOD_256_BITMAP = 0xFF; \n\
\n\
@const let TAPE_LEN = 16384; \n\
@const let STACK_LEN = 1024; \n\
\n\
let stack : arr[uint, STACK_LEN,]; \n\
let ptr : uint = 0; \n\
\n\
func interpret(tape : str*,) : int { \n\
	for (let iptr : uint, i : int,; iptr <= tape.size(); iptr++, i = tape[iptr]) { \n\
		if (i == '<') { \n\
			if (--ptr >= stack.size()) \n\
				break; \n\
		} else if (i == '>') { \n\
			if (++ptr >= stack.size()) \n\
				break; \n\
		} else if (i == '+') \n\
			++stack[ptr] &= MOD_256_BITMAP; \n\
		else if (i == '-') \n\
			--stack[ptr] &= MOD_256_BITMAP; \n\
		else if (i == '.') \n\
			std.io.putchar(stack[ptr]); \n\
		else if (i == ',') \n\
			stack[ptr] = std.io.getchar() & MOD_256_BITMAP; \n\
		else if (i == '[') { \n\
			if (stack[ptr] == 0) { \n\
				for (let nesting : uint = 1; nesting > 0) { \n\
					if (++iptr; iptr >= tape.size()) \n\
						return FAILURE; \n\
\n\
					if (i = tape[iptr]; i == ']') \n\
						--nesting; \n\
					else if (i == '[') \n\
						++nesting; \n\
				} \n\
			} \n\
		} else if (i == ']') { \n\
			if (stack[ptr] == 0) { \n\
				for (let nesting : uint = 1; nesting > 0) { \n\
					if (++iptr; iptr >= tape.size()) \n\
						return FAILURE; \n\
\n\
					if (i = tape[iptr]; i == '[') \n\
						--nesting; \n\
					else if (i == ']') \n\
						++nesting; \n\
				} \n\
			} \n\
		} else return FAILURE; \n\
	} \n\
\n\
	return SUCCESS; \n\
} \n\
\n\
func main() : int { \n\
	let tape : str = io.getstr(); \n\
\n\
	return interpret(tape); \n\
}";


int runCmdEnv() {
	lsd::String inputBuffer;
	inputBuffer.reserve(config::inputBufferStartingSize);


	elyrium::compiler::Parser parser(code, "idk");
	elyrium::compiler::ast::Module module;

	try {
		module = parser.parse();
	} catch(const elyrium::Exception& exception) {
		std::printf("%s", exception.what());
		std::fflush(stdin);
		inputBuffer.clear();

		return 1;
	}

	module.print();

	/*
	while (true) {
		std::printf(">>> ");

		auto c = std::getchar();
		while(c != '\n' && c != EOF) {
			inputBuffer.pushBack(c);
			c = std::getchar();
		}

		if (inputBuffer == "__EXIT_CLI__") break;

		elyrium::compiler::Parser parser(inputBuffer.data(), "stdin");
		elyrium::compiler::ast::stmt_ptr stmt;

		try {
			stmt = parser.parse();
		} catch(const elyrium::Exception& exception) {
			std::printf("%s", exception.what());
			std::fflush(stdin);
			inputBuffer.clear();

			continue;
		}

		stmt->print();

		std::fflush(stdin);
		inputBuffer.clear();
	}
	*/

	return 0;
}

int checkOptions(char* arg) {
	std::printf("Option given!\n");

	return 0;
}

int runFile(char* path) {
	auto globalPath = std::filesystem::current_path().append(path);
	auto file = std::fopen(globalPath.c_str(), "r");

	if (!file) {
		std::printf("Could not open file at path \"%s\" with error: [Errno: %i] | %s\n", globalPath.c_str(), errno, std::strerror(errno));
		
		return errno;
	} else {
		std::printf("File loaded!\n");

		return 0;
	}
}

} // namespace

int main(int argc, char* argv[]) {
	if (argc > 1) {
		if (*argv[1] == '-') return checkOptions(argv[1]);
		else return runFile(argv[1]);
	}
	else {
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto time = std::localtime(&now);

		lsd::String timeStr(48, '\0'); // Generous guess at how long the date will be
		std::strftime(timeStr.data(), timeStr.size(), "%Y-%b-%d, %T", time);
		
		std::printf(
			"[%s] | Elyrium Interpreter %s | Elyrium Base Library %s\nType \"todo\" for additional info.\n", 
			timeStr.data(),
#ifdef ELYRIUM_CLI_VERSION
			ELYRIUM_CLI_VERSION,
#else
			"VERSION UNDEFINED",
#endif
			elyrium::config::version
		);
		
		return runCmdEnv();
	}
}
