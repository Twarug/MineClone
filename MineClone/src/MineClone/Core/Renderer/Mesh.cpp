#include "mcpch.h"
#include "Mesh.h"

#include "RendererAPI.h"

namespace mc
{
    Mesh::Mesh() = default;
    Mesh::~Mesh() {
        Dispose();
    }

    void Mesh::Render(const Mat4& transform) const {
        if(m_vertexBuffer && m_indexBuffer)
            RendererAPI::Draw(transform, m_vertexBuffer, m_indexBuffer, m_indicesCount);
    }

    void Mesh::SetIndices(std::span<u32> indices) {
        u32 newCount = static_cast<u32>(indices.size());

        if(m_indicesCount == 0) {
            if (newCount > 0)
               m_indexBuffer = RendererAPI::CreateIndexBuffer(indices);
        }
        else
            throw std::exception("TODO: add buffer updating");

        m_indicesCount = newCount;
    }

    void Mesh::Dispose() {
        if(m_indexBuffer)
            RendererAPI::DeleteBuffer(m_indexBuffer);

        if(m_vertexBuffer)
            RendererAPI::DeleteBuffer(m_vertexBuffer);

        m_indicesCount = 0;
        m_vertexBufferSize = 0;
    }
}
