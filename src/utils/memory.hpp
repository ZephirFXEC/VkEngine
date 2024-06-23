//
// Created by Enzo Crema on 02/01/2024.
//

#pragma once

#include <atomic>

#include "logger.hpp"
#include "types.hpp"

using Tag = enum Tag : u8 {
	MEMORY_TAG_UNKNOWN,
	MEMORY_TAG_ARRAY,
	MEMORY_TAG_VECTOR,
	MEMORY_TAG_TEXTURE,
	MEMORY_TAG_BUFFER,
	MEMORY_TAG_RENDERER,
	MEMORY_TAG_ENGINE,
	MEMORY_TAG_VULKAN,
	MEMORY_TAG_WINDOW,

	MEMORY_TAG_COUNT
};

struct MemoryStats {
	std::atomic<u64> totalAllocated{};
	std::array<std::atomic<u64>, MEMORY_TAG_COUNT> tagAllocated{};
	u32 allocCount = 0;

	void add(const u64 size, const Tag tag) {
		totalAllocated.fetch_add(size, std::memory_order::relaxed);
		tagAllocated.at(tag).fetch_add(size, std::memory_order::relaxed);
		allocCount++;
	}

	void remove(const u64 size, const Tag tag) {
		totalAllocated.fetch_sub(size, std::memory_order::relaxed);
		tagAllocated.at(tag).fetch_sub(size, std::memory_order::relaxed);
		allocCount--;
	}

	void reset() {
		totalAllocated.store(0, std::memory_order::relaxed);
		for (auto& t : tagAllocated) {
			t.store(0, std::memory_order::relaxed);
		}
	}
};

class Memory : NO_COPY_NOR_MOVE {
   public:
	template <typename T>
	static T* allocMemory(const size_t size, const Tag tag) {
		if (tag == MEMORY_TAG_UNKNOWN) {
			VKWARN("Allocating memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
		}

		const size_t totalSize = size * sizeof(T);
		mMemoryStats.add(totalSize, tag);

		T* block = new (std::nothrow) T[size];  // Using new instead of malloc

		if (block) {
			zeroMemory(block, totalSize);
		}

		return block;
	}

	template <typename T>
	static void freeMemory(T* block, const size_t size, const Tag tag) {
		if (tag == MEMORY_TAG_UNKNOWN) {
			VKWARN("Freeing memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
		}

		if (block == nullptr) {
			VKWARN("Trying to free nullptr!");
			return;
		}

		if (!std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < size; ++i) {
				block[i].~T();
			}
		}

		const size_t totalSize = size * sizeof(T);
		mMemoryStats.remove(totalSize, tag);

		delete[] block;  // Using delete[] instead of free
	}

	template <typename T>
	static void* zeroMemory(T* block, const u64 size) {
		return memset(block, 0, size);
	}

	static void* copyMemory(void* dest, const void* src, const u64 size) { return memcpy(dest, src, size); }

	static void* setMemory(void* dest, const i32 value, const u64 size) { return memset(dest, value, size); }

	static void initializeMemory() { mMemoryStats.reset(); }


	static void shutdownMemory() {
		if (mMemoryStats.totalAllocated.load() > 0) {
			VKWARN("Memory leak detected!");
			getMemoryUsage();
		}
	}


	static void getMemoryUsage() {
		constexpr u64 kib = 1024ul;
		constexpr u64 mib = 1024ul * kib;
		constexpr u64 gib = 1024ul * mib;

		std::string memoryUsage = fmt::format("Total allocated: {} bytes\n", mMemoryStats.totalAllocated.load());

		for (u32 i = 0; i < MEMORY_TAG_COUNT; i++) {
			if (const u64 bytes = mMemoryStats.tagAllocated.at(i).load(); bytes > 0) {
				std::string tag = MEMORY_TAG_NAMES.at(i);
				std::string bytesStr{fmt::format("{} bytes", bytes)};
				std::string gibStr{fmt::format("{:.2f} GiB", static_cast<f32>(bytes) / static_cast<f32>(gib))};
				std::string mibStr{fmt::format("{:.2f} MiB", static_cast<f32>(bytes) / static_cast<f32>(mib))};
				std::string kibStr{fmt::format("{:.2f} KiB", static_cast<f32>(bytes) / static_cast<f32>(kib))};

				memoryUsage += fmt::format("{}: {} ({}, {}, {})\n", tag, bytesStr, gibStr, mibStr, kibStr);
			}
		}

		fmt::print("\r{}", memoryUsage);
		fmt::print("\n{}", mMemoryStats.allocCount);
	}

   private:
	static constexpr std::array<const char*, MEMORY_TAG_COUNT> MEMORY_TAG_NAMES = {
	    "Unknown", "Array", "Vector", "Texture", "Buffer", "Renderer", "Engine", "Vulkan", "Window"};

	inline static MemoryStats mMemoryStats = {};
};
