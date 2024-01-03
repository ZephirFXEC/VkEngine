//
// Created by Enzo Crema on 02/01/2024.
//

#include "memory.hpp"
#include "logger.hpp"

void Memory::initializeMemory() { memset(&mMemoryStats, 0, sizeof(mMemoryStats)); }

void Memory::shutdownMemory() { memset(&mMemoryStats, 0, sizeof(mMemoryStats)); }

void* Memory::allocMemory(u64 size, Tag tag) {
	if (tag == Tag::MEMORY_TAG_UNKNOWN) {
		VKWARN("Allocating memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
	}

	mMemoryStats.totalAllocated += size;
	mMemoryStats.tagAllocated.at(tag) += size;

	void* block = malloc(size);
	memset(block, 0, size);

	return block;
}

void* Memory::freeMemory(void* block, u64 size, Tag tag) {
	if (tag == Tag::MEMORY_TAG_UNKNOWN) {
		VKWARN("Freeing memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
	}

	mMemoryStats.totalAllocated -= size;
	mMemoryStats.tagAllocated.at(tag) -= size;

	free(block);
	return nullptr;
}

void* Memory::zeroMemory(void* block, u64 size) { return memset(block, 0, size); }

void* Memory::copyMemory(void* dest, const void* src, u64 size) { return memcpy(dest, src, size); }

void* Memory::setMemory(void* dest, i32 value, u64 size) { return memset(dest, value, size); }

void Memory::getMemoryUsage() {
	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	std::string memoryUsage = fmt::format("Total allocated: {} bytes\n", mMemoryStats.totalAllocated);

	for (u32 i = 0; i < Tag::MEMORY_TAG_COUNT; i++) {
		const u64 bytes = mMemoryStats.tagAllocated.at(i);
		if (bytes > 0) {
			std::string tag = MEMORY_TAG_NAMES.at(i);
			std::string bytesStr = fmt::format("{} bytes", bytes);
			std::string gibStr = fmt::format("{:.2f} GiB", (f32)bytes / (f32)gib);
			std::string mibStr = fmt::format("{:.2f} MiB", (f32)bytes / (f32)mib);
			std::string kibStr = fmt::format("{:.2f} KiB", (f32)bytes / (f32)kib);

			memoryUsage += fmt::format("{}: {} ({}, {}, {})\n", tag, bytesStr, gibStr, mibStr, kibStr);
		}
	}

	fmt::print("{}", memoryUsage);
}


Memory::MemoryStats Memory::mMemoryStats = {};
