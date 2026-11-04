/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommand.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diespino <diespino@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/03 18:51:17 by diespino          #+#    #+#             */
/*   Updated: 2026/11/04 15:39:22 by dortega-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::handleJoin(Client& client, parsedCmd& cmd) {

//	Necesito revisar si:
//		# cliente esta registrado
//		# revisar parametros
//		# CHANNEL_NAME es valido ('#', '&'?, <alguno mas>)
//
//	MSG_ERROR.cpp ???
//
	if (cmd.params.empty())
	{
		reply( client, ":server 461 " + client.getNick() + " JOIN :Not enough parameters");
		return ;
	}
	if (cmd.params[0][0] != '#')
	{
		reply( client, ":server 403 " + client.getNick() + " " + cmd.params[0] + " :No such channel");
		return ;
	}
//
//	Comprobar si:
//		# cliente ya es mienbro
//		# canal invite-only
//		# canal tiene key
//		# se alcanzo el limite del channel
//		# eliminar invitacion si la tenia
//
	Channel* chan = getChannel(cmd.params[0]);
	if ( chan == NULL )
	{
		chan = createChannel(cmd.params[0]);
		if (chan == NULL)// MSG ERROR ???
			return ;
		chan->addOperator(client.getFd());// first joiner becomes operator
	}
	chan->addMember( client.getFd() );
//
//	Enviar mensaje de JOINED a todos los clientes del CHANNEL
//
	std::ostringstream fd;
	fd << client.getFd();

//	No deberia ser chan, user/nick + joined + CHANNEL_NAME??
	broadcast( *chan, "fd " + fd.str() + " joined " + chan->getName(), -1 );
	return ;
}

void    Server::handlePass(Client& client, parsedCmd& cmd)
{
	// Si el cliente aún no tiene nick, el estándar indica que se use "*" en las respuestas
	std::string nick = client.getNick().empty() ? "*" : client.getNick();

	if (client.isRegistered() || client.isPassOk())
	{
		reply(client, ":server 462 " + nick + " :Unauthorized command (already registered)");
		return ;
	}

	if (cmd.params.empty())
	{
		reply(client, ":server 461 " + nick + " PASS :Not enough parameters");
		return ;
	}

	if (cmd.params[0] == _pwd)
	{
		client.setPassOk(true);
	}
	else
	{
		reply(client, ":server 464 " + nick + " :Password incorrect");
		client.markForDisconnect();
	}
}
