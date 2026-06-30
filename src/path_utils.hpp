#ifndef PATH_UTILS_HPP
#define PATH_UTILS_HPP

#include <string>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

using namespace std;

inline char pathSeparator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

inline bool hasTrailingSeparator(const string &path)
{
    if (path.empty())
    {
        return false;
    }

    const char lastCharacter = path[path.size() - 1];

    return lastCharacter == '/' || lastCharacter == '\\';
}

inline string pathJoin(const string &leftPath, const string &rightPath)
{
    if (leftPath.empty())
    {
        return rightPath;
    }

    if (hasTrailingSeparator(leftPath))
    {
        return leftPath + rightPath;
    }

    return leftPath + pathSeparator() + rightPath;
}

inline bool pathExists(const string &path)
{
    struct stat pathStat;

    return stat(path.c_str(), &pathStat) == 0;
}

inline bool createDirectory(const string &path)
{
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

inline bool createDirectories(const string &path)
{
    if (pathExists(path))
    {
        return true;
    }

    const size_t separatorPosition = path.find_last_of("/\\");

    if (separatorPosition != string::npos && separatorPosition > 0)
    {
        if (!createDirectories(path.substr(0, separatorPosition)))
        {
            return false;
        }
    }

    if (!createDirectory(path))
    {
        return pathExists(path);
    }

    return true;
}

#endif