#include "Cache.h"

Cache::Cache(Buffer& buffer, Animator& animator) : m_Buffer(buffer), m_Animator(animator) {}

void Cache::writeCache(std::string originalPath, std::string cachePath, ItemInterface& item, int meshOffset) {

	std::ofstream out(cachePath, std::ios::binary);

	CacheHeader header{};
	header.sourceHash = hashFile(sourcePath);

	header.vertexCount = m_Buffer.globalVertices.size();
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

	for (size_t i = 0; i < header.meshCount; i++) {
		MeshEntry meshEntry{};
		meshEntry.meshIndex = header.meshOffset;
		meshEntry.indexCount = header.meshCount;
		meshEntry.materialIndex = item.materialData.materialIndices[i];

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
}

bool Cache::cacheExists(const std::string path) {

	if (!std::filesystem::exists(path))
		return false;

	std::ifstream in(path, std::ios::binary);
	CacheData header;
	in.read((char*)&header, sizeof(header));

	if (header.magic != 0x4D4F444C)
		return false;

	if (header.version != 1)
		return false;

	uint64_t currentHash = hashFile(path);

	return currentHash == header.sourceHash;
}