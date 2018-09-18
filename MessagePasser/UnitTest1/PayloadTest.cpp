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
		void _performAction(MP::Message& message)
		{
			_actionPerformed = true;
			Assert::AreEqual("This is a message", message.payload->get<const char[18]>());
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
				sendMessage({ "SimpleClient", "PerformAction", "This is a message"});
				_actionPerformed = true;
			}
		}
	};


	TEST_CLASS(UnitTest1)
	{
	public:
		template<typename T>
		bool areEqual(const std::vector<T>& v1, const std::vector<T>& v2)
		{
			return (v1.size() == v2.size() &&
				std::equal(v1.begin(), v1.end(), v2.begin()));
		}


		
		TEST_METHOD(PayloadCreationTests)
		{
		
			auto constchar = MP::Payload::make("constchar");
			Assert::AreEqual("constchar", constchar->get<const char[10]>());
			MP::Message constcharmsg = { "","", "constchar" };
			Assert::AreEqual("constchar", constcharmsg.payload->get<const char[10]>());

			auto integer = MP::Payload::make(1);
			Assert::AreEqual(1, integer->get<int>());
			MP::Message integermsg = { "","", 1 };
			Assert::AreEqual(1, integermsg.payload->get<int>());

			std::vector<int> intVector = { 1,2,3 };
			auto intvec = MP::Payload::make(std::move(intVector));		
			Assert::IsTrue(areEqual({ 1,2,3 }, intvec->get<std::vector<int>>()));

			std::vector<std::string> stringVector = { "Test", "A", "B"};
			auto intvecptr = MP::Payload::make(&stringVector);
			Assert::IsTrue(areEqual(stringVector, *intvecptr->get<std::vector<std::string>*>()));

		}
		TEST_METHOD(PayloadTest)
		{
			auto hub = MP::createMessageHub();
			auto simpleClient = std::make_shared<SimpleClient>(hub);
			hub->AddClient(simpleClient);
			auto userClient = std::make_shared<UserClient>(hub);
			hub->AddClient(userClient);
			hub->StartAllClients();

			while (simpleClient->actionNotPerformed() || userClient->actionNotPerformed())
				hub->HandleMessages();

		}

	};
}