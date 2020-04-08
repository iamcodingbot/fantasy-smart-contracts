#include <fantasy.hpp>
#include "user.cpp"
#include "tokendistribution.cpp"

EOSIO_DISPATCH(fantasy, (initiateuser)(kycuser)(blockuser)
(adddistevent)(openvoting)(addoutcome)(useroption))
