#include <eosio/system.hpp>
#include <eosio/asset.hpp>

#include <vector>

ACTION fantasy::fboot(uint32_t fantasy_event_id, uint16_t total_cost_limit, 
        uint8_t max_players, uint8_t max_players_per_team, 
        uint8_t max_bat, uint8_t max_bowl, uint8_t max_wk, 
        uint8_t max_ar, 
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
      row.total_event_weight = total_event_weight;
      row.distribution_type = distribution_type;
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
      p.score = 0;
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

ACTION fantasy::fuserselect(name user, uint32_t fantasy_event_id,
       vector<uint32_t> selected_players, uint16_t weight) {
  require_auth(user);
  users_table _users_table(get_self(), get_self().value);
  
  // check if user is a valid user
  auto& user_itr = _users_table.get(user.value, "Invalid user");

  // check if user is blacklisted
  if(user_itr.user_status == BLACK_LIST) {
    eosio::check(false, "Can not participate, user blacklisted");
  }

  fantasy_md_table _fantasy_md_table(get_self(), get_self().value);

  auto fantasy_md_itr = _fantasy_md_table.find(fantasy_event_id);
  check(fantasy_md_itr != _fantasy_md_table.end(), "Invalid fantasy_event_id");

  check(fantasy_md_itr->fantasy_event_status == FANTASY_OPEN, "Event status != FANTASY_OPEN");
 
  vector<player> players = fantasy_md_itr->base_player_data;
  uint32_t selected_players_cost = 0;
  uint8_t total_bat = 0;
  uint8_t total_bowl = 0;
  uint8_t total_ar = 0;
  uint8_t total_wk = 0;

  check(selected_players.size()<=fantasy_md_itr->max_players, "Exceeded total players");
  
  map<uint16_t, uint8_t> team_count_map;
  map <uint32_t, player> player_map;
  for(int i =0; i<selected_players.size(); i++) {
    uint32_t this_player_id = selected_players[i];
    auto itr = player_map.find(this_player_id);
    check(itr!=player_map.end(), "invalid player selected");
    selected_players_cost = selected_players_cost + itr->second.cost;
    if(itr->second.player_type_id == BAT) {
      total_bat++;
    } else if (itr->second.player_type_id == BOWL) {
      total_bowl++;
    } else if (itr->second.player_type_id == AR) {
      total_ar++;
    } else if (itr->second.player_type_id == WK) {
      total_wk++;
    } else {
      check(false, "Invalid player_type_id");
    }

    auto team_itr = team_count_map.find(itr->second.team_id);
    if(team_itr == team_count_map.end()) {
      team_count_map.insert(pair<uint16_t, uint8_t>(itr->second.team_id, 1));
    } else {
      int new_count = team_itr->second + 1;
      team_count_map.insert(pair<uint16_t, uint8_t>(itr->second.team_id, new_count));
    }
  }
  check(total_bat<=fantasy_md_itr->max_bat, "more batsmen");
  check(total_bowl<=fantasy_md_itr->max_bowl, "more bowlers");
  check(total_ar<=fantasy_md_itr->max_ar, "more all rounders");
  check(total_wk<=fantasy_md_itr->max_wk, "more wicket keepers");
  check(selected_players_cost<=fantasy_md_itr->total_cost_limit, "more wicket keepers");

  map<uint16_t, uint8_t>::iterator cnt_itr; 
  for (cnt_itr = team_count_map.begin(); cnt_itr != team_count_map.end(); ++cnt_itr) { 
      check(cnt_itr->second<=fantasy_md_itr->max_players_per_team, "players exceeded from one team"); 
  } 
  fantasy_tx_table _fantasy_tx_table(get_self(), get_self().value);

  _fantasy_tx_table.emplace(get_self(), [&](auto& row){
      row.id = _fantasy_tx_table.available_primary_key();
      row.fantasy_event_id = fantasy_event_id;
      row.user = user;
      row.weight = weight;
      row.selected_players = selected_players;
    });
}

