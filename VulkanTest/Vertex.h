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

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attr{};
        //attr[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) };
        //attr[1] = { 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) };
        //attr[2] = { 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) };

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

        return attr;
    }
};
