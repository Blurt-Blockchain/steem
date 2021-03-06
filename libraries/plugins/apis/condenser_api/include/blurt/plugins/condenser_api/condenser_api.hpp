#pragma once
#include <blurt/chain/blurt_fwd.hpp>

#include <blurt/plugins/database_api/database_api.hpp>
#include <blurt/plugins/block_api/block_api.hpp>
#include <blurt/plugins/account_history_api/account_history_api.hpp>
#include <blurt/plugins/account_by_key_api/account_by_key_api.hpp>
#include <blurt/plugins/network_broadcast_api/network_broadcast_api.hpp>
#include <blurt/plugins/tags_api/tags_api.hpp>
#include <blurt/plugins/follow_api/follow_api.hpp>
#include <blurt/plugins/condenser_api/condenser_api_legacy_objects.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>
#include <fc/api.hpp>

namespace blurt { namespace plugins { namespace condenser_api {

using std::vector;
using fc::variant;
using fc::optional;

using namespace chain;

namespace detail{ class condenser_api_impl; }

struct discussion_index
{
   string           category;         /// category by which everything is filtered
   vector< string > trending;         /// trending posts over the last 24 hours
   vector< string > payout;           /// pending posts by payout
   vector< string > payout_comments;  /// pending comments by payout
   vector< string > trending30;       /// pending lifetime payout
   vector< string > created;          /// creation date
   vector< string > responses;        /// creation date
   vector< string > updated;          /// creation date
   vector< string > active;           /// last update or reply
   vector< string > votes;            /// last update or reply
   vector< string > cashout;          /// last update or reply
   vector< string > maturing;         /// about to be paid out
   vector< string > best;             /// total lifetime payout
   vector< string > hot;              /// total lifetime payout
   vector< string > promoted;         /// pending lifetime payout
};

struct api_operation_object
{
   api_operation_object() {}
   api_operation_object( const account_history::api_operation_object& obj, const legacy_operation& l_op ) :
      trx_id( obj.trx_id ),
      block( obj.block ),
      trx_in_block( obj.trx_in_block ),
      virtual_op( obj.virtual_op ),
      timestamp( obj.timestamp ),
      op( l_op )
   {}

   transaction_id_type  trx_id;
   uint32_t             block = 0;
   uint32_t             trx_in_block = 0;
   uint32_t             op_in_trx = 0;
   uint32_t             virtual_op = 0;
   fc::time_point_sec   timestamp;
   legacy_operation     op;
};

struct api_account_object
{
   api_account_object( const database_api::api_account_object& a ) :
      id( a.id ),
      name( a.name ),
      owner( a.owner ),
      active( a.active ),
      posting( a.posting ),
      memo_key( a.memo_key ),
      json_metadata( a.json_metadata ),
      posting_json_metadata( a.posting_json_metadata ),
      proxy( a.proxy ),
      last_owner_update( a.last_owner_update ),
      last_account_update( a.last_account_update ),
      created( a.created ),
      mined( a.mined ),
      recovery_account( a.recovery_account ),
      reset_account( a.reset_account ),
      last_account_recovery( a.last_account_recovery ),
      comment_count( a.comment_count ),
      lifetime_vote_count( a.lifetime_vote_count ),
      post_count( a.post_count ),
      can_vote( a.can_vote ),
      voting_manabar( a.voting_manabar ),
      balance( legacy_asset::from_asset( a.balance ) ),
      savings_balance( legacy_asset::from_asset( a.savings_balance ) ),
      savings_withdraw_requests( a.savings_withdraw_requests ),
      reward_blurt_balance( legacy_asset::from_asset( a.reward_blurt_balance ) ),
      reward_vesting_balance( legacy_asset::from_asset( a.reward_vesting_balance ) ),
      reward_vesting_blurt( legacy_asset::from_asset( a.reward_vesting_blurt ) ),
      curation_rewards( a.curation_rewards ),
      posting_rewards( a.posting_rewards ),
      vesting_shares( legacy_asset::from_asset( a.vesting_shares ) ),
      delegated_vesting_shares( legacy_asset::from_asset( a.delegated_vesting_shares ) ),
      received_vesting_shares( legacy_asset::from_asset( a.received_vesting_shares ) ),
      vesting_withdraw_rate( legacy_asset::from_asset( a.vesting_withdraw_rate ) ),
      next_vesting_withdrawal( a.next_vesting_withdrawal ),
      withdrawn( a.withdrawn ),
      to_withdraw( a.to_withdraw ),
      withdraw_routes( a.withdraw_routes ),
      witnesses_voted_for( a.witnesses_voted_for ),
      last_post( a.last_post ),
      last_root_post( a.last_root_post ),
      last_vote_time( a.last_vote_time ),
      post_bandwidth( a.post_bandwidth ),
      pending_claimed_accounts( a.pending_claimed_accounts )
   {
      voting_power = _compute_voting_power(a);
      proxied_vsf_votes.insert( proxied_vsf_votes.end(), a.proxied_vsf_votes.begin(), a.proxied_vsf_votes.end() );
   }

   api_account_object(){}

   uint16_t _compute_voting_power( const database_api::api_account_object& a );

   account_id_type   id;

   account_name_type name;
   authority         owner;
   authority         active;
   authority         posting;
   public_key_type   memo_key;
   string            json_metadata;
   string            posting_json_metadata;
   account_name_type proxy;

   time_point_sec    last_owner_update;
   time_point_sec    last_account_update;

   time_point_sec    created;
   bool              mined = false;
   account_name_type recovery_account;
   account_name_type reset_account;
   time_point_sec    last_account_recovery;
   uint32_t          comment_count = 0;
   uint32_t          lifetime_vote_count = 0;
   uint32_t          post_count = 0;

   bool              can_vote = false;
   util::manabar     voting_manabar;
   uint16_t          voting_power = 0;

   legacy_asset      balance;
   legacy_asset      savings_balance;

   uint8_t           savings_withdraw_requests = 0;

   legacy_asset      reward_blurt_balance;
   legacy_asset      reward_vesting_balance;
   legacy_asset      reward_vesting_blurt;

   share_type        curation_rewards;
   share_type        posting_rewards;

   legacy_asset      vesting_shares;
   legacy_asset      delegated_vesting_shares;
   legacy_asset      received_vesting_shares;
   legacy_asset      vesting_withdraw_rate;
   time_point_sec    next_vesting_withdrawal;
   share_type        withdrawn;
   share_type        to_withdraw;
   uint16_t          withdraw_routes = 0;

   vector< share_type > proxied_vsf_votes;

   uint16_t          witnesses_voted_for = 0;

   time_point_sec    last_post;
   time_point_sec    last_root_post;
   time_point_sec    last_vote_time;
   uint32_t          post_bandwidth = 0;

   share_type        pending_claimed_accounts = 0;
};

struct extended_account : public api_account_object
{
   extended_account(){}
   extended_account( const database_api::api_account_object& a ) :
      api_account_object( a ) {}

   legacy_asset                                             vesting_balance;  /// convert vesting_shares to vesting blurt
   map< uint64_t, api_operation_object >   transfer_history; /// transfer to/from vesting
   map< uint64_t, api_operation_object >   market_history;   /// limit order / cancel / fill
   map< uint64_t, api_operation_object >   post_history;
   map< uint64_t, api_operation_object >   vote_history;
   map< uint64_t, api_operation_object >   other_history;
   set< string >                                            witness_votes;
   vector< tags::tag_count_object >                         tags_usage;
   vector< follow::reblog_count >                           guest_bloggers;

   optional< vector< string > >                             comments;         /// permlinks for this user
   optional< vector< string > >                             blog;             /// blog posts for this user
   optional< vector< string > >                             feed;             /// feed posts for this user
   optional< vector< string > >                             recent_replies;   /// blog posts for this user
   optional< vector< string > >                             recommended;      /// posts recommened for this user
};

struct api_comment_object
{
   api_comment_object( const database_api::api_comment_object& c ):
      id( c.id ),
      category( c.category ),
      parent_author( c.parent_author ),
      parent_permlink( c.parent_permlink ),
      author( c.author ),
      permlink( c.permlink ),
      title( c.title ),
      body( c.body ),
      json_metadata( c.json_metadata ),
      last_update( c.last_update ),
      created( c.created ),
      active( c.active ),
      last_payout( c.last_payout ),
      depth( c.depth ),
      children( c.children ),
      net_rshares( c.net_rshares ),
      abs_rshares( c.abs_rshares ),
      vote_rshares( c.vote_rshares ),
      children_abs_rshares( c.children_abs_rshares ),
      cashout_time( c.cashout_time ),
      max_cashout_time( c.max_cashout_time ),
      total_vote_weight( c.total_vote_weight ),
      reward_weight( c.reward_weight ),
      total_payout_value( legacy_asset::from_asset( c.total_payout_value ) ),
      curator_payout_value( legacy_asset::from_asset( c.curator_payout_value ) ),
      author_rewards( c.author_rewards ),
      net_votes( c.net_votes ),
      root_author( c.root_author ),
      root_permlink( c.root_permlink ),
      max_accepted_payout( legacy_asset::from_asset( c.max_accepted_payout ) ),
      percent_blurt( c.percent_blurt ),
      allow_replies( c.allow_replies ),
      allow_votes( c.allow_votes ),
      allow_curation_rewards( c.allow_curation_rewards )
   {
      for( auto& route : c.beneficiaries )
      {
         beneficiaries.push_back( route );
      }
   }

   api_comment_object(){}

   comment_id_type   id;
   string            category;
   string            parent_author;
   string            parent_permlink;
   string            author;
   string            permlink;

   string            title;
   string            body;
   string            json_metadata;
   time_point_sec    last_update;
   time_point_sec    created;
   time_point_sec    active;
   time_point_sec    last_payout;

   uint8_t           depth = 0;
   uint32_t          children = 0;

   share_type        net_rshares;
   share_type        abs_rshares;
   share_type        vote_rshares;

   share_type        children_abs_rshares;
   time_point_sec    cashout_time;
   time_point_sec    max_cashout_time;
   uint64_t          total_vote_weight = 0;

   uint16_t          reward_weight = 0;

   legacy_asset      total_payout_value;
   legacy_asset      curator_payout_value;

   share_type        author_rewards;

   int32_t           net_votes = 0;

   account_name_type root_author;
   string            root_permlink;

   legacy_asset      max_accepted_payout;
   uint16_t          percent_blurt;
   bool              allow_replies = false;
   bool              allow_votes = false;
   bool              allow_curation_rewards = false;
   vector< beneficiary_route_type > beneficiaries;
};

struct extended_dynamic_global_properties
{
   extended_dynamic_global_properties() {}
   extended_dynamic_global_properties( const database_api::api_dynamic_global_property_object& o ) :
      head_block_number( o.head_block_number ),
      head_block_id( o.head_block_id ),
      time( o.time ),
      current_witness( o.current_witness ),
      current_supply( legacy_asset::from_asset( o.current_supply ) ),
      total_vesting_fund_blurt( legacy_asset::from_asset( o.total_vesting_fund_blurt ) ),
      total_vesting_shares( legacy_asset::from_asset( o.total_vesting_shares ) ),
      total_reward_fund_blurt( legacy_asset::from_asset( o.total_reward_fund_blurt ) ),
      total_reward_shares2( o.total_reward_shares2 ),
      pending_rewarded_vesting_shares( legacy_asset::from_asset( o.pending_rewarded_vesting_shares ) ),
      pending_rewarded_vesting_blurt( legacy_asset::from_asset( o.pending_rewarded_vesting_blurt ) ),
      maximum_block_size( o.maximum_block_size ),
      current_aslot( o.current_aslot ),
      recent_slots_filled( o.recent_slots_filled ),
      participation_count( o.participation_count ),
      last_irreversible_block_num( o.last_irreversible_block_num ),
      vote_power_reserve_rate( o.vote_power_reserve_rate ),
      delegation_return_period( o.delegation_return_period ),
      reverse_auction_seconds( o.reverse_auction_seconds ),
      available_account_subsidies( o.available_account_subsidies ),
      next_maintenance_time( o.next_maintenance_time ),
      last_budget_time( o.last_budget_time ),
      content_reward_percent( o.content_reward_percent ),
      vesting_reward_percent( o.vesting_reward_percent ),
      sps_fund_percent( o.sps_fund_percent ),
      sps_interval_ledger( legacy_asset::from_asset( o.sps_interval_ledger ) )
   {}

   uint32_t          head_block_number = 0;
   block_id_type     head_block_id;
   time_point_sec    time;
   account_name_type current_witness;

   legacy_asset      current_supply;
   legacy_asset      total_vesting_fund_blurt;
   legacy_asset      total_vesting_shares;
   legacy_asset      total_reward_fund_blurt;
   fc::uint128       total_reward_shares2;
   legacy_asset      pending_rewarded_vesting_shares;
   legacy_asset      pending_rewarded_vesting_blurt;

   uint32_t          maximum_block_size = 0;
   uint64_t          current_aslot = 0;
   fc::uint128_t     recent_slots_filled;
   uint8_t           participation_count = 0;

   uint32_t          last_irreversible_block_num = 0;

   uint32_t          vote_power_reserve_rate = BLURT_REDUCED_VOTE_POWER_RATE;
   uint32_t          delegation_return_period = BLURT_DELEGATION_RETURN_PERIOD_HF20;

   uint64_t          reverse_auction_seconds = 0;

   int64_t           available_account_subsidies = 0;

   time_point_sec    next_maintenance_time;
   time_point_sec    last_budget_time;

   uint16_t          content_reward_percent = BLURT_CONTENT_REWARD_PERCENT_HF21;
   uint16_t          vesting_reward_percent = BLURT_VESTING_FUND_PERCENT_HF16;
   uint16_t          sps_fund_percent = BLURT_PROPOSAL_FUND_PERCENT_HF21;

   legacy_asset      sps_interval_ledger;
};

struct api_witness_object
{
   api_witness_object() {}
   api_witness_object( const database_api::api_witness_object& w ) :
      id( w.id ),
      owner( w.owner ),
      created( w.created ),
      url( w.url ),
      total_missed( w.total_missed ),
      last_aslot( w.last_aslot ),
      last_confirmed_block_num( w.last_confirmed_block_num ),
      signing_key( w.signing_key ),
      props( w.props ),
      votes( w.votes ),
      virtual_last_update( w.virtual_last_update ),
      virtual_position( w.virtual_position ),
      virtual_scheduled_time( w.virtual_scheduled_time ),
      running_version( w.running_version ),
      hardfork_version_vote( w.hardfork_version_vote ),
      hardfork_time_vote( w.hardfork_time_vote ),
      available_witness_account_subsidies( w.available_witness_account_subsidies )
   {}

   witness_id_type  id;
   account_name_type       owner;
   time_point_sec          created;
   string                  url;
   uint32_t                total_missed = 0;
   uint64_t                last_aslot = 0;
   uint64_t                last_confirmed_block_num = 0;
   public_key_type         signing_key;
   api_chain_properties    props;
   share_type              votes;
   fc::uint128_t           virtual_last_update;
   fc::uint128_t           virtual_position;
   fc::uint128_t           virtual_scheduled_time = fc::uint128_t::max_value();
   version                 running_version;
   hardfork_version        hardfork_version_vote;
   time_point_sec          hardfork_time_vote = BLURT_GENESIS_TIME;
   int64_t                 available_witness_account_subsidies = 0;
};

struct api_witness_schedule_object
{
   api_witness_schedule_object() {}
   api_witness_schedule_object( const database_api::api_witness_schedule_object& w ) :
      id( w.id ),
      current_virtual_time( w.current_virtual_time ),
      next_shuffle_block_num( w.next_shuffle_block_num ),
      num_scheduled_witnesses( w.num_scheduled_witnesses ),
      elected_weight( w.elected_weight ),
      timeshare_weight( w.timeshare_weight ),
      witness_pay_normalization_factor( w.witness_pay_normalization_factor ),
      median_props( w.median_props ),
      majority_version( w.majority_version ),
      max_voted_witnesses( w.max_voted_witnesses ),
      max_runner_witnesses( w.max_runner_witnesses ),
      hardfork_required_witnesses( w.hardfork_required_witnesses ),
      account_subsidy_rd( w.account_subsidy_rd ),
      account_subsidy_witness_rd( w.account_subsidy_witness_rd ),
      min_witness_account_subsidy_decay( w.min_witness_account_subsidy_decay )
   {
      current_shuffled_witnesses.insert( current_shuffled_witnesses.begin(), w.current_shuffled_witnesses.begin(), w.current_shuffled_witnesses.end() );
   }

   witness_schedule_id_type      id;
   fc::uint128_t                 current_virtual_time;
   uint32_t                      next_shuffle_block_num = 1;
   vector< account_name_type >   current_shuffled_witnesses;
   uint8_t                       num_scheduled_witnesses = 1;
   uint8_t                       elected_weight = 1;
   uint8_t                       timeshare_weight = 5;
   uint32_t                      witness_pay_normalization_factor = 25;
   api_chain_properties          median_props;
   version                       majority_version;
   uint8_t                       max_voted_witnesses           = BLURT_MAX_VOTED_WITNESSES_HF17;
   uint8_t                       max_runner_witnesses          = BLURT_MAX_RUNNER_WITNESSES_HF17;
   uint8_t                       hardfork_required_witnesses   = BLURT_HARDFORK_REQUIRED_WITNESSES;

   rd_dynamics_params            account_subsidy_rd;
   rd_dynamics_params            account_subsidy_witness_rd;
   int64_t                       min_witness_account_subsidy_decay = 0;
};

struct api_reward_fund_object
{
   api_reward_fund_object() {}
   api_reward_fund_object( const database_api::api_reward_fund_object& r ) :
      id( r.id ),
      name( r.name ),
      reward_balance( legacy_asset::from_asset( r.reward_balance ) ),
      recent_claims( r.recent_claims ),
      last_update( r.last_update ),
      content_constant( r.content_constant ),
      percent_curation_rewards( r.percent_curation_rewards ),
      percent_content_rewards( r.percent_content_rewards ),
      author_reward_curve( r.author_reward_curve ),
      curation_reward_curve( r.curation_reward_curve )
   {}

   reward_fund_id_type     id;
   reward_fund_name_type   name;
   legacy_asset            reward_balance;
   fc::uint128_t           recent_claims = 0;
   time_point_sec          last_update;
   uint128_t               content_constant = 0;
   uint16_t                percent_curation_rewards = 0;
   uint16_t                percent_content_rewards = 0;
   protocol::curve_id      author_reward_curve = protocol::linear;
   protocol::curve_id      curation_reward_curve = protocol::square_root;
};

struct api_escrow_object
{
   api_escrow_object() {}
   api_escrow_object( const database_api::api_escrow_object& e ) :
      id( e.id ),
      escrow_id( e.escrow_id ),
      from( e.from ),
      to( e.to ),
      agent( e.agent ),
      ratification_deadline( e.ratification_deadline ),
      escrow_expiration( e.escrow_expiration ),
      blurt_balance( legacy_asset::from_asset( e.blurt_balance ) ),
      pending_fee( legacy_asset::from_asset( e.pending_fee ) ),
      to_approved( e.to_approved ),
      disputed( e.disputed ),
      agent_approved( e.agent_approved )
   {}

   escrow_id_type    id;
   uint32_t          escrow_id = 20;
   account_name_type from;
   account_name_type to;
   account_name_type agent;
   time_point_sec    ratification_deadline;
   time_point_sec    escrow_expiration;
   legacy_asset      blurt_balance;
   legacy_asset      pending_fee;
   bool              to_approved = false;
   bool              disputed = false;
   bool              agent_approved = false;
};

struct api_savings_withdraw_object
{
   api_savings_withdraw_object() {}
   api_savings_withdraw_object( const database_api::api_savings_withdraw_object& s ) :
      id( s.id ),
      from( s.from ),
      to( s.to ),
      memo( s.memo ),
      request_id( s.request_id ),
      amount( legacy_asset::from_asset( s.amount ) ),
      complete( s.complete )
   {}

   savings_withdraw_id_type id;

   account_name_type from;
   account_name_type to;
   string            memo;
   uint32_t          request_id = 0;
   legacy_asset      amount;
   time_point_sec    complete;
};

struct api_vesting_delegation_object
{
   api_vesting_delegation_object() {}
   api_vesting_delegation_object( const database_api::api_vesting_delegation_object& v ) :
      id( v.id ),
      delegator( v.delegator ),
      delegatee( v.delegatee ),
      vesting_shares( legacy_asset::from_asset( v.vesting_shares ) ),
      min_delegation_time( v.min_delegation_time )
   {}

   vesting_delegation_id_type id;
   account_name_type delegator;
   account_name_type delegatee;
   legacy_asset      vesting_shares;
   time_point_sec    min_delegation_time;
};

struct api_vesting_delegation_expiration_object
{
   api_vesting_delegation_expiration_object() {}
   api_vesting_delegation_expiration_object( const database_api::api_vesting_delegation_expiration_object& v ) :
      id( v.id ),
      delegator( v.delegator ),
      vesting_shares( legacy_asset::from_asset( v.vesting_shares ) ),
      expiration( v.expiration )
   {}

   vesting_delegation_expiration_id_type id;
   account_name_type delegator;
   legacy_asset      vesting_shares;
   time_point_sec    expiration;
};

struct api_proposal_object
{
   api_proposal_object() {}
   api_proposal_object( const database_api::api_proposal_object& p ) :
      id( p.id ),
      proposal_id( p.proposal_id ),
      creator( p.creator ),
      receiver( p.receiver ),
      start_date( p.start_date ),
      end_date( p.end_date ),
      daily_pay( legacy_asset::from_asset( p.daily_pay ) ),
      subject( p.subject ),
      permlink( p.permlink ),
      total_votes( p.total_votes )
   {}

   proposal_id_type  id;
   proposal_id_type  proposal_id;
   account_name_type creator;
   account_name_type receiver;
   time_point_sec    start_date;
   time_point_sec    end_date;
   legacy_asset      daily_pay;
   string            subject;
   string            permlink;
   uint64_t          total_votes = 0;
};

struct discussion : public api_comment_object
{
   discussion() {}

   discussion( const api_comment_object& c ) : api_comment_object( c ) {}

   discussion( const tags::discussion& d ) :
      api_comment_object( d ),
      url( d.url ),
      root_title( d.root_title ),
      pending_payout_value( legacy_asset::from_asset( d.pending_payout_value ) ),
      total_pending_payout_value( legacy_asset::from_asset( d.total_pending_payout_value ) ),
      active_votes( d.active_votes ),
      replies( d.replies ),
      promoted( legacy_asset::from_asset( d.promoted ) ),
      body_length( d.body_length ),
      reblogged_by( d.reblogged_by ),
      first_reblogged_by( d.first_reblogged_by ),
      first_reblogged_on( d.first_reblogged_on )
   {}

   string                        url; /// /category/@rootauthor/root_permlink#author/permlink
   string                        root_title;
   legacy_asset                  pending_payout_value; ///< BLURT_SYMBOL
   legacy_asset                  total_pending_payout_value; ///< BLURT_SYMBOL including replies
   vector< tags::vote_state >    active_votes;
   vector< string >              replies; ///< author/slug mapping
   legacy_asset                  promoted;
   uint32_t                      body_length = 0;
   vector< account_name_type >   reblogged_by;
   optional< account_name_type > first_reblogged_by;
   optional< time_point_sec >    first_reblogged_on;
};

struct tag_index
{
   vector< tags::tag_name_type > trending; /// pending payouts
};

struct api_tag_object
{
   api_tag_object( const tags::api_tag_object& o ) :
      name( o.name ),
      total_payouts( legacy_asset::from_asset( o.total_payouts ) ),
      net_votes( o.net_votes ),
      top_posts( o.top_posts ),
      comments( o.comments ),
      trending( o.trending ) {}

   api_tag_object() {}

   string               name;
   legacy_asset         total_payouts;
   int32_t              net_votes = 0;
   uint32_t             top_posts = 0;
   uint32_t             comments = 0;
   fc::uint128          trending = 0;
};

struct state
{
   string                                             current_route;

   extended_dynamic_global_properties                 props;

   tag_index                                          tag_idx;

   /**
    * "" is the global tags::discussion index
    */
   map< string, discussion_index >                    discussion_idx;

   map< string, api_tag_object >                tags;

   /**
    *  map from account/slug to full nested tags::discussion
    */
   map< string, discussion >                          content;
   map< string, extended_account >                    accounts;

   map< string, api_witness_object >                  witnesses;
   api_witness_schedule_object                        witness_schedule;
   string                                             error;
};

struct scheduled_hardfork
{
   hardfork_version     hf_version;
   fc::time_point_sec   live_time;
};

struct account_vote
{
   string         authorperm;
   uint64_t       weight = 0;
   int64_t        rshares = 0;
   int16_t        percent = 0;
   time_point_sec time;
};

enum withdraw_route_type
{
   incoming,
   outgoing,
   all
};

typedef vector< variant > get_version_args;

struct get_version_return
{
   get_version_return() {}
   get_version_return( fc::string bc_v, fc::string s_v, fc::string fc_v )
      :blockchain_version( bc_v ), blurt_revision( s_v ), fc_revision( fc_v ) {}

   fc::string blockchain_version;
   fc::string blurt_revision;
   fc::string fc_revision;
};

typedef map< uint32_t, api_operation_object > get_account_history_return_type;

typedef vector< variant > broadcast_transaction_synchronous_args;

struct broadcast_transaction_synchronous_return
{
   broadcast_transaction_synchronous_return() {}
   broadcast_transaction_synchronous_return( transaction_id_type txid, int32_t bn, int32_t tn, bool ex )
   : id(txid), block_num(bn), trx_num(tn), expired(ex) {}

   transaction_id_type   id;
   int32_t               block_num = 0;
   int32_t               trx_num   = 0;
   bool                  expired   = false;
};

struct comment_feed_entry
{
   comment_feed_entry( const follow::comment_feed_entry& c ) :
      comment( c.comment ),
      reblog_on( c.reblog_on ),
      entry_id( c.entry_id )
   {
      reblog_by.resize( c.reblog_by.size() );

      for( auto& a : c.reblog_by )
      {
         reblog_by.push_back( a );
      }
   }

   comment_feed_entry() {}

   api_comment_object            comment;
   vector< account_name_type >   reblog_by;
   time_point_sec                reblog_on;
   uint32_t                      entry_id = 0;
};

struct comment_blog_entry
{
   comment_blog_entry( const follow::comment_blog_entry& c ) :
      comment( c.comment ),
      blog( c.blog ),
      reblog_on( c.reblog_on ),
      entry_id( c.entry_id )
   {}

   comment_blog_entry() {}

   api_comment_object   comment;
   string               blog;
   time_point_sec       reblog_on;
   uint32_t             entry_id = 0;
};

#define DEFINE_API_ARGS( api_name, arg_type, return_type )  \
typedef arg_type api_name ## _args;                         \
typedef return_type api_name ## _return;

/*               API,                                    args,                return */
DEFINE_API_ARGS( get_trending_tags,                      vector< variant >,   vector< api_tag_object > )
DEFINE_API_ARGS( get_state,                              vector< variant >,   state )
DEFINE_API_ARGS( get_active_witnesses,                   vector< variant >,   vector< account_name_type > )
DEFINE_API_ARGS( get_block_header,                       vector< variant >,   optional< block_header > )
DEFINE_API_ARGS( get_block,                              vector< variant >,   optional< legacy_signed_block > )
DEFINE_API_ARGS( get_ops_in_block,                       vector< variant >,   vector< api_operation_object > )
DEFINE_API_ARGS( get_config,                             vector< variant >,   fc::variant_object )
DEFINE_API_ARGS( get_dynamic_global_properties,          vector< variant >,   extended_dynamic_global_properties )
DEFINE_API_ARGS( get_chain_properties,                   vector< variant >,   api_chain_properties )
DEFINE_API_ARGS( get_witness_schedule,                   vector< variant >,   api_witness_schedule_object )
DEFINE_API_ARGS( get_hardfork_version,                   vector< variant >,   hardfork_version )
DEFINE_API_ARGS( get_next_scheduled_hardfork,            vector< variant >,   scheduled_hardfork )
DEFINE_API_ARGS( get_reward_fund,                        vector< variant >,   api_reward_fund_object )
DEFINE_API_ARGS( get_key_references,                     vector< variant >,   vector< vector< account_name_type > > )
DEFINE_API_ARGS( get_accounts,                           vector< variant >,   vector< extended_account > )
DEFINE_API_ARGS( get_account_references,                 vector< variant >,   vector< account_id_type > )
DEFINE_API_ARGS( lookup_account_names,                   vector< variant >,   vector< optional< api_account_object > > )
DEFINE_API_ARGS( lookup_accounts,                        vector< variant >,   set< string > )
DEFINE_API_ARGS( get_account_count,                      vector< variant >,   uint64_t )
DEFINE_API_ARGS( get_owner_history,                      vector< variant >,   vector< database_api::api_owner_authority_history_object > )
DEFINE_API_ARGS( get_recovery_request,                   vector< variant >,   optional< database_api::api_account_recovery_request_object > )
DEFINE_API_ARGS( get_escrow,                             vector< variant >,   optional< api_escrow_object > )
DEFINE_API_ARGS( get_withdraw_routes,                    vector< variant >,   vector< database_api::api_withdraw_vesting_route_object > )
DEFINE_API_ARGS( get_savings_withdraw_from,              vector< variant >,   vector< api_savings_withdraw_object > )
DEFINE_API_ARGS( get_savings_withdraw_to,                vector< variant >,   vector< api_savings_withdraw_object > )
DEFINE_API_ARGS( get_vesting_delegations,                vector< variant >,   vector< api_vesting_delegation_object > )
DEFINE_API_ARGS( get_expiring_vesting_delegations,       vector< variant >,   vector< api_vesting_delegation_expiration_object > )
DEFINE_API_ARGS( get_witnesses,                          vector< variant >,   vector< optional< api_witness_object > > )
DEFINE_API_ARGS( get_witness_by_account,                 vector< variant >,   optional< api_witness_object > )
DEFINE_API_ARGS( get_witnesses_by_vote,                  vector< variant >,   vector< api_witness_object > )
DEFINE_API_ARGS( lookup_witness_accounts,                vector< variant >,   vector< account_name_type > )
DEFINE_API_ARGS( get_witness_count,                      vector< variant >,   uint64_t )
DEFINE_API_ARGS( get_transaction_hex,                    vector< variant >,   string )
DEFINE_API_ARGS( get_transaction,                        vector< variant >,   legacy_signed_transaction )
DEFINE_API_ARGS( get_required_signatures,                vector< variant >,   set< public_key_type > )
DEFINE_API_ARGS( get_potential_signatures,               vector< variant >,   set< public_key_type > )
DEFINE_API_ARGS( verify_authority,                       vector< variant >,   bool )
DEFINE_API_ARGS( verify_account_authority,               vector< variant >,   bool )
DEFINE_API_ARGS( get_active_votes,                       vector< variant >,   vector< tags::vote_state > )
DEFINE_API_ARGS( get_account_votes,                      vector< variant >,   vector< account_vote > )
DEFINE_API_ARGS( get_content,                            vector< variant >,   discussion )
DEFINE_API_ARGS( get_content_replies,                    vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_tags_used_by_author,                vector< variant >,   vector< tags::tag_count_object > )
DEFINE_API_ARGS( get_post_discussions_by_payout,         vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_comment_discussions_by_payout,      vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_trending,            vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_created,             vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_active,              vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_cashout,             vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_votes,               vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_children,            vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_hot,                 vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_feed,                vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_blog,                vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_comments,            vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_promoted,            vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_replies_by_last_update,             vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_discussions_by_author_before_date,  vector< variant >,   vector< discussion > )
DEFINE_API_ARGS( get_account_history,                    vector< variant >,   get_account_history_return_type )
DEFINE_API_ARGS( broadcast_transaction,                  vector< variant >,   json_rpc::void_type )
DEFINE_API_ARGS( broadcast_block,                        vector< variant >,   json_rpc::void_type )
DEFINE_API_ARGS( get_followers,                          vector< variant >,   vector< follow::api_follow_object > )
DEFINE_API_ARGS( get_following,                          vector< variant >,   vector< follow::api_follow_object > )
DEFINE_API_ARGS( get_follow_count,                       vector< variant >,   follow::get_follow_count_return )
DEFINE_API_ARGS( get_feed_entries,                       vector< variant >,   vector< follow::feed_entry > )
DEFINE_API_ARGS( get_feed,                               vector< variant >,   vector< comment_feed_entry > )
DEFINE_API_ARGS( get_blog_entries,                       vector< variant >,   vector< follow::blog_entry > )
DEFINE_API_ARGS( get_blog,                               vector< variant >,   vector< comment_blog_entry > )
DEFINE_API_ARGS( get_reblogged_by,                       vector< variant >,   vector< account_name_type > )
DEFINE_API_ARGS( get_blog_authors,                       vector< variant >,   vector< follow::reblog_count > )
DEFINE_API_ARGS( list_proposals,                         vector< variant >,   vector< api_proposal_object > )
DEFINE_API_ARGS( find_proposals,                         vector< variant >,   vector< api_proposal_object > )
DEFINE_API_ARGS( list_proposal_votes,                    vector< variant >,   vector< database_api::api_proposal_vote_object > )

#undef DEFINE_API_ARGS

class condenser_api
{
public:
   condenser_api();
   ~condenser_api();

   DECLARE_API(
      (get_version)
      (get_trending_tags)
      (get_state)
      (get_active_witnesses)
      (get_block_header)
      (get_block)
      (get_ops_in_block)
      (get_config)
      (get_dynamic_global_properties)
      (get_chain_properties)
      (get_witness_schedule)
      (get_hardfork_version)
      (get_next_scheduled_hardfork)
      (get_reward_fund)
      (get_key_references)
      (get_accounts)
      (get_account_references)
      (lookup_account_names)
      (lookup_accounts)
      (get_account_count)
      (get_owner_history)
      (get_recovery_request)
      (get_escrow)
      (get_withdraw_routes)
      (get_savings_withdraw_from)
      (get_savings_withdraw_to)
      (get_vesting_delegations)
      (get_expiring_vesting_delegations)
      (get_witnesses)
      (get_witness_by_account)
      (get_witnesses_by_vote)
      (lookup_witness_accounts)
      (get_witness_count)
      (get_transaction_hex)
      (get_transaction)
      (get_required_signatures)
      (get_potential_signatures)
      (verify_authority)
      (verify_account_authority)
      (get_active_votes)
      (get_account_votes)
      (get_content)
      (get_content_replies)
      (get_tags_used_by_author)
      (get_post_discussions_by_payout)
      (get_comment_discussions_by_payout)
      (get_discussions_by_trending)
      (get_discussions_by_created)
      (get_discussions_by_active)
      (get_discussions_by_cashout)
      (get_discussions_by_votes)
      (get_discussions_by_children)
      (get_discussions_by_hot)
      (get_discussions_by_feed)
      (get_discussions_by_blog)
      (get_discussions_by_comments)
      (get_discussions_by_promoted)
      (get_replies_by_last_update)
      (get_discussions_by_author_before_date)
      (get_account_history)
      (broadcast_transaction)
      (broadcast_transaction_synchronous)
      (broadcast_block)
      (get_followers)
      (get_following)
      (get_follow_count)
      (get_feed_entries)
      (get_feed)
      (get_blog_entries)
      (get_blog)
      (get_reblogged_by)
      (get_blog_authors)
      (list_proposals)
      (find_proposals)
      (list_proposal_votes)
   )

   private:
      friend class condenser_api_plugin;
      void api_startup();

      std::unique_ptr< detail::condenser_api_impl > my;
};

} } } // blurt::plugins::condenser_api

FC_REFLECT( blurt::plugins::condenser_api::discussion_index,
            (category)(trending)(payout)(payout_comments)(trending30)(updated)(created)(responses)(active)(votes)(maturing)(best)(hot)(promoted)(cashout) )

FC_REFLECT( blurt::plugins::condenser_api::api_tag_object,
            (name)(total_payouts)(net_votes)(top_posts)(comments)(trending) )

FC_REFLECT( blurt::plugins::condenser_api::state,
            (current_route)(props)(tag_idx)(tags)(content)(accounts)(witnesses)(discussion_idx)(witness_schedule)(error) )

FC_REFLECT( blurt::plugins::condenser_api::api_operation_object,
             (trx_id)(block)(trx_in_block)(op_in_trx)(virtual_op)(timestamp)(op) )

FC_REFLECT( blurt::plugins::condenser_api::api_account_object,
             (id)(name)(owner)(active)(posting)(memo_key)(json_metadata)(posting_json_metadata)
             (proxy)(last_owner_update)(last_account_update)
             (created)(mined)
             (recovery_account)(last_account_recovery)(reset_account)
             (comment_count)(lifetime_vote_count)(post_count)(can_vote)(voting_manabar)(voting_power)
             (balance)
             (savings_balance)
             (savings_withdraw_requests)
             (reward_blurt_balance)(reward_vesting_balance)(reward_vesting_blurt)
             (vesting_shares)(delegated_vesting_shares)(received_vesting_shares)(vesting_withdraw_rate)(next_vesting_withdrawal)(withdrawn)(to_withdraw)(withdraw_routes)
             (curation_rewards)
             (posting_rewards)
             (proxied_vsf_votes)(witnesses_voted_for)
             (last_post)(last_root_post)(last_vote_time)
             (post_bandwidth)(pending_claimed_accounts)
          )

FC_REFLECT_DERIVED( blurt::plugins::condenser_api::extended_account, (blurt::plugins::condenser_api::api_account_object),
            (vesting_balance)(transfer_history)(market_history)(post_history)(vote_history)(other_history)(witness_votes)(tags_usage)(guest_bloggers)(comments)(feed)(blog)(recent_replies)(recommended) )

FC_REFLECT( blurt::plugins::condenser_api::api_comment_object,
             (id)(author)(permlink)
             (category)(parent_author)(parent_permlink)
             (title)(body)(json_metadata)(last_update)(created)(active)(last_payout)
             (depth)(children)
             (net_rshares)(abs_rshares)(vote_rshares)
             (children_abs_rshares)(cashout_time)(max_cashout_time)
             (total_vote_weight)(reward_weight)(total_payout_value)(curator_payout_value)(author_rewards)(net_votes)
             (root_author)(root_permlink)
             (max_accepted_payout)(percent_blurt)(allow_replies)(allow_votes)(allow_curation_rewards)
             (beneficiaries)
          )

FC_REFLECT( blurt::plugins::condenser_api::extended_dynamic_global_properties,
            (head_block_number)(head_block_id)(time)
            (current_witness)
            (current_supply)
            (total_vesting_fund_blurt)(total_vesting_shares)
            (total_reward_fund_blurt)(total_reward_shares2)(pending_rewarded_vesting_shares)(pending_rewarded_vesting_blurt)
            (maximum_block_size)(current_aslot)(recent_slots_filled)(participation_count)(last_irreversible_block_num)
            (vote_power_reserve_rate)(delegation_return_period)(reverse_auction_seconds)(available_account_subsidies)
            (next_maintenance_time)(last_budget_time)(content_reward_percent)(vesting_reward_percent)(sps_fund_percent)(sps_interval_ledger)
          )

FC_REFLECT( blurt::plugins::condenser_api::api_witness_object,
             (id)
             (owner)
             (created)
             (url)(votes)(virtual_last_update)(virtual_position)(virtual_scheduled_time)(total_missed)
             (last_aslot)(last_confirmed_block_num)(signing_key)
             (props)
             (running_version)
             (hardfork_version_vote)(hardfork_time_vote)
             (available_witness_account_subsidies)
          )

FC_REFLECT( blurt::plugins::condenser_api::api_witness_schedule_object,
             (id)
             (current_virtual_time)
             (next_shuffle_block_num)
             (current_shuffled_witnesses)
             (num_scheduled_witnesses)
             (elected_weight)
             (timeshare_weight)
             (witness_pay_normalization_factor)
             (median_props)
             (majority_version)
             (max_voted_witnesses)
             (max_runner_witnesses)
             (hardfork_required_witnesses)
             (account_subsidy_rd)
             (account_subsidy_witness_rd)
             (min_witness_account_subsidy_decay)
          )

FC_REFLECT( blurt::plugins::condenser_api::api_reward_fund_object,
            (id)
            (name)
            (reward_balance)
            (recent_claims)
            (last_update)
            (content_constant)
            (percent_curation_rewards)
            (percent_content_rewards)
            (author_reward_curve)
            (curation_reward_curve)
         )

FC_REFLECT( blurt::plugins::condenser_api::api_escrow_object,
             (id)(escrow_id)(from)(to)(agent)
             (ratification_deadline)(escrow_expiration)
             (blurt_balance)(pending_fee)
             (to_approved)(agent_approved)(disputed) )

FC_REFLECT( blurt::plugins::condenser_api::api_savings_withdraw_object,
             (id)
             (from)
             (to)
             (memo)
             (request_id)
             (amount)
             (complete)
          )

FC_REFLECT( blurt::plugins::condenser_api::api_vesting_delegation_object,
            (id)(delegator)(delegatee)(vesting_shares)(min_delegation_time) )

FC_REFLECT( blurt::plugins::condenser_api::api_vesting_delegation_expiration_object,
            (id)(delegator)(vesting_shares)(expiration) )

FC_REFLECT( blurt::plugins::condenser_api::api_proposal_object,
             (id)(proposal_id)(creator)(receiver)(start_date)(end_date)(daily_pay)(subject)(permlink)(total_votes) )

FC_REFLECT_DERIVED( blurt::plugins::condenser_api::discussion, (blurt::plugins::condenser_api::api_comment_object),
             (url)(root_title)(pending_payout_value)(total_pending_payout_value)
             (active_votes)(replies)(promoted)
             (body_length)(reblogged_by)(first_reblogged_by)(first_reblogged_on)
          )

FC_REFLECT( blurt::plugins::condenser_api::scheduled_hardfork,
            (hf_version)(live_time) )

FC_REFLECT( blurt::plugins::condenser_api::account_vote,
            (authorperm)(weight)(rshares)(percent)(time) )

FC_REFLECT( blurt::plugins::condenser_api::tag_index, (trending) )

FC_REFLECT_ENUM( blurt::plugins::condenser_api::withdraw_route_type, (incoming)(outgoing)(all) )

FC_REFLECT( blurt::plugins::condenser_api::get_version_return,
            (blockchain_version)(blurt_revision)(fc_revision) )

FC_REFLECT( blurt::plugins::condenser_api::broadcast_transaction_synchronous_return,
            (id)(block_num)(trx_num)(expired) )

FC_REFLECT( blurt::plugins::condenser_api::comment_feed_entry,
            (comment)(reblog_by)(reblog_on)(entry_id) )

FC_REFLECT( blurt::plugins::condenser_api::comment_blog_entry,
            (comment)(blog)(reblog_on)(entry_id) )

