#pragma once

#include <appbase/application.hpp>
#include <blurt/chain/database.hpp>
#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>

#include <blurt/plugins/debug_node/debug_node_plugin.hpp>

#include <blurt/utilities/key_conversion.hpp>

#include <blurt/plugins/block_api/block_api_plugin.hpp>
#include <blurt/plugins/condenser_api/condenser_api_legacy_asset.hpp>
#include <blurt/plugins/database_api/database_api_plugin.hpp>

#include <fc/network/http/connection.hpp>
#include <fc/network/ip.hpp>

#include <array>
#include <iostream>

extern uint32_t BLURT_TESTING_GENESIS_TIMESTAMP;

#define PUSH_TX \
   blurt::chain::test::_push_transaction

#define PUSH_BLOCK \
   blurt::chain::test::_push_block

// See below
#define REQUIRE_OP_VALIDATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   op.validate(); \
   op.field = temp; \
}
#define REQUIRE_OP_EVALUATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = temp; \
   db.push_transaction( trx, ~0 ); \
}

/*#define BLURT_REQUIRE_THROW( expr, exc_type )          \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "BLURT_REQUIRE_THROW begin "        \
         << req_throw_info << std::endl;                  \
   BOOST_REQUIRE_THROW( expr, exc_type );                 \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "BLURT_REQUIRE_THROW end "          \
         << req_throw_info << std::endl;                  \
}*/

#define BLURT_REQUIRE_THROW( expr, exc_type )          \
   BOOST_REQUIRE_THROW( expr, exc_type );

#define BLURT_CHECK_THROW( expr, exc_type )            \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "BLURT_CHECK_THROW begin "          \
         << req_throw_info << std::endl;                  \
   BOOST_CHECK_THROW( expr, exc_type );                   \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "BLURT_CHECK_THROW end "            \
         << req_throw_info << std::endl;                  \
}

#define REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, exc_type ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   BLURT_REQUIRE_THROW( op.validate(), exc_type ); \
   op.field = temp; \
}
#define REQUIRE_OP_VALIDATION_FAILURE( op, field, value ) \
   REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, fc::exception )

#define REQUIRE_THROW_WITH_VALUE_2(op, field, value, exc_type) \
{ \
   auto bak = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = bak; \
   BLURT_REQUIRE_THROW(db.push_transaction(trx, ~0), exc_type); \
}

#define REQUIRE_THROW_WITH_VALUE( op, field, value ) \
   REQUIRE_THROW_WITH_VALUE_2( op, field, value, fc::exception )

///This simply resets v back to its default-constructed value. Requires v to have a working assingment operator and
/// default constructor.
#define RESET(v) v = decltype(v)()
///This allows me to build consecutive test cases. It's pretty ugly, but it works well enough for unit tests.
/// i.e. This allows a test on update_account to begin with the database at the end state of create_account.
#define INVOKE(test) ((struct test*)this)->test_method(); trx.clear()

#define PREP_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name));   \
   fc::ecc::private_key name ## _post_key = generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "_post" ); \
   public_key_type name ## _public_key = name ## _private_key.get_public_key();

#define ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = account_create(BOOST_PP_STRINGIZE(name), name ## _public_key, name ## _post_key.get_public_key()); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define GET_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name)); \
   const account_object& name = get_account(BOOST_PP_STRINGIZE(name)); \
   account_id_type name ## _id = name.id; \
   (void)name ##_id

#define ACTORS_IMPL(r, data, elem) ACTOR(elem)
#define ACTORS(names) BOOST_PP_SEQ_FOR_EACH(ACTORS_IMPL, ~, names) \
   validate_database();

#define SMT_SYMBOL( name, decimal_places, db ) \
   asset_symbol_type name ## _symbol = get_new_smt_symbol( decimal_places, db );

#define ASSET( s ) \
   blurt::plugins::condenser_api::legacy_asset::from_string( s ).to_asset()

#define FUND( account_name, amount ) \
   fund( account_name, amount ); \
   generate_block();

// To be incorporated into fund() method if deemed appropriate.
// 'SMT' would be dropped from the name then.
#define FUND_SMT_REWARDS( account_name, amount ) \
   db->adjust_reward_balance( account_name, amount ); \
   db->adjust_supply( amount ); \
   generate_block();

#define OP2TX(OP,TX,KEY) \
TX.operations.push_back( OP ); \
TX.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION ); \
TX.sign( KEY, db->get_chain_id(), fc::ecc::bip_0062 );

#define PUSH_OP(OP,KEY) \
{ \
   signed_transaction tx; \
   OP2TX(OP,tx,KEY) \
   db->push_transaction( tx, 0 ); \
}

#define PUSH_OP_TWICE(OP,KEY) \
{ \
   signed_transaction tx; \
   OP2TX(OP,tx,KEY) \
   db->push_transaction( tx, 0 ); \
   db->push_transaction( tx, database::skip_transaction_dupe_check ); \
}

#define FAIL_WITH_OP(OP,KEY,EXCEPTION) \
{ \
   signed_transaction tx; \
   OP2TX(OP,tx,KEY) \
   BLURT_REQUIRE_THROW( db->push_transaction( tx, 0 ), EXCEPTION ); \
}

namespace blurt { namespace chain {

using namespace blurt::protocol;

struct database_fixture {
   // the reason we use an app is to exercise the indexes of built-in
   //   plugins
   chain::database* db = nullptr;
   signed_transaction trx;
   public_key_type committee_key;
   account_id_type committee_account;
   fc::ecc::private_key private_key = fc::ecc::private_key::generate();
   fc::ecc::private_key init_account_priv_key = fc::ecc::private_key::regenerate( fc::sha256::hash( string( "init_key" ) ) );
   string debug_key = blurt::utilities::key_to_wif( init_account_priv_key );
   public_key_type init_account_pub_key = init_account_priv_key.get_public_key();
   uint32_t default_skip = 0 | database::skip_undo_history_check | database::skip_authority_check;
   fc::ecc::canonical_signature_type default_sig_canon = fc::ecc::fc_canonical;

   plugins::debug_node::debug_node_plugin* db_plugin;

   optional<fc::temp_directory> data_dir;
   bool skip_key_index_test = false;

   database_fixture() {}
   virtual ~database_fixture() { appbase::reset(); }

   static fc::ecc::private_key generate_private_key( string seed = "init_key" );
   void open_database( uint16_t shared_file_size_in_mb = 8 );
   void generate_block(uint32_t skip = 0,
                               const fc::ecc::private_key& key = generate_private_key("init_key"),
                               int miss_blocks = 0);

   /**
    * @brief Generates block_count blocks
    * @param block_count number of blocks to generate
    */
   void generate_blocks(uint32_t block_count);

   /**
    * @brief Generates blocks until the head block time matches or exceeds timestamp
    * @param timestamp target time to generate blocks until
    */
   void generate_blocks(fc::time_point_sec timestamp, bool miss_intermediate_blocks = true);

   const account_object& account_create(
      const string& name,
      const string& creator,
      const private_key_type& creator_key,
      const share_type& fee,
      const public_key_type& key,
      const public_key_type& post_key,
      const string& json_metadata
   );

   const account_object& account_create(
      const string& name,
      const public_key_type& key,
      const public_key_type& post_key
   );

   const account_object& account_create(
      const string& name,
      const public_key_type& key
   );

   const witness_object& witness_create(
      const string& owner,
      const private_key_type& owner_key,
      const string& url,
      const public_key_type& signing_key,
      const share_type& fee
   );

   void fund( const string& account_name, const share_type& amount = 500000 );
   void fund( const string& account_name, const asset& amount );
   void transfer( const string& from, const string& to, const asset& amount );
   void vest( const string& from, const string& to, const asset& amount );
   void vest( const string& from, const share_type& amount );
   void proxy( const string& account, const string& proxy );
   void set_witness_props( const flat_map< string, vector< char > >& new_props );
   const asset& get_balance( const string& account_name )const;
   void sign( signed_transaction& trx, const fc::ecc::private_key& key );

   vector< operation > get_last_operations( uint32_t ops );

   void validate_database();
};

struct clean_database_fixture : public database_fixture
{
   clean_database_fixture( uint16_t shared_file_size_in_mb = 8 );
   virtual ~clean_database_fixture();

   void resize_shared_mem( uint64_t size );
   void validate_database();
};

struct live_database_fixture : public database_fixture
{
   live_database_fixture();
   virtual ~live_database_fixture();

   fc::path _chain_dir;
};

struct sps_proposal_database_fixture : public clean_database_fixture
{
   sps_proposal_database_fixture( uint16_t shared_file_size_in_mb = 8 )
                           : clean_database_fixture( shared_file_size_in_mb ){}
   virtual ~sps_proposal_database_fixture(){}

   void plugin_prepare();

   int64_t create_proposal(   std::string creator, std::string receiver,
                              time_point_sec start_date, time_point_sec end_date,
                              asset daily_pay, const fc::ecc::private_key& key );

   void vote_proposal( std::string voter, const std::vector< int64_t >& id_proposals, bool approve, const fc::ecc::private_key& key );

   bool exist_proposal( int64_t id );
   const proposal_object* find_proposal( int64_t id );

   void remove_proposal(account_name_type _deleter, flat_set<int64_t> _proposal_id, const fc::ecc::private_key& _key);

   bool find_vote_for_proposal(const std::string& _user, int64_t _proposal_id);

   uint64_t get_nr_blocks_until_maintenance_block();

   void post_comment( std::string _authro, std::string _permlink, std::string _title, std::string _body, std::string _parent_permlink, const fc::ecc::private_key& _key);
};

struct sps_proposal_database_fixture_performance : public sps_proposal_database_fixture
{
   sps_proposal_database_fixture_performance( uint16_t shared_file_size_in_mb = 512 )
                           : sps_proposal_database_fixture( shared_file_size_in_mb )
   {
      db->get_benchmark_dumper().set_enabled( true );
      db->set_sps_remove_threshold( -1 );
   }
};

struct json_rpc_database_fixture : public database_fixture
{
   private:
      blurt::plugins::json_rpc::json_rpc_plugin* rpc_plugin;

      fc::variant get_answer( std::string& request );
      void review_answer( fc::variant& answer, int64_t code, bool is_warning, bool is_fail, fc::optional< fc::variant > id );

   public:

      json_rpc_database_fixture();
      virtual ~json_rpc_database_fixture();

      void make_array_request( std::string& request, int64_t code = 0, bool is_warning = false, bool is_fail = true );
      fc::variant make_request( std::string& request, int64_t code = 0, bool is_warning = false, bool is_fail = true );
      void make_positive_request( std::string& request );
};

namespace test
{
   bool _push_block( database& db, const signed_block& b, uint32_t skip_flags = 0 );
   void _push_transaction( database& db, const signed_transaction& tx, uint32_t skip_flags = 0 );
}

} }
