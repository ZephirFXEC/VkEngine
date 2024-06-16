#include "renderer/app.hpp"
#include "utils/memory.hpp"


int main() {
	Memory::initializeMemory();

	vke::App app{};  // create app

	try {
		// try to run app
		app.run();
	} catch (const std::exception& e) {
		// catch exceptions
		fmt::print("Exception: {}\n", e.what());
		return EXIT_FAILURE;
	}

	Memory::shutdownMemory();

	return EXIT_SUCCESS;
}