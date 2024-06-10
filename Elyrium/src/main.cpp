#include <stdexcept>
#include <cstdio>
#include <chrono>
#include <filesystem>

#include <Core/Config.h>
#include <Core/Error.h>
#include <Config.h>

#include <Interpreter/Lexer.h>

#include <fmt/core.h>
#include <fmt/chrono.h>

namespace {

int runCmdEnv() {
	std::string inputBuffer;
	inputBuffer.reserve(config::inputBufferStartingSize);

	while (true) {
		fmt::print(">>> ");

		auto c = std::getchar();
		while(c != '\n' && c != EOF) {
			inputBuffer.push_back(c);
			c = std::getchar();
		}

		if (inputBuffer == "__EXIT_CMD__") break;

		elyrium::Lexer lexer(inputBuffer);
		fmt::println("Tokens: {}", lexer.stringifyTokens());

		std::fflush(stdin);

		inputBuffer.clear();
	}

	return 0;
}

int checkOptions(char* arg) {
	fmt::println("Option given!");

	return 0;
}

int runFile(char* path) {
	auto globalPath = std::filesystem::current_path().append(path);
	auto file = std::fopen(globalPath.c_str(), "r");

	if (!file) {
		fmt::println("elyrium: Could not open file at path \"{}\" with error: [Errno: {}] | {}", globalPath.c_str(), errno, std::strerror(errno));
		
		return errno;
	} else {
		fmt::println("File loaded!");

		return 0;
	}
}

}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		if (*argv[1] == '-') return checkOptions(argv[1]);
		else return runFile(argv[1]);
	} else {
		fmt::println(
			"[{:%Ec %H:%M:%S}] | Elyrium Interpreter {} | Elyrium Base Library {}\nType \"todo\" for additional info.", 
			fmt::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())), 
			ELYRIUM_VERSION,
			elyrium::config::version
		);
		
		return runCmdEnv();
	}
}
