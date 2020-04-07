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
    _distribution_event_registration_table.modify(event_itr, get_self(),[&](auto& row){
      row.option_ids = option_id;
      row.event_close_time = event_close_time;
    });
  }
}

ACTION fantasy::opendisevent(uint32_t event_id) {
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

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
    {row.outcome_option_id = winning_option_id;
    row.event_status = CLOSED;});
}

ACTION fantasy::adddisusrsel(name user, event_id, option_id) {
  require_auth(user);

  users_table _users_table(get_self(), get_self().value);

  
  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  // check if user is a valid user
  auto& user_itr = _users_table.get(user.value, "Invalid user");

  // check if user is blacklisted
  if(user_itr->user_status == BLACK_LIST) {
    eosio::check(false, "Can not participate, user blacklisted");
  }

  // check if event_id exists
  auto& event_itr = _distribution_event_registration_table.get(event_id, "Invalid eventId");
  
  // check if option_id is a valid option_id.
  vector<uint32_t> valid_option_ids= event_itr->option_ids;
  std::vector<int>::iterator option_itr;
  option_itr = find (valid_option_ids.begin(), valid_option_ids.end(), option_id);
  eosio::check(option_itr != option_itr.end(), "Invalid option");

  // check if voting for event_id is not closed
  uint64_t event_close_time = event_itr->event_close_time;
  if(event_close_time<eosio::current_time_point().sec_since_epoch(), "Event voting closed");

  // check if voting for event_id is open
  if(event_itr->event_status!="OPEN", "Event voting not in OPEN state");

  // find {event, user} combo and update an option if combo exists
  distribution_user_selection _distribution_user_selection(get_self(), get_self().value);
  auto users_index = _distribution_user_selection.get_index<name("userkey")>();
  auto users_itr = users_index.find(user.value);

  boolean found = false;
  while(users_itr!=users_index.end()){
      if(users_itr->event_id == event_id) {
        found = true;
        users_index.modify(users_itr, get_self(), [&](auto& modrec){
        modrec.option_id = option_id;});
        break;
      }
  }

  // if {event , user} combo does not exist, create a new entry
  if(found==false) {
    _distribution_user_selection.emplace(get_self(), [&](auto& newrec) {
      newrec.selection_id = _distribution_user_selection.available_primary_key();
      newrec.user = user;
      newrec.event_id = event_id;
      newrec.option_id = option_id;
    });
  }

}

ACTION fantasy::regfanevent(uint32_t fantasy_event_id, uint16_t cost_limit, uint8_t max_players, uint8_t max_players_per_team, 
          uint8_t max_bat, uint8_t max_bowl, uint8_t max_wk, uint8_t max_ar, uint32_t max_participants, player base_player_data) {
  require_auth(get_self());
  fantasy_meta_data _fantasy_meta_data(get_self(), get_self().value);

  auto fantasy_meta_data_itr = _fantasy_meta_data.find(fantasy_event_id);
  
  if(fantasy_meta_data_itr == _fantasy_meta_data.end()){
    _fantasy_meta_data.emplace(get_self(), [&](auto& row){
      row.fantasy_event_id = fantasy_event_id;
      row.cost_limit = cost_limit;
      row.max_players = max_players;
      row.max_players_per_team = max_players_per_team;
      row.max_bat = max_bat;
      row.max_bowl = max_bowl;
      row.max_ar = max_ar;
      row.max_participants = max_participants;
      row.base_player_data = base_player_data;
    });
  } else {
    _fantasy_meta_data.modify(fantasy_meta_data_itr, get_self(),[&](auto& row){
      row.cost_limit = cost_limit;
      row.max_players = max_players;
      row.max_players_per_team = max_players_per_team;
      row.max_bat = max_bat;
      row.max_bowl = max_bowl;
      row.max_ar = max_ar;
      row.max_participants = max_participants;
      row.base_player_data = base_player_data;
    });
  }

}

EOSIO_DISPATCH(fantasy, (adduser)(selection)(regevent)(regoption)(closeevent))
