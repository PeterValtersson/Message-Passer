#include "stdafx.h"
#include "CppUnitTest.h"
#include "../include/Client.h"
#include "../include/IMessageHub.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	class SimpleClient : public MP::Client {
	public:
		SimpleClient(std::shared_ptr<MP::IMessageHub> hub) : MP::Client(hub, 16ms), _actionPerformed(false)
		{
			AddMessageHandlerPair({ "PerformAction", std::bind(&SimpleClient::_performAction, this, _1) });
		}
		~SimpleClient() {}
		const Utilities::GUID Identifier()const noexcept
		{
			return "SimpleClient"_hash;
		}
		bool actionNotPerformed()const noexcept
		{
			return !_actionPerformed;
		}
	private:
		bool _actionPerformed;
		void _performAction(MP::Message&& message)
		{
			_actionPerformed = true;
		}


	};
	class UserClient : public MP::Client {
	public:
		UserClient(std::shared_ptr<MP::IMessageHub> hub) : MP::Client(hub, 16ms), _actionPerformed(false)
		{

		}
		const Utilities::GUID Identifier()const noexcept override
		{
			return "UserClient"_hash;
		}

		~UserClient() {}

		bool actionNotPerformed()const noexcept
		{
			return !_actionPerformed;
		}
	private:
		bool _actionPerformed;
		void _performOtherActions() override
		{

			if (!_actionPerformed)
			{
				_sendMessage({ "SimpleClient", "PerformAction" });
				_actionPerformed = true;
			}
		}
	};

	class SimpleClientPromise : public MP::Client {
	public:
		SimpleClientPromise(std::shared_ptr<MP::IMessageHub> hub) : MP::Client(hub, 16ms), _actionPerformed(false)
		{
			AddMessageHandlerPair({ "PerformAction", std::bind(&SimpleClientPromise::_performAction, this, _1) });
		}
		~SimpleClientPromise() {}
		const Utilities::GUID Identifier()const noexcept
		{
			return "SimpleClient"_hash;
		}
		bool actionNotPerformed()const noexcept
		{
			return !_actionPerformed;
		}
	private:
		bool _actionPerformed;
		void _performAction(MP::Message&& message)
		{
			message.status.set_value(MP::Status::Success);
			_actionPerformed = true;	
		}


	};
	class UserClientFuture : public MP::Client {
	public:
		UserClientFuture(std::shared_ptr<MP::IMessageHub> hub) : MP::Client(hub, 16ms), _actionPerformed(false)
		{

		}
		const Utilities::GUID Identifier()const noexcept override
		{
			return "UserClient"_hash;
		}

		~UserClientFuture() {}

		bool actionNotPerformed()const noexcept
		{
			return !_actionPerformed;
		}
	private:
		bool _actionPerformed;
		void _performOtherActions() override
		{

			if (!_actionPerformed)
			{
				auto future = _sendMessage({ "SimpleClient", "PerformAction" });
				Assert::IsTrue(MP::Status::Success == future.get());
				_actionPerformed = true;
			}
		}
	};
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(OneToOneCommunicationTest)
		{
			auto hub = MP::CreateMessageHub();
			auto simpleClient = std::make_unique<SimpleClient>(hub);
			auto scPtr = simpleClient.get();
			hub->AddClient(std::move(simpleClient));
			auto userClient = std::make_unique<UserClient>(hub);
			auto ucPtr = userClient.get();
			hub->AddClient(std::move(userClient));
			hub->StartAllClients();

			while (scPtr->actionNotPerformed() || ucPtr->actionNotPerformed())
				hub->HandleMessages();

		}
		TEST_METHOD(FutureTest)
		{
			auto hub = MP::CreateMessageHub();
			auto simpleClient = std::make_unique<SimpleClientPromise>(hub);
			auto scPtr = simpleClient.get();
			hub->AddClient(std::move(simpleClient));
			auto userClient = std::make_unique<UserClientFuture>(hub);
			auto ucPtr = userClient.get();
			hub->AddClient(std::move(userClient));
			hub->StartAllClients();

			while (scPtr->actionNotPerformed() || ucPtr->actionNotPerformed())
				hub->HandleMessages();

		}
	};
}