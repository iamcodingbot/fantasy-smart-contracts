#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT fantasy : public contract {
  public:
    using contract::contract;

    ACTION initiateuser(name user);
    ACTION kycuser(name user);
    ACTION blockuser(name user);

    ACTION initiate(uint32_t event_id, vector<uint32_t> option_ids, uint64_t event_close_time);
    ACTION openvote(uint32_t event_id);
    ACTION closevote(uint32_t event_id);
    ACTION eventclose(uint32_t event_id, uint32_t option_id);
    ACTION tokenissue(name to, uint32_t event_id, asset q, string m);
    ACTION publicxfer(uint32_t event_id, uint16_t batch_size);
    ACTION devxfer(uint32_t event_id, name dev_account);
    ACTION cleanupdist(uint32_t event_id);

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
      INITIATE= 0,
      OPEN_VOTE= 1,
      CLOSE_VOTE= 2,
      EVENT_CLOSE= 3,
      TOKEN_ISSUE=4,
      PUBLIC_TOKEN_TRANSFER= 5,
      DEV_TOKEN_TRANSFER= 6 
    };

    TABLE event_metadata
    {
      uint32_t event_id;
      vector<uint32_t> option_ids;
      uint64_t vote_close_time;
      uint8_t status = INITIATING;
      uint32_t winning_option_id;
      uint32_t total_participants;
      uint32_t winning_participants;
      uint32_t total_rewards;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("eventmd"), event_metadata> event_metadata_table;

    TABLE event_tx {
      uint128_t id;
      name user;
      uint64_t  event_id;
      uint32_t option_id;
      auto primary_key() const { return id; }
      uint128_t user_key() const {
        return user.value;
      }
      uint64_t event_key() const {
        return event_id;
      }
    };
    typedef multi_index<name("eventtx"), event_tx,
    indexed_by<name("userkey"), const_mem_fun<event_tx, uint128_t, &event_tx::user_key>>,
    indexed_by<name("eventkey"), const_mem_fun<event_tx, uint64_t, &event_tx::event_key>>
    > event_tx_table;

    TABLE event_stats {
      uint64_t stat_id;
      uint64_t event_id;
      uint32_t option_id;
      uint32_t count;
      auto primary_key() const { return stat_id; }
      uint64_t event_key() const {
        return event_id;
      }
    };
    typedef multi_index<name("eventstats"), event_stats,
    indexed_by<name("eventkey"), const_mem_fun<event_stats, uint64_t, &event_stats::event_key>>
    > event_stats_table;

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
