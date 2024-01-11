//
// Created by Enzo Crema on 02/01/2024.
//

#pragma once

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

	struct MemoryStats : NO_COPY_NOR_MOVE{
		u64 totalAllocated{};
		std::array<u64, MEMORY_TAG_COUNT> tagAllocated{};
	};


	static void* allocMemory(size_t size,  Tag tag);

	static void freeMemory(void* block, size_t size, Tag tag);

	static void* zeroMemory(void* block, u64 size);
	static void* copyMemory(void* dest, const void* src, u64 size);
	static void* setMemory(void* dest, i32 value, u64 size);

	static void initializeMemory();
	static void shutdownMemory();
	static void getMemoryUsage();

	static MemoryStats mMemoryStats;

};