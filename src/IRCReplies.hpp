#ifndef IRC_REPLIES_HPP
#define IRC_REPLIES_HPP

namespace IRC
{
    enum ReplyCode
    {
        RPL_WELCOME = 1,
        ERR_NOSUCHNICK = 401,
        ERR_UNKNOWNCOMMAND = 421,
        ERR_NONICKNAMEGIVEN = 431,
        ERR_ERRONEUSNICKNAME = 432,
        ERR_NICKNAMEINUSE = 433,
        ERR_NEEDMOREPARAMS = 461
    };
}

#endif