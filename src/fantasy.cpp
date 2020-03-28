#include <fantasy.hpp>

ACTION fantasy::adduser(name user) {
  require_auth(get_self());

  users_table _users_table(get_self(), get_self().value);

  auto users_itr = _users_table.find(user.value);
  if (users_itr == _users_table.end()) {
    // Create a message record if it does not exist
    _users_table.emplace(get_self(), [&](auto& row) {
      row.user = user;
    });
  }
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

ACTION fantasy::regevent(
    uint8_t event_type, 
    string event_text, 
    string event_closes_at) {
  require_auth(get_self());
  events_table _events_table(get_self(), get_self().value);

    _events_table.emplace(get_self(), [&](auto & row) {
    row.event_id = _events_table.available_primary_key();
    row.event_type = event_type;
    row.event_text = event_text;
    row.event_closes_at = event_closes_at;
  });
}

ACTION fantasy::regoption(
    uint64_t option_type,
    uint64_t event_id,
    uint64_t option_text) {

  require_auth(get_self());
  events_table _events_table(get_self(), get_self().value);
  auto& event = _events_table.get(event_id, "Event does not exist");
  options_table _options_table(get_self(), get_self().value);

    _options_table.emplace(get_self(), [&](auto & row) {
      row.option_id = _options_table.available_primary_key();
      row.option_type = option_type;
      row.event_id = event_id;
      row.option_text = option_text;
    });
}


ACTION fantasy::closeevent(uint64_t event_id, uint64_t option_id, string outcome_text){
  require_auth(get_self());
  options_table _options_table(get_self(), get_self().value);   
  auto& option = _options_table.get(option_id, "Option does not exist in options table");

  auto users_index = _options_table.get_index<name("eventkey")>();
  auto& users_itr = users_index.get(event_id, "Event does not exist in options table");

  events_table _events_table(get_self(), get_self().value);
  auto& event = _events_table.get(event_id, "Event does not exist in events table");




  _events_table.modify(event, get_self(), [&](auto& row){row.event_status = CLOSED;});
      
  outcome_table _outcome_table(get_self(), get_self().value);
  auto outcome_itr = _outcome_table.find(event_id);

  if(outcome_itr == _outcome_table.end()) {
        _outcome_table.emplace(get_self(), [&](auto & row) {
          row.event_id = event_id;
          row.option_id = option_id;
          row.outcome_text = outcome_text;});
  } else {
        _outcome_table.modify(outcome_itr, get_self(), [&](auto& row){
          row.option_id = option_id;
          row.outcome_text = outcome_text;});
  }


}

EOSIO_DISPATCH(fantasy, (adduser)(selection)(regevent)(regoption)(closeevent))
