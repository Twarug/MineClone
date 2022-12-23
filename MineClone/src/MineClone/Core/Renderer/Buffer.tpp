#pragma once
#include "Buffer.h"

#include "RendererAPI.h"

namespace mc
{
    template <typename Vertex>
    Ref<Buffer> Buffer::CreateVertexBuffer(std::span<Vertex> data) {
        u64 size = sizeof(Vertex) * data.size();

        auto stageBuffer = CreateStageBuffer(size, data.data());

        auto vertexBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        RendererAPI::CopyBuffer(stageBuffer, vertexBuffer, size);

        return vertexBuffer;
    }

    template <std::integral Index>
    Ref<Buffer> Buffer::CreateIndexBuffer(std::span<Index> data) {
        u64 size = sizeof(Index) * data.size();

        auto stageBuffer = CreateStageBuffer(size, data.data());

        auto vertexBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        RendererAPI::CopyBuffer(stageBuffer, vertexBuffer, size);

        return vertexBuffer;
    }
}
