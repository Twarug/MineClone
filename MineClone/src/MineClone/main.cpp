#include "mcpch.h"

#include "MineClone/Application.h"

int main(int argc, char* argv[]) {
    try {
        mc::Application* app = new mc::Application("MineClone");
        app->Run();
    }
    catch(std::exception& ex) {
        std::cout << ex.what();
    }

    return 0;
}
