#include "mcpch.h"
#include "Config.h"

#include "Core/Renderer/RendererTypes.h"
#include <vulkan/vulkan_core.h>


namespace mc
{
    VertexDescription Vertex2D::GetDescription() {
        return {
            .bindingDescription = {
                .binding = 0,
                .stride = sizeof(Vertex2D),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
            .attributeDescriptions = {
                {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex2D, pos),
                },
                {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex2D, color),
                },
            },
        };
    }

    VertexDescription Vertex3D::GetDescription() {
        return {
            .bindingDescription = {
                .binding = 0,
                .stride = sizeof(Vertex3D),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
            .attributeDescriptions = {
                {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, pos),
                },
                {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, normal),
                },
                {
                    .location = 2,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, color),
                },
                {
                    .location = 3,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex3D, uv),
                },
            },
        };
    }


    const std::array<std::array<Vertex3D, 4>, 6> Config::VERTICES = {{
        // Top
        {
            {
                {{0, 1, 1}, {0, 1, 0}, {1, 1, 1}, {0, 0}}, // 0
                {{1, 1, 1}, {0, 1, 0}, {1, 1, 1}, {1, 0}}, // 1
                {{0, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1}}, // 2
                {{1, 1, 0}, {0, 1, 0}, {1, 1, 1}, {1, 1}}, // 3
            }
        },

        // Bottom
        {
            {
                {{0, 0, 0}, {0, -1, 0}, {1, 1, 1}, {0, 0}}, // 4
                {{1, 0, 0}, {0, -1, 0}, {1, 1, 1}, {1, 0}}, // 5
                {{0, 0, 1}, {0, -1, 0}, {1, 1, 1}, {0, 1}}, // 6
                {{1, 0, 1}, {0, -1, 0}, {1, 1, 1}, {1, 1}}, // 7
            }
        },

        // Front
        {
            {
                {{0, 0, 1}, {0, 0, 1}, {1, 1, 1}, {0, 0}}, // 8
                {{1, 0, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0}}, // 9
                {{0, 1, 1}, {0, 0, 1}, {1, 1, 1}, {0, 1}}, // 10
                {{1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 1}}, // 11
            }
        },

        // Back
        {
            {
                {{1, 0, 0}, {0, 0, -1}, {1, 1, 1}, {0, 0}}, // 12
                {{0, 0, 0}, {0, 0, -1}, {1, 1, 1}, {1, 0}}, // 13
                {{1, 1, 0}, {0, 0, -1}, {1, 1, 1}, {0, 1}}, // 14
                {{0, 1, 0}, {0, 0, -1}, {1, 1, 1}, {1, 1}}, // 15
            }
        },

        // Right
        {
            {
                {{1, 0, 1}, {1, 0, 0}, {1, 1, 1}, {0, 0}}, // 16
                {{1, 0, 0}, {1, 0, 0}, {1, 1, 1}, {1, 0}}, // 17
                {{1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {0, 1}}, // 18
                {{1, 1, 0}, {1, 0, 0}, {1, 1, 1}, {1, 1}}, // 19
            }
        },

        // Left
        {
            {
                {{0, 0, 0}, {-1, 0, 0}, {1, 1, 1}, {0, 0}}, // 20
                {{0, 0, 1}, {-1, 0, 0}, {1, 1, 1}, {1, 0}}, // 21
                {{0, 1, 0}, {-1, 0, 0}, {1, 1, 1}, {0, 1}}, // 22
                {{0, 1, 1}, {-1, 0, 0}, {1, 1, 1}, {1, 1}}, // 23
            }
        },
    }};

    const std::array<u32, 6ull * 6ull> Config::INDICES = {{
        // Top
        0, 3, 1,
        0, 2, 3,

        // Bottom
        4, 7, 5,
        4, 6, 7,

        // Front
        8, 11, 9,
        8, 10, 11,

        // Back
        12, 15, 13,
        12, 14, 15,

        // Right
        16, 19, 17,
        16, 18, 19,

        // Left
        20, 23, 21,
        20, 22, 23,
    }};
}
