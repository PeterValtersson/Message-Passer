#ifndef _INTERFACE_MESSAGE_HUB_H_
#define _INTERFACE_MESSAGE_HUB_H_
#include <memory>
#include <GUID.h>

namespace MP
{
	class Client;
	struct Message {
		Utilities::GUID target;
		Utilities::GUID identifier;
	};

	struct ClientDuplicate : public std::exception {
		ClientDuplicate() : std::exception("Tried to add a client with an identical identifier to another client") {}
	};


	class IMessageHub {
	public:
		virtual ~IMessageHub() {}
		virtual void AddClient(std::unique_ptr<Client>&& client) = 0;
		virtual void StartAllClients() noexcept = 0;
		virtual void HandleMessages() noexcept = 0;
	protected:
		IMessageHub(){}
		

		
	};

#if defined DLL_EXPORT_MESSAGE_PASSER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
	DECLDIR std::shared_ptr<IMessageHub> CreateMessageHub();

}

#endif