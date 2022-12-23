#include "mcpch.h"
#include "Mesh.h"

#include "RendererAPI.h"

namespace mc
{
    Mesh::~Mesh() {
        Dispose();
    }

    void Mesh::Render(const Mat4& transform) const {
        if(m_vertexBuffer && m_indexBuffer)
            RendererAPI::Draw(transform, m_vertexBuffer, m_indexBuffer, m_indicesCount);
    }

    void Mesh::SetIndices(std::span<const u32> indices) {
        u32 newCount = (u32)indices.size();

        if(m_indexBufferCap < newCount) {
            // Create/recreate buffer
            m_indexBuffer = Buffer::CreateIndexBuffer(indices);
            m_indexBufferCap = newCount;
        }
        else if(newCount > 0) {
            // Update Data
            u64 size = newCount * sizeof(u32);
            auto stageBuffer = Buffer::CreateStageBuffer(size, indices.data());

            RendererAPI::CopyBuffer(stageBuffer, m_indexBuffer, size);
        }

        m_indicesCount = newCount;
    }

    void Mesh::Dispose() {
        if(m_indexBuffer)
            m_indexBuffer->Delete();

        if(m_vertexBuffer)
            m_vertexBuffer->Delete();

        m_indexBuffer = nullptr;
        m_vertexBuffer = nullptr;
        
        m_indexBufferCap = 0;
        m_vertexBufferCap = 0;
        m_indicesCount = 0;
        m_vertexCount = 0;
    }
}
