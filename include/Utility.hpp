/// Common utilities module

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <cctype>
#include <fstream>

// Output log file, ugly solution global for logging
// into files! This approach is not recommended, I
// used here to focus on JPEG, not logging! :p
extern std::ofstream logFile;

namespace cppeg
{
    namespace utils
    {
        /// String helpers
        /// Check whether a character is a whitespace
        ///
        /// @param ch the character to check for whitespace
        /// @return true if character is whitespace, else return false
        inline const bool isWhiteSpace(const char ch)
        {
            return iscntrl(ch) || isblank(ch) || isspace(ch);
        }

        /// Check whether a string consists entirely of whitespaces
        ///
        /// @param str string under test
        /// @return true if the specified string consists entirely of whitespaces, else false
        inline const bool isStringWhiteSpace(const std::string &str)
        {
            for (auto &&c : str)
                if (!isWhiteSpace(c))
                    return false;
            return true;
        }
    }
}

#endif // UTILITY_HPP
