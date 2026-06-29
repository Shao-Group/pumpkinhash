#ifndef PATH_UTILS_HPP
#define PATH_UTILS_HPP

#include <string>
#include <sys/stat.h>
#include <sys/types.h>

inline std::string pathJoin(const std::string &leftPath, const std::string &rightPath)
{
    if (leftPath.empty())
    {
        return rightPath;
    }

    if (leftPath[leftPath.size() - 1] == '/')
    {
        return leftPath + rightPath;
    }

    return leftPath + "/" + rightPath;
}

inline bool pathExists(const std::string &path)
{
    struct stat pathStat;

    return stat(path.c_str(), &pathStat) == 0;
}

inline bool createDirectories(const std::string &path)
{
    if (pathExists(path))
    {
        return true;
    }

    const std::size_t separatorPosition = path.find_last_of('/');

    if (separatorPosition != std::string::npos && separatorPosition > 0)
    {
        if (!createDirectories(path.substr(0, separatorPosition)))
        {
            return false;
        }
    }

    if (mkdir(path.c_str(), 0755) != 0)
    {
        return pathExists(path);
    }

    return true;
}

#endif
