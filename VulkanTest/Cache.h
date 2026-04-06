#pragma once

#include <cstdint>
#include <vector>

#include "animation/AnimatorStruct.h"
#include "Vertex.hpp"
#include "items/ItemInterface.h"
#include "Buffer.h"
#include "Animator.h"
#include "utils/Hash.hpp"

class Buffer;

class Cache
{
public:
	Cache(Buffer& buffer, Animator& animator);

	struct CacheModel {
		uint32_t vertexCount;
		uint32_t indexCount;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		AnimatorStruct::Skeleton skeleton;
		AnimatorStruct::AnimatorData animatorData;
	};

	struct CacheVec3 {
		float x, y, z;
	};

	struct CacheHeader {

		uint32_t magic = 0x4D4F444C;
		uint32_t version = 1;
		uint64_t sourceHash;

		uint64_t vertexCount;
		uint64_t indexCount;
		uint64_t meshCount;

		uint64_t vertexOffset;
		uint64_t indexOffset;
		uint64_t meshOffset;

		uint64_t skeletonOffset;
		uint64_t animationOffset;

		CacheVec3 center;
		CacheVec3 extents;
	};

	struct MeshEntry {
		uint32_t meshIndex;
		uint32_t indexCount;

		//uint32_t materialIndex;
	};



	void loadCache(std::string originalPath, std::string cachePath, ItemInterface& item);
	void writeCache(std::string originalPath, std::string cachePath, ItemInterface& item);
	bool cacheExists(const std::string path);

private:
	Buffer& m_Buffer;
	Animator& m_Animator;
};