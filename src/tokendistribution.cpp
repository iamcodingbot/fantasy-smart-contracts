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
  _initiate_stats(event_id, option_ids);

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

  distribution_stats_table _distribution_stats_table(get_self(), get_self().value);
  auto stats_events_index = _distribution_stats_table.get_index<name("eventkey")>();
  auto stats_events_itr = stats_events_index.find(event_id);

  check(stats_events_itr != stats_events_index.end(), "sure about event id?");

  uint32_t total_participants = 0;
  uint32_t total_winning_participants = 0;
  uint32_t total_rewards = 0;

  while(stats_events_itr->event_id == event_id) {
    total_participants = stats_events_itr->count + total_participants;
    if(stats_events_itr->option_id == winning_option_id) {
      total_winning_participants = stats_events_itr->count;
    }
    stats_events_itr++;
  }
  
  set_total_rewards_state(total_participants, total_rewards);

  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
    {row.outcome_option_id = winning_option_id;
    row.total_participants = total_participants;
    row.winning_participants = total_winning_participants;
    row.total_rewards = total_rewards;
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
        // updating stats
        int8_t incr = 1;
        int8_t decr = -1;
        uint32_t curr_option_id = users_itr->option_id;
        _mod_count(event_id, curr_option_id, decr);
        _mod_count(event_id, option_id, incr);
        // updating user option
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
    // updating stats
     int8_t incr = 1;
     _mod_count(event_id, option_id, incr);
  }

}
// void token::issue( const name& to, const asset& quantity, const string& memo )
// todo put total rewards as asset
ACTION fantasy::issue(name to, uint32_t event_id, asset q, string m) {
  distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);

  auto event_itr = _distribution_event_registration_table.find(event_id);

  check(event_itr != _distribution_event_registration_table.end(), "invalid event");

  uint32_t amount = event_itr->total_rewards;

  eosio::asset quantity(amount, eosio::symbol("FANTASY",4));

  action{
    permission_level{get_self(), "active"_n},
    "eosio.token"_n,
    "issue"_n,
    std::make_tuple(to, quantity , std::string("issued"))
  }.send();
    
  _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
      {row.event_status = ISSUED;});

}

ACTION fantasy::distribute(uint32_t event_id, uint16_t batch_size) {
    distribution_event_registration_table _distribution_event_registration_table(get_self(), get_self().value);
    auto distrib_events_itr = _distribution_event_registration_table.find(event_id);

    check(distrib_events_itr != _distribution_event_registration_table.end(),
      "invalid event id");

    check(distrib_events_itr->event_status == ISSUED, "not ready for distribution");  
    uint32_t winning_option_id = distrib_events_itr->outcome_option_id;

    uint32_t total_participants = distrib_events_itr->total_participants;
    uint32_t total_winning_participants = distrib_events_itr->winning_participants;
    uint32_t total_rewards = distrib_events_itr->total_rewards;
    uint32_t per_vote = total_rewards/total_winning_participants;

    distribution_user_selection_table _distribution_user_selection_table(get_self(), get_self().value);
    auto user_selection_index = _distribution_user_selection_table.get_index<name("eventkey")>();
    check(user_selection_index.lower_bound(event_id) != user_selection_index.end(), "Distribution ended");
    for (auto i = user_selection_index.lower_bound(event_id); i != user_selection_index.upper_bound(event_id);) {
    eosio::asset x(per_vote, eosio::symbol("FANTASY",4));
        if(i->option_id == winning_option_id) {
            action{
              permission_level{get_self(), "active"_n},
              "eosio.token"_n,
              "transfer"_n,
              std::make_tuple(name("fantasy"), i->user,x, std::string("Party! Your hodl is free."))
            }.send();
        }
        i = user_selection_index.erase(i);
        if(--batch_size == 0) {
          break;
        }
    }
    if(user_selection_index.lower_bound(event_id) == user_selection_index.end()) {
      _distribution_event_registration_table.modify(event_itr, get_self(), [&](auto& row)
        {row.event_status = DISTRIBUTION_CLOSED;});
    }

}
void fantasy::set_total_rewards_state(uint32_t& total_participants, uint32_t& total_rewards) {
      total_rewards = 20000*total_participants;
}
void fantasy::_mod_count( uint32_t& event_id, uint32_t& option_id, int8_t& delta) {
    distribution_stats_table _distribution_stats_table(get_self(), get_self().value);
    auto events_index = _distribution_stats_table.get_index<name("eventkey")>();
    auto events_itr = events_index.find(event_id);
    check(events_itr!=events_index.end(), "Are you sure of event id?");

    while(events_itr!=events_index.end() && events_itr->event_id == event_id) {
      if(events_itr->option_id==option_id) {
        uint32_t curr_count = events_itr->count;
        events_index.modify(events_itr, get_self(), [&](auto& modrec){
          modrec.count = curr_count + delta;
        });
        break;
      }
      events_itr++;
    }
}

void fantasy::_initiate_stats( uint32_t& event_id, vector<uint32_t>& option_ids) {
    distribution_stats_table _distribution_stats_table(get_self(), get_self().value);
    auto events_index = _distribution_stats_table.get_index<name("eventkey")>();
    for (auto i = events_index.lower_bound(event_id); i != events_index.upper_bound(event_id);) {
        i = events_index.erase(i);
    }

   for(int i = 0; i < option_ids.size(); i++) {
        _distribution_stats_table.emplace(get_self(), [&](auto& row){
          row.stat_id = _distribution_stats_table.available_primary_key();
          row.event_id = event_id;
          row.option_id = option_ids[i];
          row.count = 0;});
    } 
}