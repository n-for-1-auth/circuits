library ieee;
use ieee.std_logic_1164.all;

entity aes128_key_schedule is
    port (
        key        : in  std_logic_vector(127 downto 0);
        round_keys : out std_logic_vector(1407 downto 0)
    );
	
	-- by Jonathan Bromley: https://groups.google.com/g/comp.lang.vhdl/c/eBZQXrw2Ngk/m/4H7oL8hdHMcJ 
	function reverse_any_vector (a: in std_logic_vector)
	return std_logic_vector is
	  variable result: std_logic_vector(a'RANGE);
	  alias aa: std_logic_vector(a'REVERSE_RANGE) is a;
	begin
	  for i in aa'RANGE loop
		result(i) := aa(i);
	  end loop;
	  return result;
	end; -- function reverse_any_vector
end entity;

architecture rtl of aes128_key_schedule is
	type key_type is array(43 downto 0) of std_logic_vector(31 downto 0);
    signal keys   : key_type;

	component aes128_key is
        generic (
            word_ctr : integer
        );
        port (
            key1i : in  std_logic_vector(31 downto 0);
            keyki : in  std_logic_vector(31 downto 0);
            keyo  : out std_logic_vector(31 downto 0)
        );
    end component aes128_key;
begin
	key_initialize_generate : for i in 0 to 3 generate
        keys_process : process(key) is
            variable left  : integer;
            variable right : integer;
        begin
            left    := 128 - 32 * i - 1;
            right   := left - 31;

            keys(i) <= key(left downto right);
        end process keys_process;
	end generate;

    key_generate : for i in 4 to 43 generate
        key0 : aes128_key
            generic map (
                word_ctr => i
            )
            port map (
                key1i => keys(i-1),
                keyki => keys(i-4),
                keyo  => keys(i)
            );
    end generate;
	
	key_output_generate: for i in 0 to 43 generate
		key_output: process(keys) is
        begin
			round_keys((32 * i + 7) downto 32 * i) <= reverse_any_vector(keys(i)(7 downto 0));
			round_keys((32 * i + 15) downto (32 * i + 8)) <= reverse_any_vector(keys(i)(15 downto 8));
			round_keys((32 * i + 23) downto (32 * i + 16)) <= reverse_any_vector(keys(i)(23 downto 16));
			round_keys((32 * i + 31) downto (32 * i + 24)) <= reverse_any_vector(keys(i)(31 downto 24));
		end process key_output;
	end generate;
end architecture rtl;
