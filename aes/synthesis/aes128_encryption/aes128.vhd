library ieee;
use ieee.std_logic_1164.all;

entity aes128 is
    port (
        round_keys      : in  std_logic_vector(1407 downto 0);
        plaintext		: in  std_logic_vector(127 downto 0);
        ciphertext 		: out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes128 is
	type key_array is array(43 downto 0) of std_logic_vector(31 downto 0);
	type ct_array is array(13 downto 0) of std_logic_vector(127 downto 0);
    signal keys   : key_array;
    signal cts    : ct_array;

    signal sboxo  : std_logic_vector(127 downto 0);
    signal shifto : std_logic_vector(127 downto 0);
	
	signal ciphertext_inversed : std_logic_vector(127 downto 0);

    component aes128_enc is
        port (
            key0i : in  std_logic_vector(31 downto 0);
            key1i : in  std_logic_vector(31 downto 0);
            key2i : in  std_logic_vector(31 downto 0);
            key3i : in  std_logic_vector(31 downto 0);
            datai : in  std_logic_vector(127 downto 0);

            datao : out std_logic_vector(127 downto 0)
        );
    end component aes128_enc;

    component aes128_sbox is
        port (
            datai : in  std_logic_vector(7 downto 0);
            datao : out std_logic_vector(7 downto 0)
        );
    end component aes128_sbox;

    component aes128_shift is
        port (
            datai : in  std_logic_vector(127 downto 0);
            datao : out std_logic_vector(127 downto 0)
        );
    end component aes128_shift;
	
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
begin
	key_input_generate: for i in 0 to 43 generate
		key_input: process(keys) is
        begin
			keys(i)(7 downto 0) <= reverse_any_vector(round_keys((32 * i + 7) downto 32 * i));
			keys(i)(15 downto 8) <= reverse_any_vector(round_keys((32 * i + 15) downto (32 * i + 8)));
			keys(i)(23 downto 16) <= reverse_any_vector(round_keys((32 * i + 23) downto (32 * i + 16)));
			keys(i)(31 downto 24) <= reverse_any_vector(round_keys((32 * i + 31) downto (32 * i + 24)));
		end process key_input;
	end generate;

    enc_generate : for i in 1 to 9 generate
        enc0 : aes128_enc
            port map (
                key0i => keys(4*i),
                key1i => keys(4*i+1),
                key2i => keys(4*i+2),
                key3i => keys(4*i+3),
                datai => cts(i-1),

                datao => cts(i)
            );
    end generate;

    sbox_generate : for i in 0 to 15 generate
        sbox0 : aes128_sbox
            port map (
                datai => cts(9)(8*i+7 downto 8*i),
                datao => sboxo(8*i+7 downto 8*i)
            );
    end generate;

    shift0 : aes128_shift
        port map (
            datai => sboxo,
            datao => shifto
        );

    enc_process : process(keys, plaintext) is
    begin
        cts(0) <= plaintext xor (keys(0) & keys(1) & keys(2) & keys(3));
    end process enc_process;

    ciphertext_inversed <= shifto xor (keys(40) & keys(41) & keys(42) & keys(43));
	
	ciphertext(127 downto 0) <= reverse_any_vector(ciphertext_inversed(127 downto 0));
end architecture rtl;
