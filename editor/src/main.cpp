#include "draft/core/crash_handler.hpp"
#include "draft/editor/editor_application.hpp"

#include <filesystem>

using namespace Draft;

int main(int argc, char** argv){
    CrashHandler::install();

    EditorApplication editor("Draft Editor", 1600, 900);

    if(argc > 1)
        editor.request_open_project(std::filesystem::path(argv[1]));

    editor.run();
    return 0;
}
