#include "ReflectionGenerator.h"
#include <print>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::println("ReflectionTool: No header files provided.");

        return 0;
    }

    bool shouldForce = false;
    if (argc >= 4)
    {
        const std::string forceArg = argv[3];
        if (forceArg == "-f" || forceArg == "-force")
        {
            shouldForce = true;
        }
    }

    std::filesystem::path directory = argv[2];
    const std::filesystem::path output = directory / "Reflection.generated.h";
    if (!exists(output))
    {
        shouldForce = true;
    }

    ReflectionGenerator generator(argv[1]);
    if (generator.ProcessDirectoryRecursive(directory, shouldForce))
    {
        std::println("ReflectionTool: Successfully generated reflection info.");
    }
    else
    {
        std::println("ReflectionTool: Failed to generate reflection info.");
        return -1;
    }
    generator.GenerateReflectionInitializer(directory, shouldForce);

    return 0;
}
