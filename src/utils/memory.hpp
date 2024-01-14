//
// Created by Enzo Crema on 02/01/2024.
//

#pragma once

#include "logger.hpp"
#include "types.hpp"

class Memory : NO_COPY_NOR_MOVE {
public:
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

	static constexpr std::array<const char*, MEMORY_TAG_COUNT> MEMORY_TAG_NAMES = {
		"Unknown", "Array", "Vector", "Texture", "Buffer", "Renderer", "Engine", "Vulkan", "Window"};

	struct MemoryStats : NO_COPY_NOR_MOVE {
		u64 totalAllocated{};
		std::array<u64, MEMORY_TAG_COUNT> tagAllocated{};
	};


	template <typename T>
	static T* allocMemory(const size_t size, const Tag tag) {
		if (tag == MEMORY_TAG_UNKNOWN) {
			VKWARN("Allocating memory with MEMORY_TAG_UNKNOWN. Re-classify it!");
		}

		const size_t totalSize = size * sizeof(T);
		mMemoryStats.totalAllocated += totalSize;
		mMemoryStats.tagAllocated.at(tag) += totalSize;

		T* block = new(std::nothrow) T[size]; // Using new instead of malloc

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

		if(!std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < size; ++i) {
				block[i].~T();
			}
        }

		const size_t totalSize = size * sizeof(T);
		mMemoryStats.totalAllocated -= totalSize;
		mMemoryStats.tagAllocated.at(tag) -= totalSize;

		delete[] block; // Using delete[] instead of free
	}

	template <typename T>
	static void* zeroMemory(T* block, const u64 size) { return memset(block, 0, size); }

	static void* copyMemory(void* dest, const void* src, const u64 size) { return memcpy(dest, src, size); }

	static void* setMemory(void* dest, const i32 value, const u64 size) { return memset(dest, value, size); }

	static void initializeMemory() {
		mMemoryStats.tagAllocated.fill(0);
		mMemoryStats.totalAllocated = 0;
	}


	static void shutdownMemory() {
		if (mMemoryStats.totalAllocated > 0) {
			VKWARN("Memory leak detected!");
			getMemoryUsage();
		}
	}


	static void getMemoryUsage() {
		constexpr u64 gib = 1024ul * 1024ul * 1024ul;
		constexpr u64 mib = 1024ul * 1024ul;
		constexpr f64 kib = 1024ul;

		std::string memoryUsage = fmt::format("Total allocated: {} bytes\n", mMemoryStats.totalAllocated);

		for (u32 i = 0; i < MEMORY_TAG_COUNT; i++) {
			if (const u64 bytes = mMemoryStats.tagAllocated.at(i); bytes > 0) {
				std::string tag = MEMORY_TAG_NAMES.at(i);
				std::string bytesStr{fmt::format("{} bytes", bytes)};
				std::string gibStr{fmt::format("{:.2f} GiB", S_CAST(f32, bytes) / S_CAST(f32, gib))};
				std::string mibStr{fmt::format("{:.2f} MiB", S_CAST(f32, bytes) / S_CAST(f32, mib))};
				std::string kibStr{fmt::format("{:.2f} KiB", S_CAST(f32, bytes) / S_CAST(f32, kib))};

				memoryUsage += fmt::format("{}: {} ({}, {}, {})\n", tag, bytesStr, gibStr, mibStr, kibStr);
			}
		}

		fmt::print("\r{}", memoryUsage);
	}

	inline static MemoryStats mMemoryStats = {};
};
