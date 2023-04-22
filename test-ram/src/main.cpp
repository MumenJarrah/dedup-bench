#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>
#include <chrono>
#include <cmath>
#include <iomanip>


std::unique_ptr<std::istream> read_file_to_buffer(std::string f_path, uint32_t multiplier) {
    // buffer of size 1MiB
    const uint64_t buffer_size = pow(2, 20);
    auto buffer = std::make_unique<char[]>(buffer_size);

    std::filesystem::path file_path{f_path};
    std::ifstream file_ptr;
    file_ptr.open(file_path, std::ios::in | std::ios::binary);
    // get length of file:
    file_ptr.seekg(0, std::ios::end);
    uint64_t length = file_ptr.tellg();
    uint64_t final_length = multiplier * length;
    std::cout << "Test file has " << length << " bytes, ";
    std::cout << "will repeat it " << multiplier << " times ";
    std::cout << "to get a buffer of size " << (double)final_length / pow(10, 9) << " GB" << std::endl;;
    auto ss = std::make_unique<std::stringstream>();

    for (uint32_t i = 0; i < multiplier; ++i) {
        file_ptr.seekg(0, std::ios::beg);
        uint64_t curr_bytes_read = 0;
        uint64_t bytes_to_read = std::min(buffer_size, length);
        while(curr_bytes_read < length) {
            // read data from file into the buffer
            file_ptr.read(buffer.get(), bytes_to_read);
            ss->write(buffer.get(), bytes_to_read);
            curr_bytes_read += bytes_to_read;
            bytes_to_read = std::min(buffer_size, length - curr_bytes_read);
        }
    }
    std::cout << "Done reading file" << std::endl;

    // cleanup
    file_ptr.close();
    return ss;
}

int main(int argc, char * argv[]){
    if(argc != 3){
        std::cout << "Usage: ./test-ram.exe <file-path> <multiplier>" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string file_path = std::string(argv[1]);
    uint32_t multiplier = std::stoul(argv[2]);

    // read the file into an istream
    std::unique_ptr<std::istream> stream = read_file_to_buffer(file_path, multiplier);
    char read_byte = 'a';
    // read the first byte to bring the page with the beginning of the buffer into RAM
    // and try evict the page containing the end of the buffer
    stream->read(&read_byte, 1);
    std::cout << "First byte (hex value): " << std::setw(2) << std::setfill('0') << std::hex << (int)read_byte << std::endl;
    stream->seekg(0, std::ios::beg);

    // measure the time it takes to seek to the last byte of the buffer and read it
    auto begin = std::chrono::high_resolution_clock::now();
    stream->seekg(-1, std::ios::end);
    stream->read(&read_byte, 1);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> us_double = end - begin;
    std::cout << "Last byte (hex value): " << std::setw(2) << std::setfill('0') << std::hex << (int)read_byte << std::endl;
    std::cout << "Time taken (microseconds): " << us_double.count() << std::endl;
    exit(EXIT_SUCCESS);
}
