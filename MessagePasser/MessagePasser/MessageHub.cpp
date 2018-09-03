#include "MessageHub.h"
#include <optional>
#include <algorithm>
#include "HelperFunction.h"


MP::MessageHub::~MessageHub()
{
	
}

void MP::MessageHub::AddClient(std::unique_ptr<Client>&& client)
{
	if (const auto index = find(_clientIdentifiers, client->Identifier()); index.has_value())
		throw ClientDuplicate();

	_clientIdentifiers.push_back(client->Identifier());
	_clients.push_back(std::move(client));
}

void MP::MessageHub::StartAllClients() noexcept
{
	for (auto& client : _clients)
		client->Start();
}

void MP::MessageHub::HandleMessages() noexcept
{
	for (auto& client : _clients)
	{
		auto& messageQueue = client->outgoingMessages();
		while (!messageQueue.wasEmpty())
		{
			auto& message = messageQueue.top();
			if (const auto index = find(_clientIdentifiers, message.target); index.has_value())
			{
				auto& clientTarget = _clients[*index];
				message.target = _clientIdentifiers[*index];
				clientTarget->incommingMessages().push(std::move(message));
			}
			messageQueue.pop();
		}
	}
}
