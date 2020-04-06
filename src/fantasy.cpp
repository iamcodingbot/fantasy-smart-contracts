#include <fantasy.hpp>
#include <eosio/system.hpp>


ACTION fantasy::addmoduser(name user, uint8_t status) {
  require_auth(get_self());

  users_table _users_table(get_self(), get_self().value);

  auto user_itr = _users_table.find(user.value);

  if (user_itr == _users_table.end()) {
    _user_table.emplace(get_self(), [&](auto& row) {
      row.user = user;
      row.user_status = status;
    });
  } else {
    _users_table.modify(user_itr, get_self(), [&](auto& row){
          row.user_status = status;
    });
  }
}

ACTION fantasy::regdistevent(uint32_t event_id, vector<uint8_t> option_ids, uint64_t event_close_time){
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto event_itr = _distribution_event_registration_table.find(event_id);

  if(event_itr == _distribution_event_registration_table.end()) {
    _distribution_event_registration_table.emplace(get_self(), [&](auto& row){
      row.event_id = event_id;
      row.option_ids = option_id;
      row.event_close_time = event_close_time;
      row.event_status = INITIATING;
    });
  } else {
      eosio::check(false, "Event already exists. Update event using other actions");
  }
}

ACTION fantasy::upddisoption(uint32_t event_id, vector<uint8_t> option_ids) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row){row.option_ids = option_ids;});

}


ACTION fantasy::upddisclotim(uint32_t event_id, uint64_t event_close_time) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row){row.event_close_time = event_close_time;});

}


ACTION fantasy::addoutcome(uint32_t event_id, uint32_t winning_option_id) {
  require_auth(get_self());

  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
    {row.outcome_option_id = winning_option_id;
    row.event_status = CLOSED;});
}

ACTION fantasy::adddisusrsel(name user, event_id, option_id) {
  require_auth(user);
  // check if event_id exists
  // check if voting for event_id is not closed
  // check if option_id is a valid option_id.

    distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

    auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");
    vector<uint32_t> option_ids= event_itr->option_ids;
    std::vector<int>::iterator option_itr;

    option_itr = find (option_ids.begin(), option_ids.end(), option_id);
    eosio::check(option_itr != option_itr.end(), "Invalid option");
   
    uint64_t event_close_time = event_itr->event_close_time;
    if(event_close_time<eosio::current_time_point().sec_since_epoch(), "Event voting closed");

    


}

ACTION fantasy::selection(name user, uint32_t option_id, uint32_t event_id) {
  // TODO find better way to search for subset of record
  require_auth(user);
  selection_table _selection_table(get_self(), get_self().value);
  auto users_index = _selection_table.get_index<name("userkey")>();
  auto users_itr = users_index.find(user.value);
  bool found = false;
  if (users_itr == users_index.end()) {
    // doesnt exist throw error
  } else 
  {
    while(users_itr!=users_index.end()) {
      if(users_itr->user!=user) {
        // records for this user are exhaused
        break;
      } else {
        if(users_itr->event_id == event_id) {
          found = true;
          users_index.modify(users_itr, get_self(), [&](auto& modrec){
          modrec.option_id = option_id;});
          break;
        }
      }
      users_itr ++ ;
    }
  } 
  if(found==false) {
    _selection_table.emplace(get_self(), [&](auto& newrec) {
      newrec.id = _selection_table.available_primary_key();
      newrec.user = user;
      newrec.event_id = event_id;
      newrec.option_id = option_id;});
  }
}


EOSIO_DISPATCH(fantasy, (adduser)(selection)(regevent)(regoption)(closeevent))
