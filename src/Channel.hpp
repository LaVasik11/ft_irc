#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>

class Client;

class Channel
{
public:
    Channel(Client& owner, std::string title);

    void        addMember(Client& client);
    void        removeMember(Client& client);
    void        addOperator(Client& client);
    void        setPassword(std::string password);

    std::string getTitle() const;
    std::string getTopic() const;
    std::string getPassword() const;

    bool        isInviteOnly() const;
    bool        isTopicRestricted() const;
    bool        hasPassword() const;
    bool        hasUserLimit() const;
    bool        hasMember(const Client& client) const;

    size_t      getUserLimit() const;
    size_t      getMembersCount() const;

private:
    Client*                 _owner;
    std::string             _title;
    std::string             _topic;
    std::string             _password;
    int                     _userLimit;
    bool                    _inviteOnly;
    bool                    _topicRestricted;
    std::vector<Client*>    _members;
    std::vector<Client*>    _operators;
};

#endif