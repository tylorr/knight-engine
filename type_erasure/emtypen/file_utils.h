#ifndef FILE_UTILS_INCLUDED__
#define FILE_UTILS_INCLUDED__

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <algorithm>

inline std::string file_slurp(const std::string &filename)
{
    std::string retval;

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs.seekg(0, std::ifstream::end);

    if (0 < ifs.tellg())
        retval.resize(ifs.tellg());
    ifs.seekg(0);

    ifs.read(&retval[0], retval.size());

    if (!ifs) {
        std::cout << errno << ": " << std::strerror(errno) << "\n";
        std::cout << "error: only " << ifs.gcount() << " could be read\n";
    }

    return retval;
}

inline std::vector<std::string> line_break (const std::string & file)
{
    std::vector<std::string> retval;

    std::string::size_type prev_pos = 0;
    while (true) {
        std::string::size_type pos = file.find('\n', prev_pos);
        if (pos == std::string::npos)
            break;
        retval.push_back(file.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + 1;
    }

    retval.push_back(file.substr(prev_pos));

    return retval;
}

#endif
