#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel(Client& owner, std::string title)
    : _owner(&owner), _title(title), _userLimit(0),
      _inviteOnly(false), _topicRestricted(false) {}

void Channel::addMember(Client& client)
{
    if (!hasMember(client))
        _members.push_back(&client);
}

void Channel::removeMember(Client& client)
{
    std::vector<Client*>::iterator	it;

    it = std::find(_members.begin(), _members.end(), &client);
    if (it != _members.end())
        _members.erase(it);

    it = std::find(_operators.begin(), _operators.end(), &client);
    if (it != _operators.end())
        _operators.erase(it);
}

void Channel::addOperator(Client& client)
{
    if (hasMember(client) && std::find(_operators.begin(), _operators.end(), &client) == _operators.end())
        _operators.push_back(&client);
}

void Channel::setPassword(std::string password)
{
    _password = password;
}

std::string Channel::getTitle() const
{
    return (_title);
}

std::string Channel::getTopic() const
{
    return (_topic);
}

std::string Channel::getPassword() const
{
    return (_password);
}

bool Channel::isInviteOnly() const
{
    return (_inviteOnly);
}

bool Channel::isTopicRestricted() const
{
    return (_topicRestricted);
}

bool Channel::hasPassword() const
{
    return (!_password.empty());
}

bool Channel::hasUserLimit() const
{
    return (_userLimit > 0);
}

bool Channel::hasMember(const Client& client) const
{
    if (std::find(_members.begin(), _members.end(), &client) != _members.end())
        return (true);
    return (false);
}

bool Channel::hasOperator(const Client& client) const
{
    if (std::find(_operators.begin(), _operators.end(), &client) != _operators.end())
        return (true);
    return (false);
}

size_t Channel::getUserLimit() const
{
    return (_userLimit);
}

size_t Channel::getMembersCount() const
{
    return (_members.size());
}
const std::vector<Client*>& Channel::getMembers() const
{
    return (_members);
}