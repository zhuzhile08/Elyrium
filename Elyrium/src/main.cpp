#include <stdexcept>
#include <cstdio>
#include <chrono>
#include <filesystem>

#include <Common/Config.h>
#include <Config.h>

#include <Interpreter/Lexer.h>

#include <fmt/core.h>
#include <fmt/chrono.h>

namespace {

int runCmdEnv() {
	fmt::print(">>> ");

	/**
	char buffer[2];
	while(std::fgets(buffer, 2, stdin)) {
		return 0;
	}
	*/

	elyrium::Lexer lexer("let a : i32 = 2 + 3; let b: f32 =3+2.4;let c:string=\"Hello world\";");
	fmt::println("Tokens: {}", lexer.stringifyTokens());

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