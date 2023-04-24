/**
 * @file chunking_common.cpp
 * @author WASL
 * @brief Implementations of functions common across all chunking techniques
 * @version 0.1
 * @date 2023-02-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "chunking_common.hpp"
#include <memory>
#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cmath>


File_Chunk::File_Chunk(uint64_t _chunk_size) {
    chunk_size = _chunk_size;
    chunk_data = std::make_unique<char[]>(_chunk_size);
}

File_Chunk::File_Chunk(const File_Chunk& other): File_Chunk(other.chunk_size) {
    memcpy(this->chunk_data.get(), other.chunk_data.get(), this->chunk_size);
    if (other.chunk_hash) {
        this->chunk_hash = std::make_unique<Hash>(*other.chunk_hash);
    }
}

File_Chunk::File_Chunk(File_Chunk&& other) noexcept {
    this->chunk_size = other.chunk_size;
    this->chunk_data = std::move(other.chunk_data);
    this->chunk_hash = std::move(other.chunk_hash);
    // release ownership of the data in the other object
    other.chunk_data.release();
    other.chunk_hash.release();
}

uint64_t File_Chunk::get_size() const {
    return chunk_size;
}

char* File_Chunk::get_data() const {
    return chunk_data.get();
}

BYTE* File_Chunk::get_hash() const {
    if (chunk_hash) {
        return chunk_hash->getHash();
    }
    return nullptr;
}

void File_Chunk::init_hash(HashingTech hashing_tech, uint64_t size) {
    chunk_hash = std::make_unique<Hash>(hashing_tech, size);
}

std::string File_Chunk::to_string() const {
    if (chunk_hash) {
        return chunk_hash->toString() + "," + std::to_string(chunk_size);
    }
    return "INVALID HASH";
}

void File_Chunk::print() const {
    std::cout << "\tChunk Size: " << chunk_size << std::endl;
    if (chunk_hash) {
        std::cout << "\tChunk Hash: " << chunk_hash->toString() << std::endl;
    }
    std::cout << "\tChunk Data: ";
    for (uint64_t i = 0; i < chunk_size; ++i) {
        // this only works if the data is ASCII
        std::cout << chunk_data[i];
    }
    std::cout << std::endl;
}

// ========== Chunking_Techniques =============
std::vector<std::unique_ptr<std::istream>> Chunking_Technique::read_files_to_buffers(std::string dir_path) {
    std::vector<std::unique_ptr<std::istream>> buffers;
    // buffer of size 1MiB
    const uint64_t buffer_size = pow(2, 20);
    auto buffer = std::make_unique<char[]>(buffer_size);

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
        std::filesystem::path file_path = entry.path();
        if (std::filesystem::is_directory(file_path)) {
            continue;
        }

        std::ifstream file_ptr;
        file_ptr.open(file_path, std::ios::in | std::ios::binary);
        // get length of file:
        file_ptr.seekg(0, std::ios::end);
        uint64_t length = file_ptr.tellg();
        file_ptr.seekg(0, std::ios::beg);
        auto ss = std::make_unique<std::stringstream>();

        uint64_t curr_bytes_read = 0;
        uint64_t bytes_to_read = std::min(buffer_size, length);
        while(curr_bytes_read < length) {
            // read data from file into the buffer
            file_ptr.read(buffer.get(), bytes_to_read);
            ss->write(buffer.get(), bytes_to_read);
            curr_bytes_read += bytes_to_read;
            bytes_to_read = std::min(buffer_size, length - curr_bytes_read);
        }
        buffers.emplace_back(std::move(ss));

        // cleanup
        file_ptr.close();
    }
    return buffers;
}