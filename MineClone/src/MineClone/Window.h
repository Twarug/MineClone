#pragma once

namespace mc
{
    class Window
    {
    public:
        Window(u32 width, u32 height, std::string_view title);
        ~Window();

        void Update() const;
        
        void Resize(u32 width, u32 height);
        
        uint2 GetSize() const { return m_size; }
        u32 GetWidth() const { return m_size.x; }
        u32 GetHeight() const { return m_size.y; }
        
        void* GetNativeWindow() { return m_nativeWindow; }
        const void* GetNativeWindow() const { return m_nativeWindow; }
        
    private:
        void* m_nativeWindow{};
        uint2 m_size{};
        
    private:
        inline static u32 s_windowCount = 0;
    };
}
