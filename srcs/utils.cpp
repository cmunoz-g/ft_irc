#include "IRC.hpp"

void error(const std::string& errorMsg, bool throws, bool usesErrno) {
    std::string formatMessage;

    if (usesErrno) { 
        formatMessage += "[" + std::string(strerror(errno)) + "]: ";
    }

    if (throws)
        throw std::runtime_error(std::string(RED) + "[ERROR] " + formatMessage + std::string(RESET) + errorMsg);
    else
        std::cerr << RED << "[WARNING] " << formatMessage << RESET << errorMsg << std::endl;
}

int stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int num = 0;
    ss >> num;
    return num;
}

bool isValidMode(char mode, bool isChannelMode) {
    const std::string validChannelModes = "oiktl";  // Allowed channel modes
    const std::string validUserModes = "io";        // Allowed user modes

    if (isChannelMode)
        return validChannelModes.find(mode) != std::string::npos;
    else
        return validUserModes.find(mode) != std::string::npos;
}

bool isValidNick(const std::string &nick) {
    if (nick.empty())
        return false;
    
    // Check first character: must be a letter or one of the special characters.
    char c = nick[0];
    bool validFirst = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) ||
                      (c == '[' || c == ']' || c == '\\' || c == '`' || 
                       c == '_' || c == '^' || c == '{' || c == '}' || c == '|');
    if (!validFirst)
        return false;

    // Check subsequent characters: letters, digits, special characters, or hyphen.
    for (size_t i = 1; i < nick.size(); i++) {
        char c = nick[i];
        bool valid = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) ||
                     (c == '[' || c == ']' || c == '\\' || c == '`' || 
                      c == '_' || c == '^' || c == '{' || c == '}' || c == '|') ||
                     (c == '-');
        if (!valid)
            return false;
    }
    
    // RFC 2812 defines nickname as 1 to 9 characters long.
    if (nick.size() > 9)
        return false;

    return true;
}