#pragma once

#include <blurt/protocol/types.hpp>

#include <blurt/protocol/operation_util.hpp>
#include <blurt/protocol/blurt_operations.hpp>
#include <blurt/protocol/blurt_virtual_operations.hpp>
#include <blurt/protocol/sps_operations.hpp>

namespace blurt { namespace protocol {

   /** NOTE: do not change the order of any operations prior to the virtual operations
    * or it will trigger a hardfork.
    */
   typedef fc::static_variant<
            vote_operation,
            comment_operation,

            transfer_operation,
            transfer_to_vesting_operation,
            withdraw_vesting_operation,

            account_create_operation,
            account_update_operation,

            witness_update_operation,
            account_witness_vote_operation,
            account_witness_proxy_operation,

            custom_operation,

            delete_comment_operation,
            custom_json_operation,
            comment_options_operation,
            set_withdraw_vesting_route_operation,
            claim_account_operation,
            create_claimed_account_operation,
            request_account_recovery_operation,
            recover_account_operation,
            change_recovery_account_operation,
            escrow_transfer_operation,
            escrow_dispute_operation,
            escrow_release_operation,
            escrow_approve_operation,
            transfer_to_savings_operation,
            transfer_from_savings_operation,
            cancel_transfer_from_savings_operation,
            custom_binary_operation,
            decline_voting_rights_operation,
            reset_account_operation,
            set_reset_account_operation,
            claim_reward_balance_operation,
            delegate_vesting_shares_operation,
            witness_set_properties_operation,
            create_proposal_operation,
            update_proposal_votes_operation,
            remove_proposal_operation,

            /// virtual operations below this point
            author_reward_operation,
            curation_reward_operation,
            comment_reward_operation,
            fill_vesting_withdraw_operation,
            shutdown_witness_operation,
            fill_transfer_from_savings_operation,
            hardfork_operation,
            comment_payout_update_operation,
            return_vesting_delegation_operation,
            comment_benefactor_reward_operation,
            producer_reward_operation,
            clear_null_account_balance_operation,
            proposal_pay_operation,
            sps_fund_operation
         > operation;

   /*void operation_get_required_authorities( const operation& op,
                                            flat_set<string>& active,
                                            flat_set<string>& owner,
                                            flat_set<string>& posting,
                                            vector<authority>&  other );

   void operation_validate( const operation& op );*/

   bool is_market_operation( const operation& op );

   bool is_virtual_operation( const operation& op );

   void set_op_hardfork( operation& op, uint32_t hardfork );

} } // blurt::protocol

/*namespace fc {
    void to_variant( const blurt::protocol::operation& var,  fc::variant& vo );
    void from_variant( const fc::variant& var,  blurt::protocol::operation& vo );
}*/

BLURT_DECLARE_OPERATION_TYPE( blurt::protocol::operation )
FC_REFLECT_TYPENAME( blurt::protocol::operation )
