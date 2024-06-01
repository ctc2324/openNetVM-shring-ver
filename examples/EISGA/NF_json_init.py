import json

vnf_mapping = {
    0: "aes_decrypt",
    1: "flow_tracker",
    2: "l3fwd",
    3: "simple_forward",
    4: "basic_monitor"
}

with open("network.txt", "r") as file:
    lines = file.readlines()

output_json = {
    "globals": [
        {
            "directory": "SFC/log/sfc2_output_files"
        }
    ]
}

# Create separate lists for each VNF
vnf_lists = {vnf_name: [] for vnf_name in vnf_mapping.values()}

special_vnf_count = 0

for line in lines:
    line = line.strip()
    parts = line.split(",")
    
    node_count = int(parts[0])
    vnf_id = int(parts[1])
    
    if vnf_id == -1:  # Handle special cases for vnf_id = -1
        special_vnf_count += 1
        if special_vnf_count == 1:
            vnf_name = "simple_forward"
            next_nodes = parts[2:]
            if len(next_nodes) == 1:
                parameters = f"{1} -d {next_nodes[0]}"
            elif len(next_nodes) > 1:
                parameters = f"{1} -s {' '.join(map(str, next_nodes))}"
            else:
                parameters = str(param_first)
        elif special_vnf_count == 2:
            vnf_name = "l3fwd"
            param_first = node_count
            parameters = f"{param_first}"
        else:
            continue  # Skip further -1 cases
    else:
        vnf_name = vnf_mapping.get(vnf_id, f"vnf_{vnf_id}")
        next_nodes = parts[2:]  # Get all parts after the second part as next nodes

        param_first = node_count

        if len(next_nodes) == 1:
            parameters = f"{param_first} -d {next_nodes[0][14]}"
        elif len(next_nodes) > 1:
            parameters = f"{param_first} -s {' '.join(map(str, next_nodes))}"
        else:
            parameters = str(param_first)
    
    # Append to the corresponding VNF list
    vnf_lists[vnf_name].append({"parameters": parameters})

# Convert VNF lists to JSON format, but skip empty lists
for vnf_name, vnf_list in vnf_lists.items():
    if vnf_list:  # Only add non-empty lists
        output_json[vnf_name] = vnf_list

# Write the JSON to file
with open("network.json", "w") as json_file:
    json.dump(output_json, json_file, indent=4)

print("JSON doc generated: network.json")
