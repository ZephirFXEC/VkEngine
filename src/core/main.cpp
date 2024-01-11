#include "app.hpp"
#include "utils/memory.hpp"



int main() {
	vke::App app{};  // create app

	Memory::initializeMemory();

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