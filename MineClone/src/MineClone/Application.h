#pragma once

namespace mc
{
    class Application
    {
    public:
        std::string name;
    
    public:
        Application(std::string_view name);
        
        void Run();

    public:
        static Application& Get() { return *s_instance; }

    private:
        inline static Application* s_instance = nullptr;
    };
}
