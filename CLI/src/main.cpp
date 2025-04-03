#include <stdexcept>
#include <cstdio>
#include <chrono>
#include <filesystem>

#include <Elyrium/Core/Config.hpp>
#include <Elyrium/Core/Error.hpp>

#include "Config.hpp"

#include <Elyrium/Compiler/Lexer.hpp>

namespace {

int runCmdEnv() {
	lsd::String inputBuffer;
	inputBuffer.reserve(config::inputBufferStartingSize);

	while (true) {
		std::printf(">>> ");

		auto c = std::getchar();
		while(c != '\n' && c != EOF) {
			inputBuffer.push_back(c);
			c = std::getchar();
		}

		if (inputBuffer == "__EXIT_CLI__") break;

		elyrium::compiler::Lexer lexer(inputBuffer, "");

		std::printf("Tokens: ");
		for (elyrium::compiler::Token tok = lexer.nextToken();
			 tok.type() != elyrium::compiler::Token::Type::eof &&
			 tok.type() != elyrium::compiler::Token::Type::invalid;
			 tok = lexer.nextToken()) std::printf("%s", tok.stringify().data());
		std::printf("\n");

		std::fflush(stdin);

		inputBuffer.clear();
	}

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
