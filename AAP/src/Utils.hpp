#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace AAP {
    bool DoesFilesExist(const std::vector<std::string>& files) {
        for (int i = 0; i < files.size(); ++i) {
            if (!std::filesystem::exists(files[i]))
                return false;
        }
        return true;
    }
} // namespace AAP
