// COMMMAND /proc/[pid]/comm
// PID      /proc/[pid]
// UID      /proc/[pid]/status -> UID (id -nu </proc/[pid]/loginuid) 
// FD
// TYPE
// NODE
// NAME
#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    std::string path = "/proc";
    for (const auto & entry : fs::directory_iterator(path))
        std::cout << entry.path() << std::endl;
}