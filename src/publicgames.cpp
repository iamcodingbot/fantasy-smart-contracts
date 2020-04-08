#include <eosio/system.hpp>
#include <vector>

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