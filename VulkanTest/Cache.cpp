#include "Cache.h"

#include "Buffer.h"

Cache::Cache(Buffer& buffer, Animator& animator) : m_Buffer(buffer), m_Animator(animator) {}

void Cache::writeCache(std::string originalPath, std::string cachePath, ItemInterface& item) {

	std::ofstream out(cachePath, std::ios::binary);

	CacheHeader header{};
	header.magic = 0x4D4F444C;
	header.sourceHash = Utils::hashFile(cachePath);
	std::cout << "ketchup\n";
	header.vertexCount = static_cast<uint64_t>(m_Buffer.globalVertices.size());
	std::cout << "mustard\n";
	header.indexCount = m_Buffer.globalIndices.size();
	header.meshCount = item.meshData.meshOffset.size();

	uint64_t offset = sizeof(CacheHeader);

	header.vertexOffset = offset;
	offset += header.vertexCount * sizeof(Vertex);

	header.indexOffset = offset;
	offset += header.indexCount * sizeof(uint32_t);

	header.meshOffset = offset;
	offset += header.meshCount * sizeof(MeshEntry);

	out.write(
		(char*)m_Buffer.globalVertices.data(),
		header.vertexCount * sizeof(Vertex)
	);

	out.write(
		(char*)m_Buffer.globalIndices.data(),
		header.indexCount * sizeof(uint32_t)
	);

	header.center.x = item.center.x;
	header.center.y = item.center.y;
	header.center.z = item.center.z;

	header.extents.x = item.extents.x;
	header.extents.y = item.extents.y;
	header.extents.z = item.extents.z;

	out.write(
		(char*)&item.center, sizeof(CacheVec3));
	out.write(
		(char*)&item.extents, sizeof(CacheVec3));

	for (size_t i = 0; i < header.meshCount; i++) {
		MeshEntry meshEntry{};
		meshEntry.meshIndex = header.meshOffset;
		meshEntry.indexCount = header.meshCount;
		//meshEntry.materialIndex = item.materialData.materialIndices[i];

		out.write((char*)&meshEntry, sizeof(meshEntry));
	}
}

void Cache::loadCache(std::string originalPath, std::string cachePath, ItemInterface& item) {
	 
	std::ifstream in(cachePath, std::ios::binary);

	CacheHeader header;
	in.read((char*)&header, sizeof(header));

	// - fill in global heavy-data
	// - fill in item meta data
	m_Buffer.globalVertices.reserve(header.vertexCount);
	m_Buffer.globalIndices.reserve(header.indexCount);

	in.seekg(header.vertexOffset);
	in.read(
		(char*)m_Buffer.globalVertices.data(), 
		header.vertexCount * sizeof(Vertex)
	);

	in.seekg(header.indexOffset);
	in.read(
		(char*)m_Buffer.globalIndices.data(),
		header.indexCount * sizeof(uint32_t)
	);

	in.seekg(header.meshOffset);

	item.meshData.meshOffset.resize(header.meshCount);
	item.meshData.indexCount.resize(header.meshCount);

	CacheVec3 center{};
	CacheVec3 extents{};

	in.read(
		(char*)&center, sizeof(CacheVec3)
	);
	in.read(
		(char*)&extents, sizeof(CacheVec3)
	);

	item.center = glm::vec3(center.x, center.y, center.z);
	item.extents = glm::vec3(extents.x, extents.y, extents.z);

	for (size_t i = 0; i < header.meshCount; i++) {
		MeshEntry entry;
		in.read((char*)&entry, sizeof(entry));

		item.meshData.meshOffset[i] = entry.meshIndex;
		item.meshData.indexCount[i] = entry.indexCount;
	}
}

bool Cache::cacheExists(const std::string path) {

	if (!std::filesystem::exists(path))
		return false;

	std::ifstream in(path, std::ios::binary);
	CacheHeader header{};
	in.read((char*)&header, sizeof(header));
	std::cout << header.magic << "\n";
	std::cout << path << "\n";
	if (header.magic != 0x4D4F444C)
		throw std::runtime_error("Invalid cache file");

	if (header.version != 1)
		throw std::runtime_error("Cache version is wrong");

	uint64_t currentHash = Utils::hashFile(path);

	return currentHash == header.sourceHash;
}