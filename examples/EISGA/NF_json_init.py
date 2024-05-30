import json

vnf_mapping = {
    0: "ndpi_stats",
    1: "flow_tracker",
    2: "simple_forward",
    3: "basic_monitor",
    4: "l3fwd"
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

for line in lines:
    line = line.strip()
    parts = line.split(",")
    
    node_count = int(parts[0])
    vnf_id = int(parts[1])
    
    if vnf_id == -1:  # Skip lines with vnf_id = -1
        continue
    
    head = int(parts[2])
    next_nodes = parts[3:]  # Get all parts after the third part as next nodes

    vnf_name = vnf_mapping.get(vnf_id, f"vnf_{vnf_id}")

    param_first = node_count

    if len(next_nodes) == 1:
        parameters = f"{param_first} -d {next_nodes[0]}"
    elif len(next_nodes) > 1:
        parameters = f"{param_first} -s {' '.join(map(str, next_nodes))}"
    else:
        parameters = str(param_first)
    
    # Append to the corresponding VNF list
    vnf_lists[vnf_name].append({"parameters": parameters})

# Convert VNF lists to JSON format
for vnf_name, vnf_list in vnf_lists.items():
    output_json[vnf_name] = vnf_list

# Write the JSON to file
with open("network.json", "w") as json_file:
    json.dump(output_json, json_file, indent=4)

print("JSON doc generated: network.json")
