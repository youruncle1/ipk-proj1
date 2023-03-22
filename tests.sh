#!/bin/bash

green='\033[1;32m'
red='\033[0;31m'
lred='\033[1;31m'
yellow='\033[33m'
gray='\033[0;37m'
reset='\033[0m'

show_passed=false
host=0.0.0.0
port=2023
mode=tcp
test_cases_dir="tests/tcp"

while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --show-io)
        show_passed=true
        shift
        ;;
        -h)
        host="$2"
        shift
        shift
        ;;
        -p)
        port="$2"
        shift
        shift
        ;;
        -m)
        mode="$2"
        shift
        shift
        ;;
        --help)
        echo "Usage: tests.sh [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --show-io          show input and output for each passed test (default: false)"
        echo "  -h <host>          set the host IPv4 address (default: 0.0.0.0)"
        echo "  -p <port>          set the port number (default: 2023)"
        echo "  -m <mode>          set the IPKC protocol mode (tcp or udp, default: tcp)"
        exit 0
        ;;
        *)
        echo "Unknown argument: $key"
        exit 1
        ;;
    esac
done

if [[ $mode == "tcp" ]]; then
    test_cases_dir="tests/tcp"
elif [[ $mode == "udp" ]]; then
    test_cases_dir="tests/udp"
else
    echo "Unknown mode: $mode"
    exit 1
fi


for test_case_input_file in $(ls $test_cases_dir/*_input.txt); do

    test_case_name=$(basename $test_case_input_file _input.txt)

    input=$(cat $test_case_input_file)
    expected_output=$(cat $test_cases_dir/${test_case_name}_output.txt)

    if [[ $test_case_input_file == *"sigint_input.txt" || $mode == "udp" ]]; then
        actual_output=$(timeout 5s sh -c "echo \"$input\" | ./ipkcpc -h $host -p $port -m $mode & pid=\$!; echo \"$input\" > /dev/null; sleep 1; kill -INT \$pid >/dev/null 2>&1; wait \$pid; exit \$?" | grep -v '^$') || true
    else
        actual_output=$(echo "$input" | ./ipkcpc -h $host -p $port -m $mode)
    fi

    if [ "$actual_output" != "$expected_output" ]; then
        echo -e "${lred}Test $test_case_name: failed${reset}"
        echo -e "${red}Input:${reset}"
        echo -e "${gray}$input${reset}"
        echo -e "${red}Expected Output:${reset}"
        echo -e "${gray}$expected_output${reset}"
        echo -e "${red}Actual Output:${reset}"
        echo -e "${gray}$actual_output${reset}"
    else
        echo -e "${green}Test $test_case_name: passed${reset}"
        if [ "$show_passed" = true ]; then
            echo -e "${yellow}Input:${reset}"
            echo -e "${gray}$input${reset}"
            echo -e "${yellow}Output:${reset}"
            echo -e "${gray}$actual_output${reset}\n"
        fi
    fi
done