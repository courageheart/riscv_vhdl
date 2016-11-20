/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer divider.
 */

#include "int_div.h"
#include "api_utils.h"

namespace debugger {

IntDiv::IntDiv(sc_module_name name_, sc_trace_file *vcd)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_rv32;
    sensitive << i_residual;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.result;
    sensitive << r.ena;
    sensitive << r.busy;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

#if 0
    vcd = sc_create_vcd_trace_file("intdiv");
    vcd->set_time_unit(1, SC_PS);

    sc_trace(vcd, i_clk, "i_clk");
    sc_trace(vcd, i_nrst, "i_nrst");
    sc_trace(vcd, i_ena, "i_ena");
    sc_trace(vcd, i_unsigned, "i_unsigned");
    sc_trace(vcd, i_rv32, "i_rv32");
    sc_trace(vcd, i_residual, "i_residual");
    sc_trace(vcd, i_a1, "i_a1");
    /*sc_trace(vcd, i_a1.read()[63], "i_a1 [63]");
    sc_trace(vcd, i_a1.read()[62], "i_a1 [62]");
    sc_trace(vcd, i_a1.read()[61], "i_a1 [61]");
    sc_trace(vcd, i_a1.read()[60], "i_a1 [60]");
    sc_trace(vcd, i_a1.read()[59], "i_a1 [59]");
    sc_trace(vcd, i_a1.read()[58], "i_a1 [58]");
    sc_trace(vcd, i_a1.read()[57], "i_a1 [57]");
    sc_trace(vcd, i_a1.read()[56], "i_a1 [56]");
    sc_trace(vcd, i_a1.read()[55], "i_a1 [55]");
    sc_trace(vcd, i_a1.read()[54], "i_a1 [54]");*/
    sc_trace(vcd, i_a2, "i_a2");
    //sc_trace(vcd, o_res, "o_res");
    //sc_trace(vcd, o_valid, "o_valid");
    //sc_trace(vcd, o_busy, "o_busy");
#else
    if (vcd) {
        sc_trace(vcd, i_ena, "/top/proc0/exec0/div0/i_ena");
        sc_trace(vcd, o_res, "/top/proc0/exec0/div0/o_res");
        sc_trace(vcd, o_valid, "/top/proc0/exec0/div0/o_valid");
        sc_trace(vcd, r.ena, "/top/proc0/exec0/div0/r_ena");
        sc_trace(vcd, r.busy, "/top/proc0/exec0/div0/r_busy");
        sc_trace(vcd, r.qr, "/top/proc0/exec0/div0/r_qr");
        sc_trace(vcd, r.invert, "/top/proc0/exec0/div0/r_invert");
        sc_trace(vcd, r.rv32, "/top/proc0/exec0/div0/r_rv32");
        sc_trace(vcd, r.resid, "/top/proc0/exec0/div0/r_resid");
        sc_trace(vcd, r.reference_div, "/top/proc0/exec0/div0/r_reference_div");
        sc_trace(vcd, wb_qr1, "/top/proc0/exec0/div0/wb_qr1");
        sc_trace(vcd, wb_qr2, "/top/proc0/exec0/div0/wb_qr2");

    }
#endif
};


void IntDiv::comb() {
    sc_uint<64> wb_a1;
    sc_uint<64> wb_a2;
    sc_biguint<65> wb_divident = 0;
    sc_biguint<65> wb_divider = 0;

    v = r;

    wb_divident[64] = 0;
    wb_divider[64] = 0;

    if (i_rv32.read()) {
        wb_a1(31, 0) = i_a1.read()(31, 0);
        wb_a2(31, 0) = i_a2.read()(31, 0);
        if (i_unsigned.read() || i_a1.read()[31] == 0) {
            wb_a1(63, 32) = 0;
        } else {
            wb_a1(63, 32) = ~0;
        }
        if (i_unsigned.read() || i_a2.read()[31] == 0) {
            wb_a2(63, 32) = 0;
        } else {
            wb_a2(63, 32) = ~0;
        }
    } else {
        wb_a1(31, 0) = i_a1;
        wb_a2(31, 0) = i_a2;
    }

    if (i_unsigned.read() || wb_a1[63] == 0) {
        wb_divident(63, 0) = wb_a1;
    } else {
        wb_divident(63, 0) = (~wb_a1) + 1;
    }
    if (i_unsigned.read() || wb_a2[63] == 0) {
        wb_divider(63, 0) = wb_a2;
    } else {
        wb_divider(63, 0) = (~wb_a2) + 1;
    }


    v.ena = (r.ena.read() << 1) | (i_ena & !r.busy);

    // Level 2*i of 64:
    wb_diff1 = r.qr(127, 63) - r.divider;
    if (wb_diff1[64]) {
        wb_qr1 = r.qr << 1;
    } else {
        wb_qr1 = (wb_diff1(63, 0), r.qr(62, 0), 1);
    }

    // Level 2*i + 1 of 64:
    wb_diff2 = wb_qr1(127, 63) - r.divider;
    if (wb_diff2[64]) {
        wb_qr2 = wb_qr1 << 1;
    } else {
        wb_qr2 = (wb_diff2(63, 0), wb_qr1(62, 0), 1);
    }


    if (i_ena.read()) {
        v.qr(127, 65) = 0;
        v.qr(64, 0) = wb_divident;
        v.divider = wb_divider;
        v.busy = 1;
        v.rv32 = i_rv32;
        v.resid = i_residual;
        v.invert = !i_unsigned.read() & (i_a1.read()[63] ^ i_a2.read()[63]);

        v.a1_dbg = i_a1;
        v.a2_dbg = i_a2;
        v.reference_div = compute_reference(i_unsigned.read(), i_rv32.read(),
                                     i_residual.read(),
                                     i_a1.read(), i_a2.read());
    } else if (r.ena.read()[32]) {
        v.busy = 0;
        if (r.invert.read()) {
            v.qr = (~r.qr) + 1;
        } else {
            v.qr = r.qr;
        }
        if (r.resid.read()) {
            v.result = r.qr(127, 64).to_uint64();
        } else {
            v.result = r.qr(63, 0).to_uint64();
        }
    } else if (r.busy.read()) {
        v.qr = wb_qr2;
    }
    

    if (i_nrst.read() == 0) {
        v.result = 0;
        v.ena = 0;
        v.busy = 0;
        v.rv32 = 0;
        v.invert = 0;
        v.qr = 0;
        v.resid = 0;
    }

    o_res = r.result;
    o_valid = r.ena.read()[33];
    o_busy = r.busy;
}

void IntDiv::registers() {
    // Debug purpose only"
    if (v.ena.read()[33]) {
        uint64_t t1 = v.result.read()(63,0).to_uint64();
        uint64_t t2 = r.reference_div.to_uint64();
        if (t1 != t2) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                "IntDiv error: rv32=%d, resid=%d, invert=%d, "
                "(%016" RV_PRI64 "x/%016" RV_PRI64 "x) => "
                "%016" RV_PRI64 "x != %016" RV_PRI64 "x\n",
                r.rv32.read(), r.resid.read(), r.invert.read(),
                r.a1_dbg.to_uint64(), r.a2_dbg.to_uint64(), t1, t2);
            cout << tstr;
            cout.flush();
        }
    }

    r = v;
}

uint64_t IntDiv::compute_reference(bool unsign, bool rv32, bool resid,
                                   uint64_t a1, uint64_t a2) {
    uint64_t ret;
    if (a2 == 0) {
        ret = 0;
    } else if (rv32) {
        if (unsign) {
            if (resid) {
                ret = (uint32_t)a1 % (uint32_t)a2;
            } else {
                ret = (uint32_t)a1 / (uint32_t)a2;
            }
        } else {
            if (resid) {
                ret = (uint64_t)((int64_t)((int32_t)a1 % (int32_t)a2));
            } else {
                ret = (uint64_t)((int64_t)((int32_t)a1 / (int32_t)a2));
            }
        }
    } else {
        if (unsign) {
            if (resid) {
                ret = a1 % a2;
            } else {
                ret = a1 / a2;
            }
        } else {
            if (resid) {
                ret = (int64_t)a1 % (int64_t)a2;
            } else {
                ret = (int64_t)a1 / (int64_t)a2;
            }
        }
    }
    return ret;
}

}  // namespace debugger

