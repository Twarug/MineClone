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
            m_vertexBuffer = Buffer::CreateVertexBuffer<T>(vertices);
            m_vertexBufferCap = newCount;
        }
        else if(newCount > 0) {
            // Update Data
            u64 size = newCount * sizeof(T);
            
            auto stageBuffer = Buffer::CreateStageBuffer(size, vertices.data());
            RendererAPI::CopyBuffer(stageBuffer, m_vertexBuffer, size);
        }

        m_vertexCount = newCount;
    }
}
