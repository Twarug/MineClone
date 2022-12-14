#include "mcpch.h"
#include "Application.h"

#include "MineClone/Core/Event/ApplicationEvents.h"
#include "MineClone/Core/Renderer/RendererAPI.h"
#include "MineClone/Core/Renderer/RendererTypes.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    struct Vertex2D
    {
        float2 pos;
        float3 color;

        static VertexDescription GetDescription() {
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
    };

    struct Vertex3D
    {
        float3 pos;
        float3 normal;
        float3 color;
        float2 uv;

        static VertexDescription GetDescription() {
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
    };

    static constexpr std::array<std::array<Vertex3D, 4>, 6> VERTICES = {
        {
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
        }
    };

    static const std::vector<u32> INDICES = {
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
    };

    Ref<AllocatedBuffer> g_vertexBuffer;
    Ref<AllocatedBuffer> g_indexBuffer;

    Ref<Texture> g_texture;

    Ref<Material> g_mat;

    Application::Application(std::string_view name)
        : name(name), m_isRunning(false) {
        s_instance = this;

        m_cubeTransform = translate(Mat4{1}, {-2.5f, 0, 0});
        m_cube2Transform = translate(Mat4{1}, {2.5f, 0, 0});
    }


    void Application::Run() {
        m_isRunning = true;

        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<FirstPersonCamera>(60.f, 1280, 720);
        m_camera->SetPos({0, 1, 10});
        m_camera->SetRot({-30, 0});

        RendererAPI::Init();

        m_world = CreateScope<World>();
        
        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(VERTICES.data(), VERTICES.size() * VERTICES[0].size()));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(INDICES));

        g_texture = RendererAPI::LoadTexture("assets/texture.png");
        auto des = Vertex3D::GetDescription();
        g_mat = Material::Create("default", des);
        g_mat->SetTexture(g_texture);

        while(m_isRunning) {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, *m_camera);
            Render();
            RendererAPI::EndFrame();
        }

        RendererAPI::Wait();
        g_mat = nullptr;
        RendererAPI::DeleteTexture(g_texture);

        RendererAPI::DeleteBuffer(g_vertexBuffer);
        RendererAPI::DeleteBuffer(g_indexBuffer);

        RendererAPI::Deinit();
    }

    void Application::Update() {
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            m_deltaTime = duration<float, seconds::period>(now - m_lastFrameTimePoint).count();
            m_lastFrameTimePoint = now;

            // printf("%f\n", .01f *m_deltaTime);
        }

        m_window->Update();
        {
            AppUpdateEvent ev{m_deltaTime};
            EventHandler<AppUpdateEvent>::Invoke(ev);
        }

        m_camera->Update(m_deltaTime);
        m_cubeTransform *= rotate(Mat4{1}, glm::radians(10.f * m_deltaTime), {0, 1, 0});

        m_world->Update();
    }

    void Application::Render() {
        g_mat->Bind();
        m_world->Render();
        RendererAPI::Draw(m_cubeTransform, g_vertexBuffer, g_indexBuffer, static_cast<u32>(INDICES.size()));
        RendererAPI::Draw(m_cube2Transform, g_vertexBuffer, g_indexBuffer, static_cast<u32>(INDICES.size()));
    }

    void Application::OnEvent(WindowCloseEvent& ev) {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

    void Application::OnEvent(WindowResizeEvent& ev) {
        RendererAPI::Resize(ev.GetWidth(), ev.GetHeight());
        m_camera->OnResize(ev.GetWidth(), ev.GetHeight());
    }
}
