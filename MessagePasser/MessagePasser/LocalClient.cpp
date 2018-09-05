#include "LocalClient.h"
extern thread_local std::weak_ptr<MP::Client> MP::localClient = std::weak_ptr<MP::Client>();
