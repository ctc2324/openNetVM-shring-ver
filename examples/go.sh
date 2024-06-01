#!/bin/bash

# The go.sh script is a convenient way to run start_nf.sh without specifying NF_NAME

NF_DIR=${PWD##*/}

if [ ! -f ../start_nf.sh ]; then
  echo "ERROR: The ./go.sh script can only be used from the NF folder"
  echo "If running from another directory use examples/start_nf.sh"
  exit 1
fi

# only check for running manager if not in Docker
if [[ -z $(pgrep -u root -f "/onvm/onvm_mgr/.*/onvm_mgr") ]] && ! grep -q "docker" /proc/1/cgroup; then
    echo "NF cannot start without a running manager"
    exit 1
fi

# Initialize additional arguments
ADDITIONAL_ARGS=""

# Process arguments
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        -s)
            shift
            while [[ "$#" -gt 1 && "$1" != -* ]]; do
                IP=$1
                NF=$2
                ADDITIONAL_ARGS="$ADDITIONAL_ARGS -sh $IP $NF"
                shift 2
            done
            ;;
        *)
            ADDITIONAL_ARGS="$ADDITIONAL_ARGS $1"
            shift
            ;;
    esac
done

# # Check if destination argument is provided
# if ! echo "$ADDITIONAL_ARGS" | grep -q "\-d"; then
#     echo "ERROR: Destination flag -d is required."
#     exit 1
# fi

# Run start_nf.sh with the provided arguments
../start_nf.sh "$NF_DIR" $ADDITIONAL_ARGS
