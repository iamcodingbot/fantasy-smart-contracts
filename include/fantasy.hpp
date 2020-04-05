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
      uint64_t player_id;
      uint64_t team_id;
      uint8_t cost;
      uint8_t player_type_id;
    };

    TABLE fantasy_meta_data {
      uint8_t fantasy_event_id;      
      uint64_t total_cost_limit;
      uint64_t max_players;
      uint64_t max_players_per_team;
      uint64_t max_bat;
      uint64_t max_bowl;
      uint64_t max_wk;
      uint64_t max_ar;
      uint64_t max_participants;
      vector<player> base_player_data;
      uint8_t fantasy_event_status;
      auto primary_key() const { return fantasy_event_id;}
    };
    typedef multi_index<name("fantasymd"), fantasy_meta_data> fantasy_meta_data_table;

    TABLE fantasy_user_selections {
      uint64_t user_selection_id;
      uint64_t fantasy_event_id;
      name user;
      uint64_t weight;
      vector<uint64_t> selected_players;
      auto primary_key() const { return user_selection_id; }
    };
    typedef multi_index<name("fantasyus"), fantasy_user_selections> fantasy_user_selections_table;


    TABLE registered_users {
      name user;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("users"), registered_users> users_table;

   enum event_status: uint64_t {
      INITIATING= 0,
      OPEN= 1,
      STARTING_NOVOTE= 2,
      ONGOING_NOVOTE= 3,
      CLOSED= 4 
    };

    enum distribution_status: uint8_t {
      REGISTERED = 0,
      STARTED= 1,
      ONGOING= 2,
      DONE = 3
    };

    TABLE distribution_event_registration {
      uint64_t event_id;
      date event_close_time;
      uint8_t event_status = INITIATING;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("regevents"), event_registration> events_table;


   TABLE option_registration {
      uint64_t option_id;
      uint64_t event_id;
      auto primary_key() const { return option_id;}
      uint64_t event_key() const {return event_id;}
    };
    typedef multi_index<
      name("options"), 
      option_registration,
      indexed_by<name("eventkey"), const_mem_fun<option_registration, uint64_t, &option_registration::event_key>>
    > options_table;


  TABLE event_outcome {
      uint64_t event_id;
      uint64_t option_id;
      uint8_t distribution_status = REGISTERED;
      date distribution_end_time;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("outcome"), event_outcome> outcome_table;


    TABLE user_selection {
      uint64_t id;
      name user;
      uint64_t  event_id;
      uint64_t option_id;
      uint64_t primary_key() const { return id; }
      uint128_t user_key() const {
        return user.value;
      }
    };
    typedef multi_index<name("selection"), user_selection,
    indexed_by<name("userkey"), 
    const_mem_fun<user_selection, uint128_t, &user_selection::user_key>>
    > selection_table;
};
