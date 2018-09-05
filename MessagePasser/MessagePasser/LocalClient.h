#ifndef _LOCAL_CLIENT_H_
#define _LOCAL_CLIENT_H_
#include <Client.h>
namespace MP
{
	extern thread_local std::weak_ptr<Client> localClient;

}
#endif

