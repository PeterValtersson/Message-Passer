#include "Client.h"
extern thread_local std::shared_ptr<MP::Client> MP::localClient = nullptr;
void MP::Client::_threadEntryPoint()

{
	//localClient = std::make_shared<MP::Client>(this);
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

std::shared_ptr<MP::Client> MP::getLocalClient()
{
	if (!localClient)
		throw NoLocalClient();
	return localClient;
}
