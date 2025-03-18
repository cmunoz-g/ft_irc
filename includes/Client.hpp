#ifndef CLIENT_HPP
#define CLIENT_HPP

class Channel;

#include "IRC.hpp"

class Client {
private:
    // *** Attributes ***
    int                                     _socket;
    std::string                             _nickname;
    std::string                             _username;
    std::string                             _buffer;
    bool                                    _capNegotiation;    // Client has sent CAP END
    bool                                    _authenticated;     // Client sent the correct PASS
    bool                                    _registered;        // Client has set NICK & USER
    unsigned int                            _passwordAttempts;
    unsigned int                            _id;
    std::vector<unsigned int>               _modes;
    std::map<const std::string, Channel*>   _channels;
    std::map<const std::string, Channel*>   _op_channels;

public:
    // *** Constructor & Destructor ///
    Client(int socket, unsigned int id);
    ~Client();

    // *** Getters, Setters ***
    int					                    getSocket() const;
    std::string const                       getNickname() const;
    std::string const                       getUsername() const;
    unsigned int                            getId() const;
    std::map<const std::string, Channel*>   getChannels() const;
    void                                    setNickname(const std::string& nickname);
    void                                    setUsername(const std::string& username);
    void                                    setAuthenticated(bool status);
    void                                    setRegistered(bool status);
    void                                    setCapNegotiationStatus(bool status);
    void                                    setBuffer(const std::string& buffer);
    
    // *** Member Functions ***
    // Checks
    bool                isAuthenticated() const;
    bool                isRegistered() const;
    bool                isCapNegotiationDone() const;

    // Buffer Operations
    void                appendToBuffer(const std::string& data);
	std::string const   getBuffer() const;
	void		        clearBuffer(void);

    // Channel Operations
    void                joinChannel(Channel *channel);
    void                leaveChannel(const Channel *channel);
    void                leaveAllChannels();
    bool                isInChannel(const Channel *channel) const;
    
    // Operator Operations
    void                setOperatorStatus(Channel *channel);
    void                removeOperatorStatus(const Channel *channel);
    bool                isOperator(const Channel *channel) const;
    
    // Communication
    void                receiveMessage(const std::string &message);
    
    // Mode Operations
    void                setMode(IRC::ClientMode mode, bool enabled);
    void                setModesFromString(const std::string& modeString);
    bool                hasMode(IRC::ClientMode mode) const;
    std::string         getModes() const;
    
    // Password Operations
    unsigned int        getPasswordAttempts() const;
    void                addPasswordAttempt(void);

    // Cleanup
    void                cleanup();

};

#endif