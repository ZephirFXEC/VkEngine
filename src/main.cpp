#include "app.hpp"

// std

int main() {
	vke::App app{};  // create app

	try {
		// try to run app
		app.run();
	} catch (const std::exception& e) {
		// catch exceptions
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}