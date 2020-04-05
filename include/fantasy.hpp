#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

CONTRACT fantasy : public contract {
  public:
    using contract::contract;

    // action to add new user
    ACTION adduser(name user);

    // action to update verification status user
    ACTION verifyuser(name user);

    // action to add new distrbution event and associated options for token distribution
    ACTION regdistevent(event_id, vector<uint8_t> option_ids, date event_close_time);

    // action to add winning option for a distribution event
    ACTION addoutcome(event_id, option_id);

    // action for token distribution for a closed distribution event
    ACTION disttoken(event_id); 

    // action to add user selection for a distribution event
    ACTION adddisusrsel(name user, event_id, option_id);

    // action to register fantasy events, its rules and associated players
    ACTION regfanevent(fantasy_event_id, cost_limit, max_players, max_players_per_team, 
          max_bat, max_bowl, max_wk, max_ar, max_participants, base_player_data);

    // action to add user selection for fantasy events
    ACTION addfanusrsel(name user, fantasy_event_id,
        uint8_t weight, vector<uint32_t> selected_player_ids);  

    // action to calculate score for fantasy event and reward coins to winners
    ACTION scorereward(scorecard??, fantasy_event_id)
  

  private:

    struct player {
      uint32_t player_id;
      uint16_t team_id;
      uint8_t cost;
      uint8_t player_type_id;
    };

    TABLE fantasy_meta_data {
      uint32_t fantasy_event_id;      
      uint16_t total_cost_limit;
      uint8_t max_players;
      uint8_t max_players_per_team;
      uint8_t max_bat;
      uint8_t max_bowl;
      uint8_t max_wk;
      uint8_t max_ar;
      uint8_t max_participants;
      vector<player> base_player_data;
      uint8_t fantasy_event_status;
      auto primary_key() const { return fantasy_event_id;}
    };
    typedef multi_index<name("fantasymd"), fantasy_meta_data> fantasy_meta_data_table;

    TABLE fantasy_user_selection {
      uint128_t user_selection_id;
      uint32_t fantasy_event_id;  
      name user;
      uint16_t weight;
      vector<uint32_t> selected_players;
      uint16_t user_score;
      auto primary_key() const { return user_selection_id; }
    };
    typedef multi_index<name("fantasyus"), fantasy_user_selection> fantasy_user_selection_table;

    enum user_status: uint8_t {
      NOT_VERIFIED= 0,
      VERIFIED=1 
    };

    TABLE registered_users {
      name user;
      uint8_t user_status;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("users"), registered_users> users_table;

   enum event_status: uint8_t {
      INITIATING= 0,
      VOTE_STARTING= 1,
      VOTE_ENDED= 2,
      CLOSED= 3,
      DISTRIBUTION_STARTED= 4,
      DISTRIBUTION_ENDED= 5 
    };

    TABLE distribution_event_registration {
      uint32_t event_id;
      vector<uint32_t> option_id;
      date event_close_time;
      uint8_t event_status = INITIATING;
      uint32_t outcome_option_id;
      date distribution_end_time;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("disteventreg"), distribution_event_registration> distribution_event_registration_table;

    TABLE distribution_user_selection {
      uint128_t selection_id;
      name user;
      uint32_t  event_id;
      uint32_t option_id;
      auto primary_key() const { return selection_id; }
      uint128_t user_key() const {
        return user.value;
      }
    };
    typedef multi_index<name("distusersel"), distribution_user_selection,
    indexed_by<name("userkey"), 
    const_mem_fun<distribution_user_selection, uint128_t, &distribution_user_selection::user_key>>
    > distribution_user_selection_table;
};
