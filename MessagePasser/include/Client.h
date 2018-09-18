#ifndef _MESSAGE_CLIENT_H_
#define _MESSAGE_CLIENT_H_
#include <GUID.h>
#include <CircularFIFO.h>

#include <functional>
#include <exception>
#include <thread>
#include <vector>
#include <chrono>

#include "IMessageHub.h"
#include <HelperFunction.h>
#include "DLL_Export.h"

using namespace std::placeholders;
using namespace std::chrono_literals;

namespace MP
{
	class Client;
	struct NoLocalClient : public std::runtime_error {
		NoLocalClient() : std::runtime_error("No local client could be found") {}
	};
	DECLSPEC std::shared_ptr<Client> getLocalClient();
	DECLSPEC void setLocalClient(std::shared_ptr<Client> client);

	using MessageHandler = std::function<void(Message& message)>;
	struct NoMessageHub : public std::exception {
		NoMessageHub() : std::exception ("A message hub was not provided to client") {}
	};
	struct MessageDuplicate : public std::exception {
		MessageDuplicate() : std::exception("Tried to add a message with an identical identifier to another message") {}
	};
	struct MessageFromInvalidThread : public std::exception {
		MessageFromInvalidThread() : std::exception("A message can only be sent from the same thread that the client is running on.") {}
	};


	struct MessageHandlerPair {
		Utilities::GUID identifier;
		MessageHandler handler;
	};
	class Client {
	public:
		virtual ~Client() noexcept
		{
			Stop();
		}

		void AddMessageHandlerPair(const MessageHandlerPair& toAdd) {
			if (const auto index = find(_messageIdentifiers, toAdd.identifier); index.has_value())
				throw MessageDuplicate();
			_messageIdentifiers.push_back(toAdd.identifier);
			_messageHandlers.push_back(std::move(toAdd.handler));
		}

		virtual void Start(std::shared_ptr<Client> me) noexcept
		{
			if (running)
				return;
			running = true;
			_myThread = std::thread(&Client::_threadEntryPoint, this, me);
		}

		virtual void Stop() noexcept
		{
			running = false;
			if(_myThread.joinable())
				_myThread.join();
		}

		virtual const Utilities::GUID Identifier()const noexcept = 0;

		inline auto& outgoingMessages() noexcept
		{
			return _outgoingMessages;
		}
		inline auto& incommingMessages() noexcept
		{
			return _incommingMessages;
		}

		MessageReturn sendMessage(Message&& message)
		{
			if (std::this_thread::get_id() != _threadID)
				throw MessageFromInvalidThread();
			auto messageReturn = message.messageReturn.get_future();
			_outgoingMessages.push(std::move(message));
			return messageReturn;
		}

	protected:
		Client(std::shared_ptr<IMessageHub> messageHub, std::chrono::milliseconds timePerFrame) : _messageHub(messageHub), running(false), _timePerFrame(timePerFrame)
		{
			if (!_messageHub) throw NoMessageHub();
		};
		Client(std::shared_ptr<IMessageHub> messageHub, std::chrono::milliseconds timePerFrame, const std::vector<MessageHandlerPair>& messageHandlerPairs)
			: _messageHub(messageHub), running(false), _timePerFrame(timePerFrame)
		{
			if (!_messageHub) throw NoMessageHub();
			for (const auto& pair : messageHandlerPairs)
				AddMessageHandlerPair(pair);
		};
		Client(std::shared_ptr<IMessageHub> messageHub, std::chrono::milliseconds timePerFrame, const std::vector<MessageHandlerPair>&& messageHandlerPairs)
			: _messageHub(messageHub), running(false), _timePerFrame(timePerFrame)
		{
			if (!_messageHub) throw NoMessageHub();
			for (const auto& pair : messageHandlerPairs)
				AddMessageHandlerPair(pair);
		};

		virtual void _performDelayedActions() {}
		virtual void _performOtherActions() {}

		
	private:
		
		std::chrono::milliseconds _timePerFrame;
		std::vector<Utilities::GUID> _messageIdentifiers;
		std::vector<MessageHandler> _messageHandlers;
		std::shared_ptr<IMessageHub> _messageHub;

		bool running;
		std::thread _myThread;
		std::thread::id _threadID;

		Utilities::CircularFiFo<Message> _incommingMessages;
		Utilities::CircularFiFo<Message> _outgoingMessages;
		
		void _threadEntryPoint(std::shared_ptr<Client> me)
		{
			setLocalClient(me);
			_threadID = std::this_thread::get_id();
			while (running)
			{
				const auto startTime = std::chrono::high_resolution_clock::now();
				while (!_incommingMessages.wasEmpty())
				{
					auto message = std::move(_incommingMessages.top());
					if (const auto& index = find(_messageIdentifiers, message.identifier); !index.has_value())
						index; // Unknown message recevied. Add a log or something.
					else
						_messageHandlers[*index](message);
					_incommingMessages.pop();
				}
				_performDelayedActions();
				_performOtherActions();
				const auto endTime = std::chrono::high_resolution_clock::now();
				const auto executionTime = endTime - startTime;
				if (executionTime < _timePerFrame) // We wait to lock the update frequency.
					std::this_thread::sleep_for(_timePerFrame - executionTime);
			}
		}
	};

}

#endif