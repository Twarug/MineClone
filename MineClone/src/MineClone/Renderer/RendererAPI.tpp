#pragma once

#include "RendererAPI.h"

namespace mc
{
    template <typename Vertex>
    Ref<AllocatedBuffer> RendererAPI::CreateVertexBuffer(std::span<Vertex> data) {
        u64 size = sizeof(Vertex) * data.size();

        auto stageBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data.data());

        auto vertexBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 

        CopyBuffer(stageBuffer, vertexBuffer, size);

        DeleteBuffer(stageBuffer);

        return vertexBuffer;
    }

    template <std::integral Index>
    Ref<AllocatedBuffer> RendererAPI::CreateIndexBuffer(std::span<Index> data) {
        u64 size = sizeof(Index) * data.size();

        auto stageBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data.data());

        auto vertexBuffer = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 

        CopyBuffer(stageBuffer, vertexBuffer, size);

        DeleteBuffer(stageBuffer);

        return vertexBuffer;
    }
}
