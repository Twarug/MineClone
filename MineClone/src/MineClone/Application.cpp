#include "mcpch.h"
#include "Application.h"


namespace mc
{
    Application::Application(std::string_view name)
        :name(name)
    {}

    
    void Application::Run()
    {
        std::cout << "Run App\n";
    }

}
