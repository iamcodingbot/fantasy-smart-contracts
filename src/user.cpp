#include <eosio/system.hpp>

ACTION fantasy::initiateuser(name user) {
  require_auth(get_self());

  users_table _users_table(get_self(), get_self().value);

  auto user_itr = _users_table.find(user.value);

  if (user_itr == _users_table.end()) {
    _users_table.emplace(get_self(), [&](auto& row) {
      row.user = user;
      row.user_status = NOT_VERIFIED;
    });
  } 
}

ACTION fantasy::kycuser(name user) {
  require_auth(get_self());

  users_table _users_table(get_self(), get_self().value);

  auto user_itr = _users_table.find(user.value);

  check(user_itr!=_users_table.end(), "User does not exist");

  _users_table.modify(user_itr, get_self(),[&](auto& row){
      row.user_status = VERIFIED;
    });

}

ACTION fantasy::blockuser(name user) {
  require_auth(get_self());

  users_table _users_table(get_self(), get_self().value);

  auto user_itr = _users_table.find(user.value);

  check(user_itr!=_users_table.end(), "User does not exist");

  _users_table.modify(user_itr, get_self(),[&](auto& row){
      row.user_status = BLACK_LIST;
    });
}
