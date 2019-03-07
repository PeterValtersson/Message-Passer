#include "stdafx.h"
#include "CppUnitTest.h"
#include "../include/Client.h"
#include "../include/IMessageHub.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	class Test : public MP::Client {
	public:
		Test( ) : MP::Client( 16ms )
		{
			addMessageHandlerPair( { "PerformAction", std::bind( &Test::_performAction, this, _1 ) } );
		}
		~Test() { }
		const Utilities::GUID identifier()const noexcept override
		{
			return Utilities::EnsureHash<"TestClient"_hash>::value;
		}
	private:
		void _performAction( MP::Message& message )
		{
			int r = 0;
			while ( r != 1000 )
			{
				auto ret = sendMessage( MP::Message("TestClient2"_hash, "PerformAction"_hash,r) );
				r = ret.get()->get<int>();
			}
			sendMessage( { "Hub"_hash, "Stop"_hash } );
		}
	};

	class Test2 : public MP::Client {
	public:
		Test2() : MP::Client( 16ms )
		{
			addMessageHandlerPair( { "PerformAction", std::bind( &Test2::_performAction, this, _1 ) } );
		}
		~Test2() { }
		const Utilities::GUID identifier()const noexcept override
		{
			return Utilities::EnsureHash<"TestClient2"_hash>::value;
		}
	private:
		void _performAction( MP::Message& message )
		{
			auto r = message.payload->get<int>();
			message.setReturn( r+1 );
		}
	};
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(OneToOneCommunicationTest)
		{
			auto hub = MP::IMessageHub::get();
			auto test = std::make_unique<Test>();
			test->sendMessage( { "TestClient"_hash, "PerformAction"_hash } );
			hub->addClient( test.get() );
			auto test2 = std::make_unique<Test2>();
			hub->addClient( test2.get() );
			hub->run();

		}
		
	};
}