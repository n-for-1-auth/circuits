import sys
import re

if len(sys.argv) < 2:
    sys.exit("Usage: python clean.py [netlist file name]")

filename = sys.argv[1]

file = open(filename, mode='r')
netlist = file.read()
file.close()

netlist = netlist.strip()

netlist_array = netlist.split(";")

new_netlist = ""

for netlist_elem in netlist_array:
    new_netlist_elem = netlist_elem.strip().replace('\n', '').replace('\r', '')
    new_netlist_elem = re.sub('\s+', ' ', new_netlist_elem)

    if (new_netlist_elem.find(".Z()") == -1) and (new_netlist_elem != ""):
        new_netlist += new_netlist_elem + ";\n"

file = open(filename, mode='w')
file.write(new_netlist)
file.close()