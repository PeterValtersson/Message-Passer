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
	
	using MessageReturn = std::future<std::unique_ptr<Payload>>;

	struct Message {
		Utilities::GUID target;
		Utilities::GUID identifier;	
		std::unique_ptr<Payload> payload;
		std::promise<std::unique_ptr<Payload>> messageReturn;
		Message() {}
		~Message() 
		{	
			try
			{
				messageReturn.set_value(nullptr);
			}
			catch (std::future_error& error) { error;/* Fails if setReturn has been called*/ }
		}
		Message(Message&& other)
		{
			target = other.target;
			identifier = other.identifier;
			payload = std::move(other.payload);
			messageReturn = std::move(other.messageReturn);
		}
		Message& operator=(Message&& other)
		{
			target = other.target;
			identifier = other.identifier;
			payload = std::move(other.payload);
			messageReturn = std::move(other.messageReturn);
			return *this;
		}
		Message(const Utilities::GUID target, const Utilities::GUID identifier)
			: target(target), identifier(identifier)
		{}
		template<typename DataType>
		Message(const Utilities::GUID target, const Utilities::GUID identifier, DataType&& data) 
			: target(target), identifier(identifier), payload(Payload::make(std::move(data)))
		{}
		template<typename DataType>
		void setReturn(DataType&& data)
		{
			try
			{
				messageReturn.set_value(Payload::make(std::move(data)));
			}
			catch (std::future_error& error) { error; }
			
		}
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

	DECLSPEC_MP std::shared_ptr<IMessageHub> createMessageHub();

}

#endif