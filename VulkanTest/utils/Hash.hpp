#pragma once

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace Utils {
	
	inline uint64_t hashFile(const std::string& path)
	{
		auto time = std::filesystem::last_write_time(path);
		return time.time_since_epoch().count();
		
	//	std::ifstream file(path, std::ios::binary);

	//	if (!file)
	//		throw std::runtime_error("Failed to open file for hashing");

	//	const uint64_t FNV_OFFSET = 1469598103934665603ULL;
	//	const uint64_t FNV_PRIME = 1099511628211ULL;

	//	uint64_t hash = FNV_OFFSET;

	//	constexpr size_t BUFFER_SIZE = 1024; // 1MB
	//	std::vector<char> buffer(BUFFER_SIZE);

	//	while (file) {
	//		file.read(buffer.data(), BUFFER_SIZE);
	//		std::streamsize bytesRead = file.gcount();

	//		for (std::streamsize i = 0; i < bytesRead; i++) {
	//			hash ^= static_cast<uint8_t>(buffer[i]);
	//			hash *= FNV_PRIME;
	//		}
	//	}

	//	return hash;
	}
}