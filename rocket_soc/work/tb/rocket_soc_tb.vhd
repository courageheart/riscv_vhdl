-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief  	  Testbench file for the SoC top-level impleemntation
------------------------------------------------------------------------------
--! @details   File was automatically generated by C++ simulation software
------------------------------------------------------------------------------
--! @warning
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_util.all;
library rocketlib;
--use rocketlib.types_rocket.all;

entity rocket_soc_tb is
  constant INCR_TIME : time := 3571 ps;--100 ns;--3571 ps;
end rocket_soc_tb;

architecture behavior of rocket_soc_tb is


  -- input/output signals:
  signal i_rst : std_logic := '1';
  signal i_sclk_p : std_logic;
  signal i_sclk_n : std_logic;
  signal i_clk_adc : std_logic := '0';
  signal i_int_clkrf : std_logic := '1';
  signal i_dip : std_logic_vector(3 downto 1);
  signal o_led : std_logic_vector(7 downto 0);
  signal i_uart1_ctsn : std_logic := '0';
  signal i_uart1_rd : std_logic := '1';
  signal o_uart1_td : std_logic;
  signal o_uart1_rtsn : std_logic;
  
  signal i_gps_ld    : std_logic := '0';--'1';
  signal i_glo_ld    : std_logic := '0';--'1';
  signal o_max_sclk  : std_logic;
  signal o_max_sdata : std_logic;
  signal o_max_ncs   : std_logic_vector(1 downto 0);
  signal i_antext_stat   : std_logic := '0';
  signal i_antext_detect : std_logic := '0';
  signal o_antext_ena    : std_logic;
  signal o_antint_contr  : std_logic;

  signal o_emdc    : std_logic;
  signal io_emdio  : std_logic;

  signal adc_cnt : integer := 0;
  signal clk_cur: std_logic := '1';
  signal check_clk_bus : std_logic := '0';
  signal iClkCnt : integer := 0;
  signal iErrCnt : integer := 0;
  signal iErrCheckedCnt : integer := 0;
  
component rocket_soc is port 
( 
  i_rst     : in std_logic; -- button "Center"
  i_sclk_p  : in std_logic;
  i_sclk_n  : in std_logic;
  i_clk_adc : in std_logic;
  i_int_clkrf : in std_logic;
  i_dip     : in std_logic_vector(3 downto 1);
  o_led     : out std_logic_vector(7 downto 0);
  -- uart1
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic;
  -- ADC samples
  i_gps_I  : in std_logic_vector(1 downto 0);
  i_gps_Q  : in std_logic_vector(1 downto 0);
  i_glo_I  : in std_logic_vector(1 downto 0);
  i_glo_Q  : in std_logic_vector(1 downto 0);
  -- rf front-end
  i_gps_ld    : in std_logic;
  i_glo_ld    : in std_logic;
  o_max_sclk  : out std_logic;
  o_max_sdata : out std_logic;
  o_max_ncs   : out std_logic_vector(1 downto 0);
  i_antext_stat   : in std_logic;
  i_antext_detect : in std_logic;
  o_antext_ena    : out std_logic;
  o_antint_contr  : out std_logic;
  i_gmiiclk_p : in    std_ulogic;
  i_gmiiclk_n : in    std_ulogic;
  o_egtx_clk  : out   std_ulogic;
  i_etx_clk   : in    std_ulogic;
  i_erx_clk   : in    std_ulogic;
  i_erxd      : in    std_logic_vector(3 downto 0);
  i_erx_dv    : in    std_ulogic;
  i_erx_er    : in    std_ulogic;
  i_erx_col   : in    std_ulogic;
  i_erx_crs   : in    std_ulogic;
  i_emdint    : in std_ulogic;
  o_etxd      : out   std_logic_vector(3 downto 0);
  o_etx_en    : out   std_ulogic;
  o_etx_er    : out   std_ulogic;
  o_emdc      : out   std_ulogic;
  io_emdio    : inout std_logic;
  o_erstn     : out   std_ulogic
);
end component;

begin


  -- Process of reading
  procReadingFile : process
    variable clk_next: std_logic;
  begin

    wait for INCR_TIME;
    if (adc_cnt + 26000000) >= 70000000 then
      adc_cnt <= (adc_cnt + 26000000) - 70000000;
      i_clk_adc <= not i_clk_adc;
    else
      adc_cnt <= (adc_cnt + 26000000);
    end if;

    while true loop
      clk_next := not clk_cur;
      if (clk_next = '1' and clk_cur = '0') then
        check_clk_bus <= '1';
      end if;

      wait for 1 ps;
      check_clk_bus <= '0';
      clk_cur <= clk_next;

      wait for INCR_TIME;
      if clk_cur = '1' then
        iClkCnt <= iClkCnt + 1;
      end if;
      if (adc_cnt + 26000000) >= 70000000 then
        adc_cnt <= (adc_cnt + 26000000) - 70000000;
        i_clk_adc <= not i_clk_adc;
      else
        adc_cnt <= (adc_cnt + 26000000);
      end if;

    end loop;
    report "Total clocks checked: " & tost(iErrCheckedCnt) & " Errors: " & tost(iErrCnt);
    wait for 1 sec;
  end process procReadingFile;


  i_sclk_p <= clk_cur;
  i_sclk_n <= not clk_cur;

  procSignal : process (i_sclk_p, iClkCnt)

  begin
    if rising_edge(i_sclk_p) then
      
      --! @note to make sync. reset  of the logic that are clocked by
      --!       htif_clk which is clock/512 by default.
      if iClkCnt = 5 then
        i_rst <= '0';
      end if;
    end if;
  end process procSignal;

  i_dip <= "101";

  -- signal parsment and assignment
  tt : rocket_soc port map
  (
    i_rst     => i_rst,
    i_sclk_p  => i_sclk_p,
    i_sclk_n  => i_sclk_n,
    i_clk_adc => i_clk_adc,
    i_int_clkrf => i_int_clkrf,
    i_dip     => i_dip,
    o_led     => o_led,
    i_uart1_ctsn => i_uart1_ctsn,
    i_uart1_rd   => i_uart1_rd,
    o_uart1_td   => o_uart1_td,
    o_uart1_rtsn => o_uart1_rtsn,
    i_gps_I  => "01",
    i_gps_Q  => "11",
    i_glo_I  => "11",
    i_glo_Q  => "01",
    i_gps_ld    => i_gps_ld,
    i_glo_ld    => i_glo_ld,
    o_max_sclk  => o_max_sclk,
    o_max_sdata => o_max_sdata,
    o_max_ncs   => o_max_ncs,
    i_antext_stat   => i_antext_stat,
    i_antext_detect => i_antext_detect,
    o_antext_ena    => o_antext_ena,
    o_antint_contr  => o_antint_contr,
    i_gmiiclk_p => '0',
    i_gmiiclk_n => '1',
    o_egtx_clk  => open,
    i_etx_clk   => '0',
    i_erx_clk   => '0',
    i_erxd      => "0000",
    i_erx_dv    => '0',
    i_erx_er    => '0',
    i_erx_col   => '0',
    i_erx_crs   => '0',
    i_emdint    => '0',
    o_etxd      => open,
    o_etx_en    => open,
    o_etx_er    => open,
    o_emdc      => o_emdc,
    io_emdio    => io_emdio,
    o_erstn     => open
 );

  procCheck : process (i_rst, check_clk_bus)
  begin
    if rising_edge(check_clk_bus) then
      if i_rst = '0' then
        iErrCheckedCnt <= iErrCheckedCnt + 1;
      end if;
    end if;
  end process procCheck;

end;
