#include "MessageHub.h"
#include <optional>
#include <algorithm>


MP::MessageHub::MessageHub() : Client(5ms)
{
	addMessageHandlerPair( { "Stop"_hash, std::bind( &MessageHub::stop, this ) } );
	this->addClient( this );
}

MP::MessageHub::~MessageHub()
{

}

void MP::MessageHub::addClient( Client* client )
{
	if ( const auto index = find( _clientIdentifiers, client->identifier() ); index.has_value() )
		throw ClientDuplicate();

	_clientIdentifiers.push_back( client->identifier() );
	_clients.push_back( client );
}
void MP::MessageHub::run() noexcept
{
	for ( auto& client : _clients )
		client->start();
	running = true;
	_threadEntryPoint();
}
//
//void MP::MessageHub::StartAllClients() noexcept
//{
//	for (auto& client : _clients)
//		client->Start(client);
//}

void MP::MessageHub::handleMessages() noexcept
{
	for ( auto& client : _clients )
	{
		auto& messageQueue = client->outgoingMessages();
		while ( !messageQueue.isEmpty() )
		{
			auto message = std::move(messageQueue.top());
			if ( const auto index = find( _clientIdentifiers, message.target ); index.has_value() )
			{
				auto& clientTarget = _clients[*index];
				message.target = _clientIdentifiers[*index];
				clientTarget->incommingMessages().push( std::move( message ) );
			}
			messageQueue.pop();
		}
	}
}

const Utilities::GUID MP::MessageHub::identifier()const noexcept
{
	return Utilities::EnsureHash<"Hub"_hash>::value;
}

void MP::MessageHub::start() noexcept
{ 
}

void MP::MessageHub::stop()noexcept
{ 
	running = false;
}

void MP::MessageHub::_performOtherActions() noexcept
{
	handleMessages();
}
