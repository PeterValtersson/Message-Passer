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

using namespace std::placeholders;
using namespace std::chrono_literals;

namespace MP
{
	using MessageHandler = std::function<void(const Message&& message)>;
	struct NoMessageHub : public std::exception {
		NoMessageHub() : std::exception ("A message hub was not provided to client") {}
		
	};
	struct MessageDuplicate : public std::exception {
		MessageDuplicate() : std::exception("Tried to add a message with an identical identifier to another message") {}
	};

	struct MessageHandlerPair {
		Utilities::GUID identifier;
		MessageHandler handler;
	};
	class Client {
	public:
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
		virtual ~Client() 
		{
			Stop();
		}

		void AddMessageHandlerPair(const MessageHandlerPair& toAdd) {
			if (const auto index = find(_messageIdentifiers, toAdd.identifier); index.has_value())
				throw MessageDuplicate();
			_messageIdentifiers.push_back(toAdd.identifier);
			_messageHandlers.push_back(std::move(toAdd.handler));
		}

		virtual void Start() noexcept
		{
			running = true;
			_myThread = std::thread(&Client::_threadEntryPoint, this);
		}

		virtual void Stop() noexcept
		{
			running = false;
			if(_myThread.joinable())
				_myThread.join();
		}

		virtual const Utilities::GUID Identifier()const noexcept = 0;

		inline auto& outgoingMessages()
		{
			return _outgoingMessages;
		}
		inline auto& incommingMessages()
		{
			return _incommingMessages;
		}

	protected:
		virtual void _performDelayedActions() {}
		virtual void _performOtherActions() {}

		void _sendMessage(const Message&& message)
		{
			_outgoingMessages.push(std::move(message));
		}
	private:
		bool running;
		std::chrono::milliseconds _timePerFrame;
		std::vector<Utilities::GUID> _messageIdentifiers;
		std::vector<MessageHandler> _messageHandlers;
		std::shared_ptr<IMessageHub> _messageHub;


		std::thread _myThread;
		Utilities::CircularFiFo<Message> _incommingMessages;
		Utilities::CircularFiFo<Message> _outgoingMessages;
		
		void _threadEntryPoint() 
		{
			
			while (running)
			{
				const auto startTime = std::chrono::high_resolution_clock::now();
				while (!_incommingMessages.wasEmpty())
				{
					auto& message = _incommingMessages.top();
					if (const auto& index = find(_messageIdentifiers, message.identifier); !index.has_value())
						index; // Unknown message recevied. Add a log or something.
					else
						_messageHandlers[*index](std::move(message));
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