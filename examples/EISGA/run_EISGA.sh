start_time=$(date +%s)

echo "Compiling and running generate_VNF.cpp..."
# ./generate_VNF

echo "Compiling and running EISGA.cpp..."
./EISGA

echo "Transmit node into to network.json"
python3 NF_json_init.py

cd ..

echo "open all NF"
python3 run_group.py EISGA/network.json

end_time=$(date +%s)
elapsed_time=$((end_time - start_time))


echo "Total time: $elapsed_time seconds" > time.txt


echo "Total time: $elapsed_time seconds"