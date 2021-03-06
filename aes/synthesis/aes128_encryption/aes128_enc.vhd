library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity aes128_enc is
    port (
        key0i : in  std_logic_vector(31 downto 0);
        key1i : in  std_logic_vector(31 downto 0);
        key2i : in  std_logic_vector(31 downto 0);
        key3i : in  std_logic_vector(31 downto 0);
        datai : in  std_logic_vector(127 downto 0);

        datao : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes128_enc is
    signal sboxo  : std_logic_vector(127 downto 0);
    signal shifto : std_logic_vector(127 downto 0);
    signal mixo   : std_logic_vector(127 downto 0);

    component aes128_sbox
        port (
            datai : in  std_logic_vector(7 downto 0);
            datao : out std_logic_vector(7 downto 0)
        );
    end component aes128_sbox;

    component aes128_shift
        port (
            datai : in  std_logic_vector(127 downto 0);
            datao : out std_logic_vector(127 downto 0)
        );
    end component aes128_shift;

    component aes128_mix
        port (
            datai : in  std_logic_vector(127 downto 0);
            datao : out std_logic_vector(127 downto 0)
        );
    end component aes128_mix;
begin
    sbox_generate : for i in 0 to 15 generate
        sbox0 : aes128_sbox
            port map (
                datai => datai(8*i+7 downto 8*i),
                datao => sboxo(8*i+7 downto 8*i)
            );
    end generate;

    shift0 : aes128_shift
        port map (
            datai => sboxo,
            datao => shifto
        );

    mix0 : aes128_mix
        port map (
            datai => shifto,
            datao => mixo
        );

    datao <= mixo xor (key0i & key1i & key2i & key3i);
end architecture rtl;
