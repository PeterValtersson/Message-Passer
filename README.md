# Message-Passer
A library for sending messages between clients. Completely lock free.
Each client runs on a seperate thread and can send messages between eachother.

## Usage
```cpp
class SimpleClient : public MP::Client {
	public:
		SimpleClient(std::shared_ptr<MP::IMessageHub> hub) 
                    : MP::Client(hub, 16ms), _actionPerformed(false)
		{
			AddMessageHandlerPair({ "PerformAction",
                             std::bind(&SimpleClient::_performAction, this, _1) });
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
		void _performAction(const MP::Message&& message)
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
```
