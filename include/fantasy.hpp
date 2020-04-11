#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT fantasy : public contract {
  public:
    using contract::contract;

    /* action to add new user. user will be able to 
    *  participate but (TODO)funds will not be available for claiming
    */
    ACTION initiateuser(name user);

    // action to indicate successful kyc. ideal status for user.
    ACTION kycuser(name user);

    /*
    * action to block a user. user will not be able to participate 
    * in any event and (TODO)funds will be given for any open event 
    * in which user is participating
    */ 
    ACTION blockuser(name user);

    // action to add/modify distribution event and associated options for token distribution
    ACTION adddistevent(uint32_t event_id, vector<uint32_t> option_ids, uint64_t event_close_time);

    // action to open an event for voting
    ACTION openvoting(uint32_t event_id);

    // action to add winning option for a distribution event
    ACTION addoutcome(uint32_t event_id, uint32_t option_id);

    // action to add user selection for a distribution event
    ACTION useroption(name user, uint32_t event_id, uint32_t option_id);

    // action to register fantasy event, associated rules 
    ACTION regfanevent(uint32_t fantasy_event_id, uint16_t total_cost_limit, 
        uint8_t max_players, uint8_t max_players_per_team, 
        uint8_t max_bat, uint8_t max_bowl, uint8_t max_wk, 
        uint8_t max_ar, uint32_t max_participants);

    // action to add a player to fantasy event
    ACTION addplayer(uint32_t fantasy_event_id, 
        uint32_t player_id, 
        uint16_t team_id,
        uint8_t cost,
        uint8_t player_type_id);  

    // action to open fantasy event
    ACTION openfanevent(uint32_t fantasy_event_id);  

    // action to add fantasy user selection
    ACTION fanselection(name user, uint32_t fantasy_event_id,
       vector<uint32_t> selected_players, uint16_t weight);  

    ACTION issue(name to, asset q, string m);
    ACTION distribute(uint32_t event_id, uint16_t batch_size);
  private:

    enum user_status: uint8_t {
      NOT_VERIFIED= 0,
      VERIFIED=1,
      BLACK_LIST=2 
    };

    TABLE registered_users {
      name user;
      uint8_t user_status;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("users"), registered_users> users_table;

   enum event_status: uint8_t {
      INITIATING= 0,
      OPEN= 1,
      CLOSED= 2,
      ISSUED=3,
      DISTRIBUTION_CLOSED= 4 
    };

    TABLE distribution_event {
      uint32_t event_id;
      vector<uint32_t> option_ids;
      uint64_t event_close_time;
      uint8_t event_status = INITIATING;
      uint32_t outcome_option_id;
      uint64_t distribution_end_time;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("disteventreg"), distribution_event> distribution_event_registration_table;

    TABLE distribution_user_selection {
      uint128_t selection_id;
      name user;
      uint64_t  event_id;
      uint32_t option_id;
      auto primary_key() const { return selection_id; }
      uint128_t user_key() const {
        return user.value;
      }
      uint64_t event_key() const {
        return event_id;
      }
    };
    typedef multi_index<name("distusersel"), distribution_user_selection,
    indexed_by<name("userkey"), const_mem_fun<distribution_user_selection, uint128_t, &distribution_user_selection::user_key>>,
    indexed_by<name("eventkey"), const_mem_fun<distribution_user_selection, uint64_t, &distribution_user_selection::event_key>>
    > distribution_user_selection_table;

    TABLE distribution_stats {
      uint64_t stat_id;
      uint64_t event_id;
      uint32_t option_id;
      uint32_t count;
      auto primary_key() const { return stat_id; }
      uint64_t event_key() const {
        return event_id;
      }
    };
    typedef multi_index<name("diststats"), distribution_stats,
    indexed_by<name("eventkey"), const_mem_fun<distribution_stats, uint64_t, &distribution_stats::event_key>>
    > distribution_stats_table;

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
      uint32_t max_participants;
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

    void _mod_count( uint32_t& event_id, uint32_t& option_id, int8_t& delta);
    void _initiate_stats( uint32_t& event_id, vector<uint32_t>& option_ids);
    void set_total_rewards_state(uint32_t& total_participants, uint32_t& total_rewards);

};
