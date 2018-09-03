#include <IMessageHub.h>
#include "MessageHub.h"

std::shared_ptr<MP::IMessageHub> MP::CreateMessageHub()
{
	return std::make_shared<MP::MessageHub>();
}