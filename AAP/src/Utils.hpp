#pragma once

#include <vector>
#include <string>

#include "Types.hpp"

namespace AAP {
    bool DoesFilesExist(const std::vector<std::string>& files);

    /**
     * Reads the binary contents of a file into a vector
     *
     * @param filename Path to the desired file
     * */
    std::vector<u8> ReadFile(const char* filename);

    bool WriteFile(const char* filename, u8* buf, u32 size);
} // namespace AAP
