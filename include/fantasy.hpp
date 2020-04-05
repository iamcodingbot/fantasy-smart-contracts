#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

CONTRACT fantasy : public contract {
  public:
    using contract::contract;

    ACTION adduser(name user);

    ACTION selection(name user, uint32_t option_id, uint32_t event_id); 

    ACTION regevent(
      uint8_t event_type, 
      string event_text, 
      string event_closes_at);

    ACTION regoption(
      uint64_t option_type,
      uint64_t event_id,
      uint64_t option_text);  

    ACTION closeevent(
      uint64_t event_id,
      uint64_t option_id,
      string outcome_text); 

    ACTION openfansel(
      name user,
      vector<uint64_t> depending_events,
      vector<uint64_t> selection_data
    );   
  

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
      uint8_t weight;
      vector<uint32_t> selected_players;
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
