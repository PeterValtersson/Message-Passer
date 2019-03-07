#include <IMessageHub.h>
#include "MessageHub.h"


#pragma data_seg (".MP_SHAREDMEMORY")
std::shared_ptr<MP::IMessageHub> messageHub = nullptr;
#pragma data_seg() 
#pragma comment(linker,"/SECTION:.MP_SHAREDMEMORY,RWS")

DECLSPEC_MP std::shared_ptr<MP::IMessageHub> MP::IMessageHub::get()
{
	if ( !messageHub )
		messageHub = std::make_shared<MessageHub>();
	return messageHub;
}