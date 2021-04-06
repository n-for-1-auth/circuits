library ieee;
use ieee.std_logic_1164.all;
use work.gcm_def.all;

entity gcm_mult is
    port (
        tt			    : in 	std_logic_vector(16383 downto 0);
        cur 		    : in 	std_logic_vector(127 downto 0);
		newcur		    : out 	std_logic_vector(127 downto 0);
    );
end entity;

architecture rtl of gcm_mult is
	signal middle_result: tt_type;
begin
	initial_result : for i in 127 downto 0 generate
		middle_result(0)(i) <= tt(i) and cur(0);
	end generate;

    result_computation : for i in 1 to 127 generate
		result_per_step : for j in 127 downto 0 generate
			middle_result(i)(j) <= middle_result(i-1)(j) xor (tt(i * 128 + j) and cur(i));
		end generate;
    end generate;
	
	newcur(127 downto 0) <= middle_result(127)(127 downto 0);
end architecture rtl;
