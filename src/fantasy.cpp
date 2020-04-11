#include <fantasy.hpp>
#include "user.cpp"
#include "tokendistribution.cpp"
#include "games.cpp"

// TODO
// Fantasy token deposit
// Token distribution and claiming/push
// Fantasy distribution and claiming/push
// create new user java program
// create kyc user java program
// .. create server programs to write metadata on blockchain

// create ios/android app
// screen_1 prediction games
// screen_2 fantasy games
// screen_3 from screen_2 player selection
// screen_4 leaderboards
// screen_5 live score screen

// unique id generator for key
// store uniqueid in db
// create backend to serve ui


// time based player choose game -p2

EOSIO_DISPATCH(fantasy, (initiateuser)(kycuser)(blockuser)
(adddistevent)(openvoting)(addoutcome)(useroption)
(regfanevent)(addplayer)(openfanevent))
