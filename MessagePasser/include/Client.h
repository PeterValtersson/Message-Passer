#ifndef _MESSAGE_CLIENT_H_
#define _MESSAGE_CLIENT_H_
#include <GUID.h>
#include <CircularFIFO.h>

#include <functional>
#include <exception>
#include <thread>
#include <vector>
#include <chrono>
#include <ErrorHandling.h>
#include "IMessageHub.h"
#include <MonadicOptional.h>
#include "DLL_Export.h"

using namespace std::placeholders;
using namespace std::chrono_literals;

namespace MP
{
	class Client;
	/*struct NoLocalClient : public Utilities::Exception {
		NoLocalClient() : Utilities::Exception("No local client could be found") {}
	};
	DECLSPEC_MP std::shared_ptr<Client> getLocalClient();
	DECLSPEC_MP void setLocalClient(std::shared_ptr<Client> client);*/

	using MessageHandler = std::function<void( Message& message )>;
	/*struct NoMessageHub : Utilities::Exception {
		NoMessageHub() : Utilities::Exception( "A message hub was not provided to client" ) { }
	};*/
	struct MessageDuplicate : Utilities::Exception {
		MessageDuplicate() : Utilities::Exception( "Tried to add a message with an identical identifier to another message" ) { }
	};
	struct MessageFromInvalidThread : Utilities::Exception {
		MessageFromInvalidThread() : Utilities::Exception( "A message can only be sent from the same thread that the client is running on." ) { }
	};


	struct MessageHandlerPair {
		Utilities::GUID identifier;
		MessageHandler handler;
	};
	class Client {
	public:
		virtual ~Client() noexcept
		{
			stop();
		}

		void addMessageHandlerPair( const MessageHandlerPair& toAdd )
		{
			if ( const auto index = find( _messageIdentifiers, toAdd.identifier ); index.has_value() )
				throw MessageDuplicate();
			_messageIdentifiers.push_back( toAdd.identifier );
			_messageHandlers.push_back( std::move( toAdd.handler ) );
		}

		

		virtual const Utilities::GUID identifier()const noexcept = 0;



		MessageReturn sendMessage( Message&& message )
		{
			/*if ( std::this_thread::get_id() != _threadID )
				throw MessageFromInvalidThread();*/
			auto messageReturn = message.messageReturn.get_future();
			_outgoingMessages.push( std::move( message ) );
			return messageReturn;
		}
		virtual void start() noexcept
		{
			if ( running )
				return;
			running = true;
			_myThread = std::thread( &Client::_threadEntryPoint, this );
		}

		virtual void stop() noexcept
		{
			running = false;
			if ( _myThread.joinable() )
				_myThread.join();
		}

		inline auto& outgoingMessages() noexcept
		{
			return _outgoingMessages;
		}
		inline auto& incommingMessages() noexcept
		{
			return _incommingMessages;
		}
	protected:
		Client( std::chrono::milliseconds timePerFrame ) : running( false ), _timePerFrame( timePerFrame )
		{
	
		};
		Client( std::chrono::milliseconds timePerFrame, const std::vector<MessageHandlerPair>& messageHandlerPairs )
			:running( false ), _timePerFrame( timePerFrame )
		{
			for ( const auto& pair : messageHandlerPairs )
				addMessageHandlerPair( pair );

		};


		virtual void _performDelayedActions() { }
		virtual void _performOtherActions() { }

		
		
		void _threadEntryPoint()
		{
			//setLocalClient(me);
			_threadID = std::this_thread::get_id();
			while ( running )
			{
				const auto startTime = std::chrono::high_resolution_clock::now();
				while ( !_incommingMessages.isEmpty() )
				{
					auto message = std::move( _incommingMessages.top() );
					if ( const auto& index = find( _messageIdentifiers, message.identifier ); !index.has_value() )
						index; // Unknown message recevied. Add a log or something.
					else
						_messageHandlers[*index]( message );
					_incommingMessages.pop();
				}
				_performDelayedActions();
				_performOtherActions();
				const auto endTime = std::chrono::high_resolution_clock::now();
				const auto executionTime = endTime - startTime;
				if ( executionTime < _timePerFrame ) // We wait to lock the update frequency.
					std::this_thread::sleep_for( _timePerFrame - executionTime );
			}
		}
		bool running;
	private:
		
		std::chrono::milliseconds _timePerFrame;
		std::vector<Utilities::GUID> _messageIdentifiers;
		std::vector<MessageHandler> _messageHandlers;

		
		std::thread _myThread;
		std::thread::id _threadID;

		Utilities::CircularFiFo<Message> _incommingMessages;
		Utilities::CircularFiFo<Message> _outgoingMessages;

		
	};

}

#endif