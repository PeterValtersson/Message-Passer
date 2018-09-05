#include <Client.h>
#include "LocalClient.h"
std::shared_ptr<MP::Client> MP::getLocalClient()
{
	if (auto sp = localClient.lock())
		return sp;
	else
		throw NoLocalClient();
}

DECLSPEC void MP::setLocalClient(std::shared_ptr<Client> client)
{
	localClient = client;
}
