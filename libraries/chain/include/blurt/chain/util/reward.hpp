#pragma once

#include <blurt/protocol/asset.hpp>
#include <blurt/protocol/config.hpp>
#include <blurt/protocol/types.hpp>
#include <blurt/protocol/misc_utilities.hpp>
#include <blurt/chain/global_property_object.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128.hpp>

namespace blurt { namespace chain { namespace util {

using blurt::protocol::asset;
using blurt::protocol::price;
using blurt::protocol::share_type;

using fc::uint128_t;

struct comment_reward_context
{
   share_type rshares;
   uint16_t   reward_weight = 0;
   asset      max_payout;
   uint128_t  total_reward_shares2;
   asset      total_reward_fund_blurt;
   protocol::curve_id   reward_curve = protocol::quadratic;
   uint128_t  content_constant = BLURT_REWARD_CONSTANT;
};

uint64_t get_rshare_reward( const comment_reward_context& ctx, const dynamic_global_property_object& gpo, uint32_t hardfork );

uint128_t evaluate_reward_curve( const uint128_t& rshares, const protocol::curve_id& curve = protocol::quadratic, const uint128_t& var1 = BLURT_REWARD_CONSTANT );

inline bool is_comment_payout_dust( uint64_t blurt_payout )
{
   return asset( blurt_payout, BLURT_SYMBOL ) < BLURT_MIN_PAYOUT;
}

} } } // blurt::chain::util

FC_REFLECT( blurt::chain::util::comment_reward_context,
   (rshares)
   (reward_weight)
   (max_payout)
   (total_reward_shares2)
   (total_reward_fund_blurt)
   (reward_curve)
   (content_constant)
   )
