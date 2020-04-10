#include <fantasy.hpp>
#include "user.cpp"
#include "tokendistribution.cpp"
#include "games.cpp"

EOSIO_DISPATCH(fantasy, (initiateuser)(kycuser)(blockuser)
(adddistevent)(openvoting)(addoutcome)(useroption)
(regfanevent)(addplayer)(openfanevent))
