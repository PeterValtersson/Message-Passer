#ifndef _INTERFACE_MESSAGE_HUB_H_
#define _INTERFACE_MESSAGE_HUB_H_
#include <memory>
#include <GUID.h>
#include <future>
#include "DLL_Export.h"
namespace MP
{
	class Client;

	struct Payload {
		virtual ~Payload() noexcept {}

		template<typename DataType>
		static std::unique_ptr<Payload> make(DataType&& data)
		{
			struct Concrete_Payload : public Payload {
				DataType _data;
				Concrete_Payload(DataType&& data) : _data(std::move(data)) {}
				const void* _get()const
				{
					return &_data;
				}
			};
			return std::make_unique<Concrete_Payload>(std::move(data));
		}
		template<typename DataType>
		const DataType& get()const
		{
			return *reinterpret_cast<const DataType*>(_get());
		}
	protected:
		Payload() {}
		virtual const void* _get()const = 0;
	};

	enum class Status {
		Success,
		Failed
	};

	struct Message {
		Utilities::GUID target;
		Utilities::GUID identifier;	
		std::unique_ptr<Payload> payload;
		std::promise<Status> status;
		Message() {}
		Message(const Utilities::GUID target, const Utilities::GUID identifier)
			: target(target), identifier(identifier)
		{}
		template<typename DataType>
		Message(const Utilities::GUID target, const Utilities::GUID identifier, DataType&& data) 
			: target(target), identifier(identifier), payload(Payload::make(std::move(data)))
		{}
	};

	struct ClientDuplicate : public std::exception {
		ClientDuplicate() : std::exception("Tried to add a client with an identical identifier to another client") {}
	};


	class IMessageHub {
	public:
		virtual ~IMessageHub() {}
		virtual void AddClient(std::shared_ptr<Client> client) = 0;
		virtual void StartAllClients() noexcept = 0;
		virtual void HandleMessages() noexcept = 0;
	protected:
		IMessageHub(){}
		

		
	};

	DECLSPEC std::shared_ptr<IMessageHub> CreateMessageHub();

}

#endif