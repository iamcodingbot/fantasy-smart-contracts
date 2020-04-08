#include <eosio/system.hpp>
#include <vector>

ACTION fantasy::adddistevent(uint32_t event_id, vector<uint32_t> option_ids, uint64_t event_close_time) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);
  auto event_itr = _distribution_event_registration_table.find(event_id);
  
  if(event_itr == _distribution_event_registration_table.end()) {
    _distribution_event_registration_table.emplace(get_self(), [&](auto& row){
      row.event_id = event_id;
      row.option_ids= option_ids;
      row.event_close_time = event_close_time;
      row.event_status = INITIATING;});
  } else {
    _distribution_event_registration_table.modify(event_itr, get_self(),[&](auto& modrow){
      modrow.option_ids = option_ids;
      modrow.event_close_time = event_close_time;});
  }
}

ACTION fantasy::openvoting(uint32_t event_id) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
    {row.event_status = OPEN;});
}


ACTION fantasy::addoutcome(uint32_t event_id, uint32_t winning_option_id) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");

  // check if winning_option_id is a valid option
  vector<uint32_t> valid_option_ids= event_itr.option_ids;
  vector<uint32_t>::iterator option_itr = find(valid_option_ids.begin(), valid_option_ids.end(), winning_option_id);;
  check(option_itr != valid_option_ids.end(), "Invalid option");

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
    {row.outcome_option_id = winning_option_id;
    row.event_status = CLOSED;});
}

ACTION fantasy::useroption(name user, uint32_t event_id, uint32_t option_id) {
  require_auth(user);

  users_table _users_table(get_self(), get_self().value);

  
  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  // check if user is a valid user
  auto& user_itr = _users_table.get(user.value, "Invalid user");

  // check if user is blacklisted
  if(user_itr.user_status == BLACK_LIST) {
    eosio::check(false, "Can not participate, user blacklisted");
  }

  // check if event_id exists
  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");
  
  // check if option_id is a valid option_id.
  vector<uint32_t> valid_option_ids= event_itr.option_ids;
  vector<uint32_t>::iterator option_itr = find(valid_option_ids.begin(), valid_option_ids.end(), option_id);;
  check(option_itr != valid_option_ids.end(), "Invalid option");

  // check if voting for event_id is not closed
  uint64_t event_close_time = event_itr.event_close_time;
  check(event_close_time<eosio::current_time_point().sec_since_epoch(), "Event voting closed");

  // check if voting for event_id is open
  check(event_itr.event_status==OPEN, "Event voting not in OPEN state");

  // find {event, user} combo and update an option if combo exists
  distribution_user_selection_table _distribution_user_selection_table(get_self(), get_self().value);
  auto users_index = _distribution_user_selection_table.get_index<name("userkey")>();
  auto users_itr = users_index.find(user.value);

  bool found = false;
  while(users_itr!=users_index.end()){
      if(users_itr->user!=user) {
        // records for this user are exhaused
        break;
      }
      if(users_itr->event_id == event_id) {
        found = true;
        users_index.modify(users_itr, get_self(), [&](auto& modrec){
        modrec.option_id = option_id;});
        break;
      }
      users_itr ++ ;
  }

  // if {event , user} combo does not exist, create a new entry
  if(found==false) {
    _distribution_user_selection_table.emplace(get_self(), [&](auto& newrec) {
      newrec.selection_id = _distribution_user_selection_table.available_primary_key();
      newrec.user = user;
      newrec.event_id = event_id;
      newrec.option_id = option_id;
    });
  }

}