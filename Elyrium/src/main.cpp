#include <stdexcept>
#include <chrono>

#include <Common/Config.h>
#include <Config.h>

#include <fmt/core.h>
#include <fmt/chrono.h>

int main(int argc, char* argv[]) {
	if (argc > 1) {
		

	} else {
		fmt::println(
			"[{:%Ec %H:%M:%S}] | Elyrium Interpreter {} | Elyrium Base Library {}\nType \"todo\" for additional info.", 
			fmt::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())), 
			ELYRIUM_VERSION,
			elyrium::config::version
		);

		fmt::print(">>>");

		char buffer[2];
		while(std::fgets(buffer, 2, stdin)) {
			return 0;
		}
	}

	return 0;
}