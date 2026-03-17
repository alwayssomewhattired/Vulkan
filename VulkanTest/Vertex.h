#pragma once
#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>
#include <cstddef>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::ivec4 boneIDs;
    glm::vec4 weights;

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(Vertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDesc;
    }

    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 6> attr{};

        attr[0].location = 0;
        attr[0].binding = 0;
        attr[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr[0].offset = offsetof(Vertex, pos);

        attr[1].location = 1;
        attr[1].binding = 0;
        attr[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr[1].offset = offsetof(Vertex, color);

        attr[2].location = 2;
        attr[2].binding = 0;
        attr[2].format = VK_FORMAT_R32G32_SFLOAT;
        attr[2].offset = offsetof(Vertex, texCoord);

        attr[3].location = 3;
        attr[3].binding = 0;
        attr[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr[3].offset = offsetof(Vertex, normal);

        attr[4].location = 4;
        attr[4].binding = 0;
        attr[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr[4].offset = offsetof(Vertex, tangent);

        attr[5].location = 5;
        attr[5].binding = 0;
        attr[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr[5].offset = offsetof(Vertex, bitangent);

        return attr;
    }
};
