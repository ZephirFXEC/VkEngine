//
// Created by Enzo Crema on 02/01/2024.
//

#include "memory.hpp"
#include "logger.hpp"

void Memory::initializeMemory() {
	mMemoryStats.tagAllocated.fill(0);
	mMemoryStats.totalAllocated = 0;
}

void Memory::shutdownMemory() {
	if (mMemoryStats.totalAllocated > 0) {
		VKWARN("Memory leak detected!");
		getMemoryUsage();
	}
}

void* Memory::allocMemory(const size_t size, const Tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		VKWARN("Allocating memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
	}

	mMemoryStats.totalAllocated += size;
	mMemoryStats.tagAllocated.at(tag) += size;

	void* block = malloc(size);

	zeroMemory(block, size);

	return block;
}

void Memory::freeMemory(void* block, const size_t size, const Tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		VKWARN("Freeing memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
	}

	if (block == nullptr) {
		VKWARN("Trying to free nullptr!");
		return;
	}

	mMemoryStats.totalAllocated -= size;
	mMemoryStats.tagAllocated.at(tag) -= size;

	free(block);
}

void* Memory::zeroMemory(void* block, const u64 size) { return memset(block, 0, size); }

void* Memory::copyMemory(void* dest, const void* src, const u64 size) { return memcpy(dest, src, size); }

void* Memory::setMemory(void* dest, const i32 value, const u64 size) { return memset(dest, value, size); }

void Memory::getMemoryUsage() {
	constexpr u64 gib = 1024ul * 1024ul * 1024ul;
	constexpr u64 mib = 1024ul * 1024ul;
	constexpr f64 kib = 1024ul;

	std::string memoryUsage = fmt::format("Total allocated: {} bytes\n", mMemoryStats.totalAllocated);

	for (u32 i = 0; i < MEMORY_TAG_COUNT; i++) {
		if (const u64 bytes = mMemoryStats.tagAllocated.at(i); bytes > 0) {
			std::string tag = MEMORY_TAG_NAMES.at(i);
			std::string bytesStr{ fmt::format("{} bytes", bytes) };
			std::string gibStr{fmt::format("{:.2f} GiB", S_CAST(f32, bytes) / S_CAST(f32, gib))};
			std::string mibStr{fmt::format("{:.2f} MiB", S_CAST(f32, bytes) / S_CAST(f32, mib))};
			std::string kibStr{fmt::format("{:.2f} KiB", S_CAST(f32, bytes) / S_CAST(f32, kib))};

			memoryUsage += fmt::format("{}: {} ({}, {}, {})\n", tag, bytesStr, gibStr, mibStr, kibStr);
		}
	}

	fmt::print("{}", memoryUsage);
}


Memory::MemoryStats Memory::mMemoryStats = {};