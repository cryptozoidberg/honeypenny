// Copyright (c) 2012-2013 The Cryptonote developers
// Copyright (c) 2012-2013 The Boolberry developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "currency_protocol/currency_protocol_defs.h"
#include "rpc/core_rpc_server_commands_defs.h"

#include "currency_core/currency_basic_impl.h"
#include "account.h"
#include "include_base_utils.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "crypto/wild_keccak.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "blockchain_storage_basic.h"

#define MAX_ALIAS_LEN         255
#define VALID_ALIAS_CHARS     "0123456789abcdefghijklmnopqrstuvwxyz-."

namespace currency
{

  struct tx_source_entry
  {
    typedef serializable_pair<uint64_t, crypto::public_key> output_entry;

    std::vector<output_entry> outputs;  //index + key
    uint64_t real_output;               //index in outputs vector of real output_entry
    crypto::public_key real_out_tx_key; //real output's transaction's public key
    size_t real_output_in_tx_index;     //index in transaction outputs vector
    uint64_t amount;                    //money
    uint64_t transfer_index;            //money

    BEGIN_SERIALIZE_OBJECT()
      FIELD(outputs)
      FIELD(real_output)
      FIELD(real_out_tx_key)
      FIELD(real_output_in_tx_index)
      FIELD(amount)
      FIELD(transfer_index)
    END_SERIALIZE()
  };

  struct tx_destination_entry
  {
    uint64_t amount;                    //money
    account_public_address addr;        //destination address

    tx_destination_entry() : amount(0), addr(AUTO_VAL_INIT(addr)) { }
    tx_destination_entry(uint64_t a, const account_public_address &ad) : amount(a), addr(ad) { }

    
    BEGIN_SERIALIZE_OBJECT()
      FIELD(amount)
      FIELD(addr)
    END_SERIALIZE()
  };

  struct alias_info_base
  {
    account_public_address m_address;
    crypto::secret_key m_view_key;
    crypto::signature m_sign;     //is this field set no nonzero - that means update alias operation
    std::string m_text_comment;

    BEGIN_SERIALIZE_OBJECT()
      FIELD(m_address)
      FIELD(m_view_key)
      FIELD(m_sign)
      FIELD(m_text_comment)
    END_SERIALIZE()
  };

  struct alias_info: public alias_info_base
  {
    std::string m_alias;
  };

  struct tx_extra_info 
  {
    crypto::public_key m_tx_pub_key;
    alias_info m_alias;
    std::string m_user_data_blob;
  };

  struct create_tx_arg
  {
    crypto::public_key spend_pub_key;  //for validations
    std::vector<currency::tx_source_entry> sources;
    std::vector<currency::tx_destination_entry> splitted_dsts;
    std::vector<uint8_t> extra;
    uint64_t unlock_time;
    uint8_t tx_outs_attr;
    uint64_t change_amount;
    std::vector<account_public_address> recipients;
    uint64_t dust;
      
    BEGIN_SERIALIZE_OBJECT()
      FIELD(spend_pub_key)
      FIELD(sources)
      FIELD(splitted_dsts)
      FIELD(extra)
      FIELD(unlock_time)
      FIELD(tx_outs_attr)
      FIELD(change_amount)
      FIELD(recipients)
      FIELD(dust)
    END_SERIALIZE()
  };

  struct create_tx_res
  {
    currency::transaction tx;
    keypair txkey;
    std::vector<serializable_pair<uint64_t, crypto::key_image>> outs_key_images; // pairs (out_index, key_image) for each change output

    BEGIN_SERIALIZE_OBJECT()
      FIELD(tx)
      FIELD(txkey)
      FIELD(outs_key_images)
    END_SERIALIZE()
  };

  struct create_tx_context
  {
    create_tx_arg arg;
    create_tx_res res;
    
    BEGIN_SERIALIZE_OBJECT()
      FIELD(arg)
      FIELD(res)
    END_SERIALIZE()
  };

  //---------------------------------------------------------------
  void get_transaction_prefix_hash(const transaction_prefix& tx, crypto::hash& h);
  crypto::hash get_transaction_prefix_hash(const transaction_prefix& tx);
  bool parse_and_validate_tx_from_blob(const blobdata& tx_blob, transaction& tx, crypto::hash& tx_hash, crypto::hash& tx_prefix_hash);
  bool parse_and_validate_tx_from_blob(const blobdata& tx_blob, transaction& tx);  
  bool get_donation_accounts(account_keys &donation_acc, account_keys &royalty_acc);
  bool construct_miner_tx(size_t height, size_t median_size, uint64_t already_generated_coins,
                                                             size_t current_block_size, 
                                                             uint64_t fee, 
                                                             const account_public_address &miner_address, 
                                                             transaction& tx, 
                                                             const blobdata& extra_nonce = blobdata(), 
                                                             size_t max_outs = 11);

  bool construct_miner_tx(size_t height, size_t median_size, uint64_t already_generated_coins, 
                                                             uint64_t already_donated_coins, 
                                                             size_t current_block_size, 
                                                             uint64_t fee, 
                                                             const account_public_address &miner_address, 
                                                             const account_public_address &donation_address, 
                                                             const account_public_address &royalty_address, 
                                                             transaction& tx, 
                                                             const blobdata& extra_nonce = blobdata(), 
                                                             size_t max_outs = 11, 
                                                             size_t amount_to_donate = 0, 
                                                             const alias_info& alias = alias_info()
                                                             );
  //---------------------------------------------------------------
  bool construct_tx_out(const account_public_address& destination_addr, const crypto::secret_key& tx_sec_key, size_t output_index, uint64_t amount, transaction& tx, uint8_t tx_outs_attr = CURRENCY_TO_KEY_OUT_RELAXED);
  bool validate_alias_name(const std::string& al);
  bool construct_tx(const account_keys& keys, const create_tx_arg& arg, create_tx_res& rsp);
  bool construct_tx(const account_keys& sender_account_keys, const std::vector<tx_source_entry>& sources, const std::vector<tx_destination_entry>& destinations, transaction& tx, keypair& txkey, uint64_t unlock_time, uint8_t tx_outs_attr = CURRENCY_TO_KEY_OUT_RELAXED);
  bool construct_tx(const account_keys& sender_account_keys, const std::vector<tx_source_entry>& sources, const std::vector<tx_destination_entry>& destinations, const std::vector<uint8_t>& extra, transaction& tx, keypair& txkey, uint64_t unlock_time, uint8_t tx_outs_attr = CURRENCY_TO_KEY_OUT_RELAXED);
  bool sign_update_alias(alias_info& ai, const crypto::public_key& pkey, const crypto::secret_key& skey);
  bool make_tx_extra_alias_entry(std::string& buff, const alias_info& alinfo, bool make_buff_to_sign = false);
  bool add_tx_extra_alias(transaction& tx, const alias_info& alinfo);
  bool parse_and_validate_tx_extra(const transaction& tx, tx_extra_info& extra);
  bool parse_and_validate_tx_extra(const transaction& tx, crypto::public_key& tx_pub_key);
  crypto::public_key get_tx_pub_key_from_extra(const transaction& tx);
  bool add_tx_pub_key_to_extra(transaction& tx, const crypto::public_key& tx_pub_key);
  bool add_tx_extra_nonce(transaction& tx, const blobdata& extra_nonce);
  bool is_out_to_acc(const account_keys& acc, const txout_to_key& out_key, const crypto::public_key& tx_pub_key, size_t output_index);
  bool lookup_acc_outs(const account_keys& acc, const transaction& tx, const crypto::public_key& tx_pub_key, std::vector<size_t>& outs, uint64_t& money_transfered);
  bool lookup_acc_outs(const account_keys& acc, const transaction& tx, std::vector<size_t>& outs, uint64_t& money_transfered);
  bool get_tx_fee(const transaction& tx, uint64_t & fee);
  uint64_t get_tx_fee(const transaction& tx);
  bool generate_key_image_helper(const account_keys& ack, const crypto::public_key& tx_public_key, size_t real_output_index, keypair& in_ephemeral, crypto::key_image& ki);
  void get_blob_hash(const blobdata& blob, crypto::hash& res);
  crypto::hash get_blob_hash(const blobdata& blob);
  std::string short_hash_str(const crypto::hash& h);
  //bool get_block_scratchpad_addendum(const block& b, std::vector<crypto::hash>& res);
  //bool get_scratchpad_patch(size_t global_start_entry, size_t local_start_entry, size_t local_end_entry, const std::vector<crypto::hash>& scratchpd, std::map<uint64_t, crypto::hash>& patch);
  //bool push_block_scratchpad_data(const block& b, std::vector<crypto::hash>& scratchpd);
  //bool push_block_scratchpad_data(size_t global_start_entry, const block& b, std::vector<crypto::hash>& scratchpd, std::map<uint64_t, crypto::hash>& patch);
  //bool pop_block_scratchpad_data(const block& b, std::vector<crypto::hash>& scratchpd);
  //bool apply_scratchpad_patch(std::vector<crypto::hash>& scratchpd, std::map<uint64_t, crypto::hash>& patch);
  bool is_mixattr_applicable_for_fake_outs_counter(uint8_t mix_attr, uint64_t fake_attr_count);
  serializable_pair<uint64_t, crypto::public_key> make_output_entry(uint64_t index, const crypto::public_key& key);

  bool get_reward_from_miner_tx(const transaction& tx, uint64_t& reward);

  crypto::hash get_transaction_hash(const transaction& t);
  bool get_transaction_hash(const transaction& t, crypto::hash& res);
  //bool get_transaction_hash(const transaction& t, crypto::hash& res, size_t& blob_size);
  blobdata get_block_hashing_blob(const block& b);
  bool get_block_hash(const block& b, crypto::hash& res);
  crypto::hash get_block_hash(const block& b);
  bool generate_genesis_block(block& bl);
  block generate_genesis_block();
  const crypto::hash& get_genesis_id();
  bool parse_and_validate_block_from_blob(const blobdata& b_blob, block& b);
  bool get_inputs_money_amount(const transaction& tx, uint64_t& money);
  uint64_t get_outs_money_amount(const transaction& tx);
  bool check_inputs_types_supported(const transaction& tx);
  bool check_outs_valid(const transaction& tx);
  blobdata get_block_hashing_blob(const block& b);
  bool parse_amount(uint64_t& amount, const std::string& str_amount);
  bool parse_payment_id_from_hex_str(const std::string& payment_id_str, payment_id_t& payment_id);

  bool check_money_overflow(const transaction& tx);
  bool check_outs_overflow(const transaction& tx);
  bool check_inputs_overflow(const transaction& tx);
  uint64_t get_block_height(const block& b);
  std::vector<uint64_t> relative_output_offsets_to_absolute(const std::vector<uint64_t>& off);
  std::vector<uint64_t> absolute_output_offsets_to_relative(const std::vector<uint64_t>& off);
  std::string print_money(uint64_t amount, bool trim_zeros = false);
  std::string dump_scratchpad(const std::vector<crypto::hash>& scr);
  std::string dump_patch(const std::map<uint64_t, crypto::hash>& patch);
  
  bool addendum_to_hexstr(const std::vector<crypto::hash>& add, std::string& hex_buff);
  bool hexstr_to_addendum(const std::string& hex_buff, std::vector<crypto::hash>& add);
  bool set_payment_id_and_swap_addr_to_tx_extra(std::vector<uint8_t>& extra, const payment_id_t& payment_id, const account_public_address& acc = account_public_address());
  bool get_payment_id_from_user_data(const std::string& user_data, payment_id_t& payment_id);
  bool get_payment_id_from_tx_extra(const transaction& tx, payment_id_t& payment_id);
  bool get_swap_info_from_tx_extra(const transaction& tx, const crypto::secret_key& sk, account_public_address& addr);
  bool get_swap_info_from_tx(const transaction& tx, const crypto::secret_key& sk, swap_transaction_info& swap_info);
  bool encrypt_user_data_with_tx_secret_key(const crypto::secret_key& sk, std::vector<uint8_t>& extra);
  bool is_swap_tx(const currency::transaction& tx);
  bool is_swap_tx(const currency::transaction& tx, const std::vector<tx_destination_entry>& destinations);
  bool encrypt_user_data_with_tx_secret_key(const crypto::secret_key& sk, std::vector<uint8_t>& extra);

  crypto::hash get_blob_longhash(const blobdata& bd, uint64_t height, const std::vector<crypto::hash>& scratchpad);
  crypto::hash get_blob_longhash_opt(const blobdata& bd, const std::vector<crypto::hash>& scratchpad);


  bool fill_tx_rpc_outputs(tx_rpc_extended_info& tei, const transaction& tx, const transaction_chain_entry* ptce);
  bool fill_tx_rpc_inputs(tx_rpc_extended_info& tei, const transaction& tx);
  bool fill_block_rpc_details(block_rpc_extended_info& pei_rpc, const block_extended_info& bei_chain, const crypto::hash& h);
  bool fill_tx_rpc_extra_items(std::vector<tx_extra_rpc_entry>& extra_rpc_entry, const transaction& tx);
  bool fill_tx_rpc_details(tx_rpc_extended_info& tei, const transaction& tx, const transaction_chain_entry* ptce, const crypto::hash& h, uint64_t timestamp, bool is_short = false);


  void print_currency_details();
  void print_coins_that_can_be_swapped();
    
  //---------------------------------------------------------------
  template<typename callback_t>
  bool get_blob_longhash(const blobdata& bd, crypto::hash& res, uint64_t height, callback_t accessor)
  {
    crypto::wild_keccak_dbl<crypto::mul_f>(reinterpret_cast<const uint8_t*>(bd.data()), bd.size(), reinterpret_cast<uint8_t*>(&res), sizeof(res), [&](crypto::state_t_m& st, crypto::mixin_t& mix)
    {
      if(!height)
      {
        memset(&mix, 0, sizeof(mix));
        return;
      }
#define GET_H(index) accessor(st[index])
      for(size_t i = 0; i!=6; i++)
      {
        *(crypto::hash*)&mix[i*4]  = XOR_4(GET_H(i*4), GET_H(i*4+1), GET_H(i*4+2), GET_H(i*4+3));  
      }
    });
    return true;
  }
  //---------------------------------------------------------------
  template<typename callback_t>
  bool get_block_longhash(const block& b, crypto::hash& res, uint64_t height, callback_t accessor)
  {
    blobdata bd = get_block_hashing_blob(b);
    return get_blob_longhash(bd, res, height, accessor);
  }
  //---------------------------------------------------------------
  template<typename callback_t>
  crypto::hash get_block_longhash(const block& b, uint64_t height, callback_t cb)
  {
    crypto::hash p = null_hash;
    get_block_longhash(b, p, height, cb);
    return p;
  }

  //---------------------------------------------------------------

  //---------------------------------------------------------------
  template<class t_object>
  bool get_object_hash(const t_object& o, crypto::hash& res)
  {
    get_blob_hash(t_serializable_object_to_blob(o), res);
    return true;
  }
  //---------------------------------------------------------------
  template<class t_object>
  crypto::hash get_object_hash(const t_object& o)
  {
    crypto::hash h;
    get_object_hash(o, h);
    return h;
  }
  //---------------------------------------------------------------

  template<class t_object>
  size_t get_object_blobsize(const t_object& o)
  {
    blobdata b = t_serializable_object_to_blob(o);
    return b.size();
  }
  //---------------------------------------------------------------
  size_t get_object_blobsize(const transaction& t);
  //---------------------------------------------------------------
  template<class t_object>
  bool get_object_hash(const t_object& o, crypto::hash& res, size_t& blob_size)
  {
    blobdata bl = t_serializable_object_to_blob(o);
    blob_size = bl.size();
    get_blob_hash(bl, res);
    return true;
  }
  //---------------------------------------------------------------
  template <typename T>
  std::string obj_to_json_str(const T& obj)
  {
    std::stringstream ss;
    json_archive<true> ar(ss, true);
    bool r = ::serialization::serialize(ar, const_cast<T&>(obj));
    CHECK_AND_ASSERT_MES(r, "", "obj_to_json_str failed: serialization::serialize returned false");
    return ss.str();
  }
  //---------------------------------------------------------------
  inline serializable_pair<uint64_t, crypto::public_key> make_output_entry(uint64_t index, const crypto::public_key& key)
  {
    return serializable_pair<uint64_t, crypto::public_key>(index, key);
  }
  //---------------------------------------------------------------
  // 62387455827 -> 455827 + 7000000 + 80000000 + 300000000 + 2000000000 + 60000000000, where 455827 <= dust_threshold
  template<typename chunk_handler_t, typename dust_handler_t>
  void decompose_amount_into_digits(uint64_t amount, uint64_t dust_threshold, const chunk_handler_t& chunk_handler, const dust_handler_t& dust_handler)
  {
    if (0 == amount)
    {
      return;
    }

    bool is_dust_handled = false;
    uint64_t dust = 0;
    uint64_t order = 1;
    while (0 != amount)
    {
      uint64_t chunk = (amount % 10) * order;
      amount /= 10;
      order *= 10;

      if (dust + chunk <= dust_threshold)
      {
        dust += chunk;
      }
      else
      {
        if (!is_dust_handled && 0 != dust)
        {
          dust_handler(dust);
          is_dust_handled = true;
        }
        if (0 != chunk)
        {
          chunk_handler(chunk);
        }
      }
    }

    if (!is_dust_handled && 0 != dust)
    {
      dust_handler(dust);
    }
  }

  blobdata block_to_blob(const block& b);
  bool block_to_blob(const block& b, blobdata& b_blob);
  blobdata tx_to_blob(const transaction& b);
  bool tx_to_blob(const transaction& b, blobdata& b_blob);
  void get_tx_tree_hash(const std::vector<crypto::hash>& tx_hashes, crypto::hash& h);
  crypto::hash get_tx_tree_hash(const std::vector<crypto::hash>& tx_hashes);
  crypto::hash get_tx_tree_hash(const block& b);

#define CHECKED_GET_SPECIFIC_VARIANT(variant_var, specific_type, variable_name, fail_return_val) \
  CHECK_AND_ASSERT_MES(variant_var.type() == typeid(specific_type), fail_return_val, "wrong variant type: " << variant_var.type().name() << ", expected " << typeid(specific_type).name()); \
  specific_type& variable_name = boost::get<specific_type>(variant_var);

}
