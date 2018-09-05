#ifndef _MESSAGE_HUB_H_
#define _MESSAGE_HUB_H_
#include <Client.h>
#include <IMessageHub.h>
#include <vector>

namespace MP
{
	class MessageHub final : public IMessageHub {
	public:
		~MessageHub();
		void AddClient(std::shared_ptr<Client> client) override;
		void StartAllClients() noexcept override;
		void HandleMessages() noexcept override;
	private:
		std::vector<Utilities::GUID> _clientIdentifiers;
		std::vector<std::shared_ptr<Client>> _clients;
	};

}
#endif