#include "app.hpp"



int main() {
	vke::App app{};  // create app

	try {
		// try to run app
		app.run();
	} catch (const std::exception& e) {
		// catch exceptions
		fmt::print("Exception: {}\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}