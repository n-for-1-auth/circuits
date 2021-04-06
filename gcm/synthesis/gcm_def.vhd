library ieee;
use ieee.std_logic_1164.all;

package gcm_def is
	type tt_type is array(127 downto 0) of std_logic_vector(127 downto 0);
	type big_tt_type is array(19 downto 0) of std_logic_vector(127 downto 0);
end package;
