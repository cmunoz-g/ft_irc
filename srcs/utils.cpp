#include "IRC.hpp"

void error(const std::string& errorMsg, bool throws, bool usesErrno) {
    std::string formatMessage;

    if (usesErrno) { 
        formatMessage += "[" + std::string(strerror(errno)) + "]: ";
    }
    else {
        formatMessage += "[No errno]: ";
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
