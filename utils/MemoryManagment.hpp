#pragma once

template <typename T, typename Derived>
class BaseMemoryManaged {
protected:
	size_t size;
	T* pointer;

public:
	// Constructor
	BaseMemoryManaged(size_t size = 0, T* ptr = nullptr) : size(size), pointer(ptr) {}

	// Destructor
	~BaseMemoryManaged() {
		static_cast<Derived*>(this)->clearMemory();
	}

	// Delete Copy Constructor
	BaseMemoryManaged(const BaseMemoryManaged&) = delete;

	// Define Move Constructor
	BaseMemoryManaged(BaseMemoryManaged&& other) noexcept
		: size(other.size), pointer(other.pointer) {
		other.pointer = nullptr;
		other.size = 0;
	}

	// Delete Copy Assignment Operator
	BaseMemoryManaged& operator=(const BaseMemoryManaged&) = delete;

	// Define Move Assignment Operator
	BaseMemoryManaged& operator=(BaseMemoryManaged&& other) noexcept {
		if (this != &other) {
			static_cast<Derived*>(this)->clearMemory();
			size = other.size;
			pointer = other.pointer;
			other.size = 0;
			other.pointer = nullptr;
		}
		return *this;
	}
};