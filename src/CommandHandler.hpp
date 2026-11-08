#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Message.hpp"
#include "Client.hpp"

class Server;

class CommandHandler
{
public:
    void handle(Server& server, Client& client, const Message& message);
};

#endif