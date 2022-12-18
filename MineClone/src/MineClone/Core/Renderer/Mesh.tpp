#pragma once
#include "Mesh.h"
#include "RendererAPI.h"

namespace mc
{
    template <typename T>
    void Mesh::SetVertices(std::span<T> vertices) {
        u32 newCount = (u32)vertices.size();
        
        if(m_vertexBufferCap < newCount) {
            // Create/recreate buffer
            if(m_vertexBufferCap > 0)
                RendererAPI::DeleteBuffer(m_vertexBuffer);
                
            m_vertexBuffer = RendererAPI::CreateVertexBuffer<T>(vertices);
            m_vertexBufferCap = newCount;
        }
        else if(newCount > 0) {
            // Update Data
            u64 size = newCount * sizeof(T);
            auto stageBuffer = RendererAPI::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertices.data());

            RendererAPI::CopyBuffer(stageBuffer, m_vertexBuffer, size);

            RendererAPI::DeleteBuffer(stageBuffer);
        }

        m_vertexCount = newCount;
    }
}
