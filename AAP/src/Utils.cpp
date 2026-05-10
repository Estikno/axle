#include "Types.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <vector>
#include <cstdio>

namespace AAP {
    bool DoesFilesExist(const std::vector<std::string>& files) {
        for (int i = 0; i < files.size(); ++i) {
            const std::filesystem::path path = files[i];
            if (!std::filesystem::exists(path))
                return false;
        }
        return true;
    }

    std::vector<u8> ReadFile(const char* filename) {
        // Open the file
        const std::filesystem::path path = filename;
        if (!std::filesystem::exists(path)) {
            fprintf(stderr, "ReadFile: File does not exist'%s'\n", filename);
            return {};
        }

        std::ifstream file(path, std::ios::binary);

        if (!file.is_open()) {
            fprintf(stderr, "ReadFile: could not open '%s'\n", filename);
            return {};
        }

        // Stop eating new lines in binary mode
        file.unsetf(std::ios::skipws);

        // Get its size
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();

        if (fileSize <= 0) {
            fprintf(stderr, "ReadFile: invalid file size for '%s'\n", filename);
            return {};
        }

        file.seekg(0, std::ios::beg);

        // Construct the vector
        std::vector<u8> buf(fileSize);
        file.read((char*) buf.data(), fileSize);

        if (!file.good()) {
            fprintf(stderr, "ReadFile: failed to read '%s'\n", filename);
            return {};
        }

        return buf;
    }

    bool WriteFile(const char* filename, u8* buf, u32 size) {
        // Open file
        const std::filesystem::path path = filename;
        std::ofstream file(path, std::ios::binary);

        if (!file.is_open()) {
            fprintf(stderr, "WriteFile: could not open '%s'\n", filename);
            return false;
        }

        file.write((char*) buf, size);

        if (!file.good()) {
            fprintf(stderr, "WriteFile: failed to write to '%s'\n", filename);
            return false;
        }

        return true;
    }
} // namespace AAP
