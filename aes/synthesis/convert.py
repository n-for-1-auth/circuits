# Based on Nigel Smart's description in https://homes.esat.kuleuven.be/~nsmart/MPC/

import sys

if len(sys.argv) < 2:
    sys.exit("Usage: python convert.py [circuit file name]")

fp = open(sys.argv[1], 'r')
(num_gates, num_wires) = fp.readline().split(' ', 2)
num_gates = int(num_gates)
num_wires = int(num_wires)
input_line = fp.readline()
output_line = fp.readline()
gates = fp.readlines()
fp.close()

# obtain the input array
input_line_array = input_line.strip().split(' ')
if int(input_line_array[0]) >= 3:
    sys.exit("Usage: The maximal number of input line elemenets is 2")

# compute the number of output gates (will need to be relocated)
num_output_gates = 0
output_line_array = output_line.strip().split(' ')
for i in range(1, len(output_line_array)):
    num_output_gates += int(output_line_array[i])

# first output gate's index
first_output_gate_index = num_gates - num_output_gates + 1

# number of EQW gates
num_eqw_gates = 0
for gate in gates:
    if gate.strip() == "":
        continue
    gate_info = gate.strip().split(' ')
    if len(gate_info) > 0:
        if gate_info[len(gate_info) - 1] == "EQW":
            num_eqw_gates += 1

new_gates_text = ""

def adjust(gate_idx):
    if int(gate_idx) >= first_output_gate_index:
        if num_eqw_gates > 0:
            return str(int(gate_idx) + 1)
        else:
            return gate_idx
    else:
        return gate_idx

new_gate_index = first_output_gate_index
zero_gate_index = new_gate_index
if num_eqw_gates > 0:
    new_gates_text += "2 1 1 1 " + str(new_gate_index) + " XOR\n"
    new_gate_index += 1

for gate in gates:
    if gate.strip() == "":
        continue
    gate_info = gate.strip().split(' ')
    if len(gate_info) > 0:
        if gate_info[len(gate_info) - 1] == "EQW":
            new_gates_text += "2 1 " + adjust(gate_info[2]) + " " + str(zero_gate_index) + " " + adjust(gate_info[3]) + " XOR\n"
        else:
            if gate_info[len(gate_info) - 1] == "INV":
                new_gates_text += "1 1 " + adjust(gate_info[2]) + " " +  adjust(gate_info[3]) + " INV\n"
            else:
                # AND or XOR
                new_gates_text += "2 1 " + adjust(gate_info[2]) + " " + adjust(gate_info[3]) + " " + adjust(gate_info[4]) + " " +  gate_info[5] + "\n"

# use the old Bristol format
fp = open(sys.argv[1], 'w')
if num_eqw_gates > 0:
    fp.write(str(num_gates + 1) + " " + str(num_wires + 1) + "\n")
else:
    fp.write(str(num_gates) + " " + str(num_wires) + "\n")

if input_line_array[0] == '2':
    fp.write(input_line_array[1] + " " + input_line_array[2] + " " + str(num_output_gates) + "\n")
else:
    if input_line_array[0] == '1':
        fp.write(input_line_array[1] + " 0 " + str(num_output_gates) + "\n")
fp.write("\n")
fp.write(new_gates_text)
fp.write("\n")
fp.close()
