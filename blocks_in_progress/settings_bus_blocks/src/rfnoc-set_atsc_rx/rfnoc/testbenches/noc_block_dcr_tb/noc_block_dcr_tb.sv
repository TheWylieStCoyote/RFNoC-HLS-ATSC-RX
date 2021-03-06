/* 
 * Copyright 2017 Rabbit Ears, RFNoC Challenge.
 * Author: Andrew Lanez
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

`timescale 1ns/1ps
`define NS_PER_TICK 1
`define NUM_TEST_CASES 5

`include "sim_exec_report.vh"
`include "sim_clks_rsts.vh"
`include "sim_rfnoc_lib.svh"
`include "dcr_in.sv"
`include "dcr_out.sv"

module noc_block_dcr_tb();
  `TEST_BENCH_INIT("noc_block_dcr",`NUM_TEST_CASES,`NS_PER_TICK);
  localparam BUS_CLK_PERIOD = $ceil(1e9/166.67e6);
  localparam CE_CLK_PERIOD  = $ceil(1e9/200e6);
  localparam NUM_CE         = 1;  // Number of Computation Engines / User RFNoC blocks to simulate
  localparam NUM_STREAMS    = 1;  // Number of test bench streams
  `RFNOC_SIM_INIT(NUM_CE, NUM_STREAMS, BUS_CLK_PERIOD, CE_CLK_PERIOD);
  `RFNOC_ADD_BLOCK(noc_block_dcr, 0);

  localparam SPP = 16; // Samples per packet

  /********************************************************
  ** Verification
  ********************************************************/
  initial begin : tb_main
    string s;
    logic [31:0] random_word;
    logic [63:0] readback;
    logic [31:0] length, long_form;

    /********************************************************
    ** Test 1 -- Reset
    ********************************************************/
    `TEST_CASE_START("Wait for Reset");
    while (bus_rst) @(posedge bus_clk);
    while (ce_rst) @(posedge ce_clk);
    `TEST_CASE_DONE(~bus_rst & ~ce_rst);

    /********************************************************
    ** Test 2 -- Check for correct NoC IDs
    ********************************************************/
    `TEST_CASE_START("Check NoC ID");
    // Read NOC IDs
    tb_streamer.read_reg(sid_noc_block_dcr, RB_NOC_ID, readback);
    $display("Read dcr NOC ID: %16x", readback);
    `ASSERT_ERROR(readback == noc_block_dcr.NOC_ID, "Incorrect NOC ID");
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 3 -- Connect RFNoC blocks
    ********************************************************/
    `TEST_CASE_START("Connect RFNoC blocks");
    `RFNOC_CONNECT(noc_block_tb,noc_block_dcr,SC16,SPP);
    `RFNOC_CONNECT(noc_block_dcr,noc_block_tb,SC16,SPP);
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 4 -- Write / readback user registers
    ********************************************************/
    `TEST_CASE_START("Write / readback user registers");
    random_word = $random();
    tb_streamer.write_user_reg(sid_noc_block_dcr, noc_block_dcr.SR_LENGTH, random_word);
    tb_streamer.read_user_reg(sid_noc_block_dcr, 0, readback);
    $sformat(s, "User register 0 incorrect readback! Expected: %0d, Actual %0d", readback[31:0], random_word);
    `ASSERT_ERROR(readback[31:0] == random_word, s);
    random_word = $random();
    tb_streamer.write_user_reg(sid_noc_block_dcr, noc_block_dcr.SR_LONG_FORM, random_word);
    tb_streamer.read_user_reg(sid_noc_block_dcr, 1, readback);
    $sformat(s, "User register 1 incorrect readback! Expected: %0d, Actual %0d", readback[31:0], random_word);
    `ASSERT_ERROR(readback[31:0] == random_word, s);
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 5 -- Test sequence
    ********************************************************/
    // dcr's user code is tested
    `TEST_CASE_START("Test sequence");
    length = 32'd128;
    long_form = 32'd1;
    tb_streamer.write_user_reg(sid_noc_block_dcr, noc_block_dcr.SR_LENGTH, length);
    tb_streamer.write_user_reg(sid_noc_block_dcr, noc_block_dcr.SR_LONG_FORM, long_form);
    begin
      localparam COUNT = 256; // Number of samples (max 16384 available)
      localparam SPL = 128; // Samples per LAST assertion
      localparam THRESH = 0.0001;
      logic [31:0] exp_val, recv_val;
      logic last;
      shortreal diff = 0;
      fork
      begin
        for (int i = 0; i < COUNT/SPL; i++) begin
	  for (int j = 0; j < SPL; j++) begin
            tb_streamer.push_word($shortrealtobits(in[i*SPL+j]), j == SPL-1);
          end
        end
      end
      begin
        for (int i = 0; i < COUNT; i++) begin
          exp_val = $shortrealtobits(out[i]);
          tb_streamer.pull_word(recv_val, last);
          diff = $bitstoshortreal(exp_val) - $bitstoshortreal(recv_val);
          $sformat(s, "%0d: Incorrect value received! Expected: %.20f, Received: %.20f", i, $bitstoshortreal(exp_val), $bitstoshortreal(recv_val));
          `ASSERT_ERROR(-1*THRESH<diff && diff<THRESH, s);
        end
      end
      join
    end
    `TEST_CASE_DONE(1);
    `TEST_BENCH_DONE;

  end
endmodule
