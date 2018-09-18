#include <IMessageHub.h>
#include "MessageHub.h"

std::shared_ptr<MP::IMessageHub> MP::createMessageHub()
{
	return std::make_shared<MP::MessageHub>();
}