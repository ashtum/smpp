// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class command_status : uint32_t
{
    rok           = 0x00000000, // no error
    rinvmsglen    = 0x00000001, // message length is invalid
    rinvcmdlen    = 0x00000002, // command length is invalid
    rinvcmdid     = 0x00000003, // invalid command id
    rinvbndsts    = 0x00000004, // incorrect bind status for given command
    ralybnd       = 0x00000005, // esme already in bound state
    rinvprtflg    = 0x00000006, // invalid priority flag
    rinvregdlvflg = 0x00000007, // invalid registered delivery flag
    rsyserr       = 0x00000008, // system error
    rinvsrcadr    = 0x0000000a, // invalid source address
    rinvdstadr    = 0x0000000b, // invalid dest addr
    rinvmsgid     = 0x0000000c, // message id is invalid
    rbindfail     = 0x0000000d, // bind failed
    rinvpaswd     = 0x0000000e, // invalid password
    rinvsysid     = 0x0000000f, // invalid system id
    rcancelfail   = 0x00000011, // cancel sm failed
    rreplacefail  = 0x00000013, // replace sm failed
    rmsgqful      = 0x00000014, // message queue full
    rinvsertyp    = 0x00000015, // invalid service type
    rinvnumdests  = 0x00000033, // invalid number of destinations
    rinvdlname    = 0x00000034, // invalid distribution list name
    rinvdestflag  = 0x00000040, // destination flag (submit_multi)
    rinvsubrep    = 0x00000042, // invalid ‘submit with replace’ request
    rinvesmsubmit = 0x00000043, // invalid esm_submit field data
    rcntsubdl     = 0x00000044, // cannot submit to distribution list
    rsubmitfail   = 0x00000045, // submit_sm or submit_multi failed
    rinvsrcton    = 0x00000048, // invalid source address ton
    rinvsrcnpi    = 0x00000049, // invalid source address npi
    rinvdstton    = 0x00000050, // invalid destination address ton
    rinvdstnpi    = 0x00000051, // invalid destination address npi
    rinvsystyp    = 0x00000053, // invalid system_type field
    rinvrepflag   = 0x00000054, // invalid replace_if_present flag
    rinvnummsgs   = 0x00000055, // invalid number of messages
    rthrottled    = 0x00000058, // throttling error (esme has exceeded allowed
                                // message limits)
    rinvsched    = 0x00000061,  // invalid scheduled delivery time
    rinvexpiry   = 0x00000062,  // invalid message (expiry time)
    rinvdftmsgid = 0x00000063,  // predefined message invalid or not found
    rx_t_appn    = 0x00000064,  // esme receiver temporary app error code
    rx_p_appn    = 0x00000065,  // esme receiver permanent app error code
    rx_r_appn    = 0x00000066,  // esme receiver reject message error code
    rqueryfail   = 0x00000067,  // query_sm request failed
    rinvoptparstream =
        0x000000c0, // error in the optional part of the pdu body.
    roptparnotallwd    = 0x000000c1, // optional parameter not allowed
    rinvparlen         = 0x000000c2, // invalid parameter length.
    rmissingoptparam   = 0x000000c3, // expected optional parameter missing
    rinvoptparamval    = 0x000000c4, // invalid optional parameter value
    rdeliveryfailure   = 0x000000fe, // delivery failure (data_sm_resp)
    runknownerr        = 0x000000ff, // unknown error
    rinvseqnum         = 0x00000400, // Invalid sequence number
    rtimeout           = 0x00000401, // time out
    src_esme_not_bound = 0x00000402, // source esme client is not bound
    dst_esme_not_bound = 0x00000403, // destination esme client is not bound
    max_try_count      = 0x00000404, // message is reached to max try count
    src_has_no_credit  = 0x00000405, // source number does not have credit
    dst_has_no_credit  = 0x00000406, // destination number does not have credit
    src_in_black_list  = 0x00000407, // source number is in black list
    dst_in_black_list  = 0x00000408, // destination number is in black list
    src_not_in_white_list = 0x00000409, // source number is not in white list
    dst_not_in_white_list =
        0x0000040A,     // destination number is not in white list
    rinvip = 0x0000040B // invalid ip
};
} // namespace smpp
