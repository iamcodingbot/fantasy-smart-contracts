#include <eosio/system.hpp>
#include <vector>

ACTION fantasy::fboot(uint32_t fantasy_event_id, uint16_t total_cost_limit, 
        uint8_t max_players, uint8_t max_players_per_team, 
        uint8_t max_bat, uint8_t max_bowl, uint8_t max_wk, 
        uint8_t max_ar, uint32_t max_participants, 
        uint8_t distribution_type, uint32_t total_event_weight) {

  require_auth(get_self());
  fantasy_md_table _fantasy_md_table(get_self(), get_self().value);

  auto fantasy_md_itr = _fantasy_md_table.find(fantasy_event_id);
  
  if(fantasy_md_itr == _fantasy_md_table.end()){
    _fantasy_md_table.emplace(get_self(), [&](auto& row){
      row.fantasy_event_id = fantasy_event_id;
      row.total_cost_limit = total_cost_limit;
      row.max_players = max_players;
      row.max_players_per_team = max_players_per_team;
      row.max_bat = max_bat;
      row.max_bowl = max_bowl;
      row.max_ar = max_ar;
      row.max_wk = max_wk;
      row.max_participants = max_participants;
      row.distribution_type = distribution_type;
      row.total_event_weight = total_event_weight;
      row.fantasy_event_status = FANTASY_BOOTING;
    });
  } else {
    check(fantasy_md_itr->fantasy_event_status == FANTASY_BOOTING,
     "event status does not allow modification");  
    _fantasy_md_table.modify(fantasy_md_itr, get_self(),[&](auto& row){
      row.total_cost_limit = total_cost_limit;
      row.max_players = max_players;
      row.max_players_per_team = max_players_per_team;
      row.max_bat = max_bat;
      row.max_bowl = max_bowl;
      row.max_ar = max_ar;
      row.max_wk = max_wk;
      row.max_participants = max_participants;
      row.distribution_type = distribution_type;
      row.total_event_weight = total_event_weight;
    });
  }

}

// validate inputs before calling.

ACTION fantasy::fplayeradd(uint32_t fantasy_event_id, 
  uint32_t player_id, 
  uint16_t team_id,
  uint8_t cost,
  uint8_t player_type_id) {

  require_auth(get_self());
  fantasy_md_table _fantasy_md_table(get_self(), get_self().value);

  auto fantasy_md_itr = _fantasy_md_table.find(fantasy_event_id);
  check(fantasy_md_itr != _fantasy_md_table.end(), "Invalid fantasy_event_id");

  check(fantasy_md_itr->fantasy_event_status == FANTASY_BOOTING, "Event status != FANTASY_BOOTING");
  
  _fantasy_md_table.modify(fantasy_md_itr, get_self(),[&](auto& row){
    vector<player> &players = row.base_player_data;
      player p;

      p.player_id = player_id;
      p.team_id = team_id;
      p.cost =cost;
      p.player_type_id =player_type_id;
      row.base_player_data.push_back(p);
  });
}

ACTION fantasy::fopen(uint32_t fantasy_event_id) {
  require_auth(get_self());
  fantasy_md_table _fantasy_md_table(get_self(), get_self().value);

  auto fantasy_md_itr = _fantasy_md_table.find(fantasy_event_id);
  check(fantasy_md_itr != _fantasy_md_table.end(), "Invalid fantasy_event_id");

  check(fantasy_md_itr->fantasy_event_status == FANTASY_BOOTING, "Event status != FANTASY_BOOTING");
  

  _fantasy_md_table.modify(fantasy_md_itr, get_self(),[&](auto& row){
      row.fantasy_event_status = FANTASY_OPEN;
  });
  // check duplicate player id;
}

ACTION fantasy::fanselection(name user, uint32_t fantasy_event_id,
       vector<uint32_t> selected_players, uint16_t weight) {
  require_auth(get_self());
  // valid user, not blacklisted
  // valid  fantasy_event_id
  // valid token symbol
  // valid selected_players
  // valid rules of the game
  // valid tokens quantity   
  // insert into table   
}

