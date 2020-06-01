#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <blurt/utilities/tempdir.hpp>
#include <blurt/utilities/database_configuration.hpp>

#include <blurt/chain/history_object.hpp>
#include <blurt/chain/blurt_objects.hpp>
#include <blurt/chain/sps_objects.hpp>

#include <blurt/plugins/account_history/account_history_plugin.hpp>
#include <blurt/plugins/chain/chain_plugin.hpp>
#include <blurt/plugins/rc/rc_plugin.hpp>
#include <blurt/plugins/webserver/webserver_plugin.hpp>
#include <blurt/plugins/witness/witness_plugin.hpp>

#include <blurt/plugins/condenser_api/condenser_api_plugin.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

//using namespace blurt::chain::test;

uint32_t BLURT_TESTING_GENESIS_TIMESTAMP = 1431700000;

using namespace blurt::plugins::webserver;
using namespace blurt::plugins::database_api;
using namespace blurt::plugins::block_api;
using blurt::plugins::condenser_api::condenser_api_plugin;

namespace blurt { namespace chain {

using std::cout;
using std::cerr;

clean_database_fixture::clean_database_fixture( uint16_t shared_file_size_in_mb )
{
   try {
   int argc = boost::unit_test::framework::master_test_suite().argc;
   char** argv = boost::unit_test::framework::master_test_suite().argv;
   for( int i=1; i<argc; i++ )
   {
      const std::string arg = argv[i];
      if( arg == "--record-assert-trip" )
         fc::enable_record_assert_trip = true;
      if( arg == "--show-test-names" )
         std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
   }

   appbase::app().register_plugin< blurt::plugins::account_history::account_history_plugin >();
   db_plugin = &appbase::app().register_plugin< blurt::plugins::debug_node::debug_node_plugin >();
   appbase::app().register_plugin< blurt::plugins::rc::rc_plugin >();
   appbase::app().register_plugin< blurt::plugins::witness::witness_plugin >();

   db_plugin->logging = false;
   appbase::app().initialize<
      blurt::plugins::account_history::account_history_plugin,
      blurt::plugins::debug_node::debug_node_plugin,
      blurt::plugins::rc::rc_plugin,
      blurt::plugins::witness::witness_plugin
      >( argc, argv );

   blurt::plugins::rc::rc_plugin_skip_flags rc_skip;
   rc_skip.skip_reject_not_enough_rc = 1;
   rc_skip.skip_deduct_rc = 0;
   rc_skip.skip_negative_rc_balance = 1;
   rc_skip.skip_reject_unknown_delta_vests = 0;
   appbase::app().get_plugin< blurt::plugins::rc::rc_plugin >().set_rc_plugin_skip_flags( rc_skip );

   db = &appbase::app().get_plugin< blurt::plugins::chain::chain_plugin >().db();
   BOOST_REQUIRE( db );

   init_account_pub_key = init_account_priv_key.get_public_key();

   open_database( shared_file_size_in_mb );

   generate_block();
   db->set_hardfork( BLURT_BLOCKCHAIN_VERSION.minor_v() );
   generate_block();

   vest( "initminer", 10000 );

   // Fill up the rest of the required miners
   for( int i = BLURT_NUM_INIT_MINERS; i < BLURT_MAX_WITNESSES; i++ )
   {
      account_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_pub_key );
      fund( BLURT_INIT_MINER_NAME + fc::to_string( i ), BLURT_MIN_PRODUCER_REWARD.amount.value );
      witness_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_priv_key, "foo.bar", init_account_pub_key, BLURT_MIN_PRODUCER_REWARD.amount );
   }

   validate_database();
   } catch ( const fc::exception& e )
   {
      edump( (e.to_detail_string()) );
      throw;
   }

   return;
}

clean_database_fixture::~clean_database_fixture()
{ try {
   // If we're unwinding due to an exception, don't do any more checks.
   // This way, boost test's last checkpoint tells us approximately where the error was.
   if( !std::uncaught_exception() )
   {
      BOOST_CHECK( db->get_node_properties().skip_flags == database::skip_nothing );
   }

   if( data_dir )
      db->wipe( data_dir->path(), data_dir->path(), true );
   return;
} FC_CAPTURE_AND_LOG( () )
   exit(1);
}

void clean_database_fixture::validate_database()
{
   database_fixture::validate_database();
   appbase::app().get_plugin< blurt::plugins::rc::rc_plugin >().validate_database();
}

void clean_database_fixture::resize_shared_mem( uint64_t size )
{
   db->wipe( data_dir->path(), data_dir->path(), true );
   int argc = boost::unit_test::framework::master_test_suite().argc;
   char** argv = boost::unit_test::framework::master_test_suite().argv;
   for( int i=1; i<argc; i++ )
   {
      const std::string arg = argv[i];
      if( arg == "--record-assert-trip" )
         fc::enable_record_assert_trip = true;
      if( arg == "--show-test-names" )
         std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
   }
   init_account_pub_key = init_account_priv_key.get_public_key();

   {
      database::open_args args;
      args.data_dir = data_dir->path();
      args.shared_mem_dir = args.data_dir;
      args.initial_supply = INITIAL_TEST_SUPPLY;
      args.shared_file_size = size;
      args.database_cfg = blurt::utilities::default_database_configuration();
      db->open( args );
   }

   boost::program_options::variables_map options;


   generate_block();
   db->set_hardfork( BLURT_BLOCKCHAIN_VERSION.minor_v() );
   generate_block();

   vest( "initminer", 10000 );

   // Fill up the rest of the required miners
   for( int i = BLURT_NUM_INIT_MINERS; i < BLURT_MAX_WITNESSES; i++ )
   {
      account_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_pub_key );
      fund( BLURT_INIT_MINER_NAME + fc::to_string( i ), BLURT_MIN_PRODUCER_REWARD.amount.value );
      witness_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_priv_key, "foo.bar", init_account_pub_key, BLURT_MIN_PRODUCER_REWARD.amount );
   }

   validate_database();
}

live_database_fixture::live_database_fixture()
{
   try
   {
      int argc = boost::unit_test::framework::master_test_suite().argc;
      char** argv = boost::unit_test::framework::master_test_suite().argv;

      ilog( "Loading saved chain" );
      _chain_dir = fc::current_path() / "test_blockchain";
      FC_ASSERT( fc::exists( _chain_dir ), "Requires blockchain to test on in ./test_blockchain" );

      appbase::app().register_plugin< blurt::plugins::account_history::account_history_plugin >();
      appbase::app().initialize<
         blurt::plugins::account_history::account_history_plugin
         >( argc, argv );

      db = &appbase::app().get_plugin< blurt::plugins::chain::chain_plugin >().db();
      BOOST_REQUIRE( db );

      {
         database::open_args args;
         args.data_dir = _chain_dir;
         args.shared_mem_dir = args.data_dir;
         args.database_cfg = blurt::utilities::default_database_configuration();
         db->open( args );
      }

      validate_database();
      generate_block();

      ilog( "Done loading saved chain" );
   }
   FC_LOG_AND_RETHROW()
}

live_database_fixture::~live_database_fixture()
{
   try
   {
      // If we're unwinding due to an exception, don't do any more checks.
      // This way, boost test's last checkpoint tells us approximately where the error was.
      if( !std::uncaught_exception() )
      {
         BOOST_CHECK( db->get_node_properties().skip_flags == database::skip_nothing );
      }

      db->pop_block();
      db->close();
      return;
   }
   FC_CAPTURE_AND_LOG( () )
   exit(1);
}

fc::ecc::private_key database_fixture::generate_private_key(string seed)
{
   static const fc::ecc::private_key committee = fc::ecc::private_key::regenerate( fc::sha256::hash( string( "init_key" ) ) );
   if( seed == "init_key" )
      return committee;
   return fc::ecc::private_key::regenerate( fc::sha256::hash( seed ) );
}

void database_fixture::open_database( uint16_t shared_file_size_in_mb )
{
   if( !data_dir )
   {
      data_dir = fc::temp_directory( blurt::utilities::temp_directory_path() );
      db->_log_hardforks = false;

      idump( (data_dir->path()) );

      database::open_args args;
      args.data_dir = data_dir->path();
      args.shared_mem_dir = args.data_dir;
      args.initial_supply = INITIAL_TEST_SUPPLY;
      args.shared_file_size = 1024 * 1024 * shared_file_size_in_mb; // 8MB(default) or more:  file for testing
      args.database_cfg = blurt::utilities::default_database_configuration();
      args.sps_remove_threshold = 20;
      db->open(args);
   }
   else
   {
      idump( (data_dir->path()) );
   }
}

void database_fixture::generate_block(uint32_t skip, const fc::ecc::private_key& key, int miss_blocks)
{
   skip |= default_skip;
   db_plugin->debug_generate_blocks( blurt::utilities::key_to_wif( key ), 1, skip, miss_blocks );
}

void database_fixture::generate_blocks( uint32_t block_count )
{
   auto produced = db_plugin->debug_generate_blocks( debug_key, block_count, default_skip, 0 );
   BOOST_REQUIRE( produced == block_count );
}

void database_fixture::generate_blocks(fc::time_point_sec timestamp, bool miss_intermediate_blocks)
{
   db_plugin->debug_generate_blocks_until( debug_key, timestamp, miss_intermediate_blocks, default_skip );
   BOOST_REQUIRE( ( db->head_block_time() - timestamp ).to_seconds() < BLURT_BLOCK_INTERVAL );
}

const account_object& database_fixture::account_create(
   const string& name,
   const string& creator,
   const private_key_type& creator_key,
   const share_type& fee,
   const public_key_type& key,
   const public_key_type& post_key,
   const string& json_metadata
   )
{
   try
   {
      auto actual_fee = std::min( fee, db->get_witness_schedule_object().median_props.account_creation_fee.amount );
      auto fee_remainder = fee - actual_fee;

      account_create_operation op;
      op.new_account_name = name;
      op.creator = creator;
      op.fee = asset( actual_fee, BLURT_SYMBOL );
      op.owner = authority( 1, key, 1 );
      op.active = authority( 1, key, 1 );
      op.posting = authority( 1, post_key, 1 );
      op.memo_key = key;
      op.json_metadata = json_metadata;

      trx.operations.push_back( op );

      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      sign( trx, creator_key );
      trx.validate();
      db->push_transaction( trx, 0 );
      trx.clear();

      if( fee_remainder > 0 )
      {
         vest( BLURT_INIT_MINER_NAME, name, asset( fee_remainder, BLURT_SYMBOL ) );
      }

      const account_object& acct = db->get_account( name );

      return acct;
   }
   FC_CAPTURE_AND_RETHROW( (name)(creator) )
}

const account_object& database_fixture::account_create(
   const string& name,
   const public_key_type& key,
   const public_key_type& post_key
)
{
   try
   {
      return account_create(
         name,
         BLURT_INIT_MINER_NAME,
         init_account_priv_key,
         std::max( db->get_witness_schedule_object().median_props.account_creation_fee.amount, share_type( 100 ) ),
         key,
         post_key,
         "" );
   }
   FC_CAPTURE_AND_RETHROW( (name) );
}

const account_object& database_fixture::account_create(
   const string& name,
   const public_key_type& key
)
{
   return account_create( name, key, key );
}

const witness_object& database_fixture::witness_create(
   const string& owner,
   const private_key_type& owner_key,
   const string& url,
   const public_key_type& signing_key,
   const share_type& fee )
{
   try
   {
      witness_update_operation op;
      op.owner = owner;
      op.url = url;
      op.block_signing_key = signing_key;
      op.fee = asset( fee, BLURT_SYMBOL );

      trx.operations.push_back( op );
      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      sign( trx, owner_key );
      trx.validate();
      db->push_transaction( trx, 0 );
      trx.clear();

      return db->get_witness( owner );
   }
   FC_CAPTURE_AND_RETHROW( (owner)(url) )
}

void database_fixture::fund(
   const string& account_name,
   const share_type& amount
   )
{
   try
   {
      transfer( BLURT_INIT_MINER_NAME, account_name, asset( amount, BLURT_SYMBOL ) );

   } FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::fund(
   const string& account_name,
   const asset& amount
   )
{
   try
   {
      db_plugin->debug_update( [=]( database& db)
      {
         if( amount.symbol.space() == asset_symbol_type::smt_nai_space )
         {
            db.adjust_balance(account_name, amount);
            db.adjust_supply(amount);
            // Note that SMT have no equivalent of SBD, hence no virtual supply, hence no need to update it.
            return;
         }

         db.modify( db.get_account( account_name ), [&]( account_object& a )
         {
            if( amount.symbol == BLURT_SYMBOL )
               a.balance += amount;
         });

         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            if( amount.symbol == BLURT_SYMBOL )
               gpo.current_supply += amount;
         });

      }, default_skip );
   }
   FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::transfer(
   const string& from,
   const string& to,
   const asset& amount )
{
   try
   {
      transfer_operation op;
      op.from = from;
      op.to = to;
      op.amount = amount;

      trx.operations.push_back( op );
      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      trx.validate();

      if( from == BLURT_INIT_MINER_NAME )
      {
         sign( trx, init_account_priv_key );
      }

      db->push_transaction( trx, ~0 );
      trx.clear();
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount) )
}

void database_fixture::vest( const string& from, const string& to, const asset& amount )
{
   try
   {
      FC_ASSERT( amount.symbol == BLURT_SYMBOL, "Can only vest TESTS" );

      transfer_to_vesting_operation op;
      op.from = from;
      op.to = to;
      op.amount = amount;

      trx.operations.push_back( op );
      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      trx.validate();

      // This sign() call fixes some tests, like withdraw_vesting_apply, that use this method
      //   with debug_plugin such that trx may be re-applied with less generous skip flags.
      if( from == BLURT_INIT_MINER_NAME )
      {
         sign( trx, init_account_priv_key );
      }

      db->push_transaction( trx, ~0 );
      trx.clear();
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount) )
}

void database_fixture::vest( const string& from, const share_type& amount )
{
   try
   {
      transfer_to_vesting_operation op;
      op.from = from;
      op.to = "";
      op.amount = asset( amount, BLURT_SYMBOL );

      trx.operations.push_back( op );
      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      trx.validate();

      if( from == BLURT_INIT_MINER_NAME )
      {
         sign( trx, init_account_priv_key );
      }

      db->push_transaction( trx, ~0 );
      trx.clear();
   } FC_CAPTURE_AND_RETHROW( (from)(amount) )
}

void database_fixture::proxy( const string& account, const string& proxy )
{
   try
   {
      account_witness_proxy_operation op;
      op.account = account;
      op.proxy = proxy;
      trx.operations.push_back( op );
      db->push_transaction( trx, ~0 );
      trx.clear();
   } FC_CAPTURE_AND_RETHROW( (account)(proxy) )
}

void database_fixture::set_witness_props( const flat_map< string, vector< char > >& props )
{
   trx.clear();
   for( size_t i=0; i<BLURT_MAX_WITNESSES; i++ )
   {
      witness_set_properties_operation op;
      op.owner = BLURT_INIT_MINER_NAME + (i == 0 ? "" : fc::to_string( i ));
      op.props = props;
      if( props.find( "key" ) == props.end() )
         op.props["key"] = fc::raw::pack_to_vector( init_account_pub_key );

      trx.operations.push_back( op );
      trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
      db->push_transaction( trx, ~0 );
      trx.clear();
   }

   const witness_schedule_object* wso = &(db->get_witness_schedule_object());
   uint32_t old_next_shuffle = wso->next_shuffle_block_num;

   for( size_t i=0; i<2*BLURT_MAX_WITNESSES+1; i++ )
   {
      generate_block();
      wso = &(db->get_witness_schedule_object());
      if( wso->next_shuffle_block_num != old_next_shuffle )
         return;
   }
   FC_ASSERT( false, "Couldn't apply properties in ${n} blocks", ("n", 2*BLURT_MAX_WITNESSES+1) );
}

const asset& database_fixture::get_balance( const string& account_name )const
{
  return db->get_account( account_name ).balance;
}

void database_fixture::sign(signed_transaction& trx, const fc::ecc::private_key& key)
{
   trx.sign( key, db->get_chain_id(), default_sig_canon );
}

vector< operation > database_fixture::get_last_operations( uint32_t num_ops )
{
   vector< operation > ops;
   const auto& acc_hist_idx = db->get_index< account_history_index >().indices().get< by_id >();
   auto itr = acc_hist_idx.end();

   while( itr != acc_hist_idx.begin() && ops.size() < num_ops )
   {
      itr--;
      const buffer_type& _serialized_op = db->get(itr->op).serialized_op;
      std::vector<char> serialized_op;
      serialized_op.reserve( _serialized_op.size() );
      std::copy( _serialized_op.begin(), _serialized_op.end(), std::back_inserter( serialized_op ) );
      ops.push_back( fc::raw::unpack_from_vector< blurt::chain::operation >( serialized_op ) );
   }

   return ops;
}

void database_fixture::validate_database()
{
   try
   {
      db->validate_invariants();
   }
   FC_LOG_AND_RETHROW();
}

void sps_proposal_database_fixture::plugin_prepare()
{
   int argc = boost::unit_test::framework::master_test_suite().argc;
   char** argv = boost::unit_test::framework::master_test_suite().argv;
   for( int i=1; i<argc; i++ )
   {
      const std::string arg = argv[i];
      if( arg == "--record-assert-trip" )
         fc::enable_record_assert_trip = true;
      if( arg == "--show-test-names" )
         std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
   }

   db_plugin = &appbase::app().register_plugin< blurt::plugins::debug_node::debug_node_plugin >();
   init_account_pub_key = init_account_priv_key.get_public_key();

   db_plugin->logging = false;
   appbase::app().initialize<
      blurt::plugins::debug_node::debug_node_plugin
   >( argc, argv );

   db = &appbase::app().get_plugin< blurt::plugins::chain::chain_plugin >().db();
   BOOST_REQUIRE( db );

   open_database();

   generate_block();
   db->set_hardfork( BLURT_NUM_HARDFORKS );
   generate_block();


   validate_database();
}

int64_t sps_proposal_database_fixture::create_proposal( std::string creator, std::string receiver,
                           time_point_sec start_date, time_point_sec end_date,
                           asset daily_pay, const fc::ecc::private_key& key )
{
   signed_transaction tx;
   create_proposal_operation op;

   op.creator = creator;
   op.receiver = receiver;

   op.start_date = start_date;
   op.end_date = end_date;

   op.daily_pay = daily_pay;

   static uint32_t cnt = 0;
   op.subject = std::to_string( cnt );

   const std::string permlink = "permlink" + std::to_string( cnt );
   post_comment(creator, permlink, "title", "body", "test", key);

   op.permlink = permlink;

   tx.operations.push_back( op );
   tx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
   sign( tx, key );
   db->push_transaction( tx, 0 );
   tx.signatures.clear();
   tx.operations.clear();

   const auto& proposal_idx = db-> template get_index< proposal_index >().indices(). template get< by_proposal_id >();
   auto itr = proposal_idx.end();
   BOOST_REQUIRE( proposal_idx.begin() != itr );
   --itr;
   BOOST_REQUIRE( creator == itr->creator );

   //An unique subject is generated by cnt
   ++cnt;

   return itr->proposal_id;
}

void sps_proposal_database_fixture::vote_proposal( std::string voter, const std::vector< int64_t >& id_proposals, bool approve, const fc::ecc::private_key& key )
{
   update_proposal_votes_operation op;

   op.voter = voter;
   op.proposal_ids.insert(id_proposals.cbegin(), id_proposals.cend());
   op.approve = approve;

   signed_transaction tx;
   tx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
   tx.operations.push_back( op );
   sign( tx, key );
   db->push_transaction( tx, 0 );
}

bool sps_proposal_database_fixture::exist_proposal( int64_t id )
{
   const auto& proposal_idx = db->get_index< proposal_index >().indices(). template get< by_proposal_id >();
   return proposal_idx.find( id ) != proposal_idx.end();
}

const proposal_object* sps_proposal_database_fixture::find_proposal( int64_t id )
{
   const auto& proposal_idx = db->get_index< proposal_index >().indices(). template get< by_proposal_id >();
   auto found = proposal_idx.find( id );

   if( found != proposal_idx.end() )
      return &(*found);
   else
      return nullptr;
}

void sps_proposal_database_fixture::remove_proposal(account_name_type _deleter, flat_set<int64_t> _proposal_id, const fc::ecc::private_key& _key)
{
   remove_proposal_operation rp;
   rp.proposal_owner = _deleter;
   rp.proposal_ids   = _proposal_id;

   signed_transaction trx;
   trx.operations.push_back( rp );
   trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
   sign( trx, _key );
   db->push_transaction( trx, 0 );
   trx.signatures.clear();
   trx.operations.clear();
}

bool sps_proposal_database_fixture::find_vote_for_proposal(const std::string& _user, int64_t _proposal_id)
{
      const auto& proposal_vote_idx = db->get_index< proposal_vote_index >().indices(). template get< by_voter_proposal >();
      auto found_vote = proposal_vote_idx.find( boost::make_tuple(_user, _proposal_id) );
      return found_vote != proposal_vote_idx.end() ;
}

uint64_t sps_proposal_database_fixture::get_nr_blocks_until_maintenance_block()
{
   auto block_time = db->head_block_time();

   auto next_maintenance_time = db->get_dynamic_global_properties().next_maintenance_time;
   auto ret = ( next_maintenance_time - block_time ).to_seconds() / BLURT_BLOCK_INTERVAL;

   FC_ASSERT( next_maintenance_time >= block_time );

   return ret;
}

void sps_proposal_database_fixture::post_comment( std::string _authro, std::string _permlink, std::string _title, std::string _body, std::string _parent_permlink, const fc::ecc::private_key& _key)
{
   generate_blocks( db->head_block_time() + BLURT_MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLURT_BLOCK_INTERVAL ), true );
   comment_operation comment;

   comment.author = _authro;
   comment.permlink = _permlink;
   comment.title = _title;
   comment.body = _body;
   comment.parent_permlink = _parent_permlink;

   signed_transaction trx;
   trx.operations.push_back( comment );
   trx.set_expiration( db->head_block_time() + BLURT_MAX_TIME_UNTIL_EXPIRATION );
   sign( trx, _key );
   db->push_transaction( trx, 0 );
   trx.signatures.clear();
   trx.operations.clear();
}

json_rpc_database_fixture::json_rpc_database_fixture()
{
   try {
   int argc = boost::unit_test::framework::master_test_suite().argc;
   char** argv = boost::unit_test::framework::master_test_suite().argv;
   for( int i=1; i<argc; i++ )
   {
      const std::string arg = argv[i];
      if( arg == "--record-assert-trip" )
         fc::enable_record_assert_trip = true;
      if( arg == "--show-test-names" )
         std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
   }

   appbase::app().register_plugin< blurt::plugins::account_history::account_history_plugin >();
   db_plugin = &appbase::app().register_plugin< blurt::plugins::debug_node::debug_node_plugin >();
   appbase::app().register_plugin< blurt::plugins::witness::witness_plugin >();
   rpc_plugin = &appbase::app().register_plugin< blurt::plugins::json_rpc::json_rpc_plugin >();
   appbase::app().register_plugin< blurt::plugins::block_api::block_api_plugin >();
   appbase::app().register_plugin< blurt::plugins::database_api::database_api_plugin >();
   appbase::app().register_plugin< blurt::plugins::condenser_api::condenser_api_plugin >();

   db_plugin->logging = false;
   appbase::app().initialize<
      blurt::plugins::account_history::account_history_plugin,
      blurt::plugins::debug_node::debug_node_plugin,
      blurt::plugins::json_rpc::json_rpc_plugin,
      blurt::plugins::block_api::block_api_plugin,
      blurt::plugins::database_api::database_api_plugin,
      blurt::plugins::condenser_api::condenser_api_plugin
      >( argc, argv );

   appbase::app().get_plugin< blurt::plugins::condenser_api::condenser_api_plugin >().plugin_startup();

   db = &appbase::app().get_plugin< blurt::plugins::chain::chain_plugin >().db();
   BOOST_REQUIRE( db );

   init_account_pub_key = init_account_priv_key.get_public_key();

   open_database();

   generate_block();
   db->set_hardfork( BLURT_BLOCKCHAIN_VERSION.minor_v() );
   generate_block();

   vest( "initminer", 10000 );

   // Fill up the rest of the required miners
   for( int i = BLURT_NUM_INIT_MINERS; i < BLURT_MAX_WITNESSES; i++ )
   {
      account_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_pub_key );
      fund( BLURT_INIT_MINER_NAME + fc::to_string( i ), BLURT_MIN_PRODUCER_REWARD.amount.value );
      witness_create( BLURT_INIT_MINER_NAME + fc::to_string( i ), init_account_priv_key, "foo.bar", init_account_pub_key, BLURT_MIN_PRODUCER_REWARD.amount );
   }

   validate_database();
   } catch ( const fc::exception& e )
   {
      edump( (e.to_detail_string()) );
      throw;
   }

   return;
}

json_rpc_database_fixture::~json_rpc_database_fixture()
{
   // If we're unwinding due to an exception, don't do any more checks.
   // This way, boost test's last checkpoint tells us approximately where the error was.
   if( !std::uncaught_exception() )
   {
      BOOST_CHECK( db->get_node_properties().skip_flags == database::skip_nothing );
   }

   if( data_dir )
      db->wipe( data_dir->path(), data_dir->path(), true );
   return;
}

fc::variant json_rpc_database_fixture::get_answer( std::string& request )
{
   return fc::json::from_string( rpc_plugin->call( request ) );
}

void check_id_equal( const fc::variant& id_a, const fc::variant& id_b )
{
   BOOST_REQUIRE( id_a.get_type() == id_b.get_type() );

   switch( id_a.get_type() )
   {
      case fc::variant::int64_type:
         BOOST_REQUIRE( id_a.as_int64() == id_b.as_int64() );
         break;
      case fc::variant::uint64_type:
         BOOST_REQUIRE( id_a.as_uint64() == id_b.as_uint64() );
         break;
      case fc::variant::string_type:
         BOOST_REQUIRE( id_a.as_string() == id_b.as_string() );
         break;
      case fc::variant::null_type:
         break;
      default:
         BOOST_REQUIRE( false );
   }
}

void json_rpc_database_fixture::review_answer( fc::variant& answer, int64_t code, bool is_warning, bool is_fail, fc::optional< fc::variant > id )
{
   fc::variant_object error;
   int64_t answer_code;

   if( is_fail )
   {
      if( id.valid() && code != JSON_RPC_INVALID_REQUEST )
      {
         BOOST_REQUIRE( answer.get_object().contains( "id" ) );
         check_id_equal( answer[ "id" ], *id );
      }

      BOOST_REQUIRE( answer.get_object().contains( "error" ) );
      BOOST_REQUIRE( answer["error"].is_object() );
      error = answer["error"].get_object();
      BOOST_REQUIRE( error.contains( "code" ) );
      BOOST_REQUIRE( error["code"].is_int64() );
      answer_code = error["code"].as_int64();
      BOOST_REQUIRE( answer_code == code );
      if( is_warning )
         BOOST_TEST_MESSAGE( error["message"].as_string() );
   }
   else
   {
      BOOST_REQUIRE( answer.get_object().contains( "result" ) );
      BOOST_REQUIRE( answer.get_object().contains( "id" ) );
      if( id.valid() )
         check_id_equal( answer[ "id" ], *id );
   }
}

void json_rpc_database_fixture::make_array_request( std::string& request, int64_t code, bool is_warning, bool is_fail )
{
   fc::variant answer = get_answer( request );
   BOOST_REQUIRE( answer.is_array() );

   fc::variants request_array = fc::json::from_string( request ).get_array();
   fc::variants array = answer.get_array();

   BOOST_REQUIRE( array.size() == request_array.size() );
   for( size_t i = 0; i < array.size(); ++i )
   {
      fc::optional< fc::variant > id;

      try
      {
         id = request_array[i][ "id" ];
      }
      catch( ... ) {}

      review_answer( array[i], code, is_warning, is_fail, id );
   }
}

fc::variant json_rpc_database_fixture::make_request( std::string& request, int64_t code, bool is_warning, bool is_fail )
{
   fc::variant answer = get_answer( request );
   BOOST_REQUIRE( answer.is_object() );
   fc::optional< fc::variant > id;

   try
   {
      id = fc::json::from_string( request ).get_object()[ "id" ];
   }
   catch( ... ) {}

   review_answer( answer, code, is_warning, is_fail, id );

   return answer;
}

void json_rpc_database_fixture::make_positive_request( std::string& request )
{
   make_request( request, 0/*code*/, false/*is_warning*/, false/*is_fail*/);
}

namespace test {

bool _push_block( database& db, const signed_block& b, uint32_t skip_flags /* = 0 */ )
{
   return db.push_block( b, skip_flags);
}

void _push_transaction( database& db, const signed_transaction& tx, uint32_t skip_flags /* = 0 */ )
{ try {
   db.push_transaction( tx, skip_flags );
} FC_CAPTURE_AND_RETHROW((tx)) }

} // blurt::chain::test

} } // blurt::chain
