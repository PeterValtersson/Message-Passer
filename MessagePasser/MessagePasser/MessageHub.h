#ifndef _MESSAGE_HUB_H_
#define _MESSAGE_HUB_H_
#include <Client.h>
#include <IMessageHub.h>
#include <vector>

namespace MP
{
	class MessageHub final : public IMessageHub , public Client {
	public:
		MessageHub();
		~MessageHub();
		void addClient( Client* client ) override;
		//void StartAllClients() noexcept override;
		
		void run()noexcept override;

		const Utilities::GUID identifier()const noexcept override;
		void start()noexcept override;
		void stop()noexcept override;
	protected:
		void _performOtherActions()noexcept override;
	private:
		void handleMessages() noexcept override;

		std::vector<Utilities::GUID> _clientIdentifiers;
		std::vector<Client*> _clients;
	};

}
#endif