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

    ACTION closeevent(uint64_t event_id,
      uint64_t option_id,
      string outcome_text);  
  

  private:

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

    TABLE event_registration {
      uint64_t event_id;
      uint8_t event_type;
      string event_text;
      string event_closes_at;
      uint8_t event_status = INITIATING;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("regevents"), event_registration> events_table;


   TABLE option_registration {
      uint64_t option_id;
      uint64_t option_type;
      uint64_t event_id;
      uint64_t option_text;
      auto primary_key() const { return option_id; }
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
      string outcome_text;
      uint8_t distribution_status = REGISTERED;
      auto primary_key() const { return event_id; }
    };
    typedef multi_index<name("outcome"), event_outcome> outcome_table;


    TABLE user_selection {
      uint64_t id;
      name    user;
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
