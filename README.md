# IPK Project 1: IPK Calculator Protocol Documentation
Name and surname: Roman Poliačik\
Login: xpolia05

## Purpose

IPKCPC is a client application that enables communication with a server over a network using either TCP or UDP following IPK Calculator protocol(__IPKCP__). The purpose of this application is to send messages to a server and receive responses back from it.

## Dependencies

- `GCC Compiler`

The source code depends on the following C libraries:

- `stdio.h` : For standard input and output functions.
- `stdlib.h` : For memory allocation and process control functions.
- `unistd.h` : For standard symbolic constants and types.
- `string.h` : For string manipulation functions.
- `arpa/inet.h` : For definitions of internet operations.
- `sys/socket.h` : For socket definitions.
- `sys/time.h` : For time measurement and manipulation.
- `netinet/in.h` : For internet protocol family definitions.
- `getopt.h` : For command line option parsing.
- `netdb.h` : For definitions for network database operations.
- `iostream` : For input and output operations in C++.
- `signal.h` : For signal handling.

## Build and Execution 
To build the source code, run one of the following commands:

__`g++ ipkcpc.cpp ipkcpc_functions.cpp -o ipkcpc`__\
or using make:\
__`make`__

To execute the client application, run the following command:

__`./ipkcpc -h <host> -p <port> -m <mode>`__

- `-h <host>`: Specifies the IPv4 address.
- `-p <port>`: Specifies the port number for the connection.
- `-m <mode>`: specifies the communication protocol (`tcp` or `udp`)
## Functions

The source code consists of the following functions:

### `sigint_handler(int sig)`

This function is a signal handler that is executed when the user presses `CTRL+C`and ensures gracefully closing connection between client and the server before exiting the program. 

If the mode is TCP(Textual variant of IPKCP), it sends a BYE message(FIN packet) to the server, waits for the response(ACK packet), and closes the connection if the response is correct using `exit_tcp_gracefully()`. 

If the mode is UDP(Binary variant of IPKCP), it simply closes the socket to release taken resources. Because UDP is connectionless protocol, it does not need to perform three-way handshake to terminate connection.

### `parameter_check(int argc, char* argv[], char **host, char **port, char **mode)`

This function checks the command line arguments passed to the client application using `getopt()`.
If any of these arguments are missing or invalid, the function prints an error message and exits with a status of `EXIT_FAILURE`.
Validity of arguments consists of checking port if it's in range 1-65535, mode that can only be tcp or udp. As for correctness of IPv4 address, this function uses the `inet_aton()` function from the standard C library to convert an IPv4 address in dotted-decimal notation stored in `char *host` into a binary format. If the conversion fails, it means that the host address is not a valid IPv4 address.

By using `inet_aton()` the client supports multiple notations of IPv4 addresses such as:
- Classic notation:     __192.168.0.1__
- Zero-filled notation: __192.168.000.001__
- Abbreviated notation: __192.168.1__
- Hexadecimal notation: __0xC0.0xA8.0x00.0x01__
- Octal notation:       __0300.0250.0.1__

### `addr_soc(char *host, char *port, struct hostent **server, struct sockaddr_in *server_address);`
addr_soc takes care of obtaining the IP address of the specified host using DNS, initializes the server address structure, and creates a TCP or UDP socket based on the mode set by the `parameter_check()`. It also sets the client socket send and receive timeouts using `setsockopt()`.

### `connect_udp(char **host,char **port)`

This function obtains the IP address of the server specified by hostname, and then creates a UDP socket with timeouts using the `addr_soc()` function. 
Then the function reads messages from the user using `fgets()` and sends them to the server. When a message is received from the user, the function prepares a request message; sets first element of the array to `0x00` representing request OPCODE and second element to length of the payload data, then uses `memcpy()` to copy buffer of the user message to rest of the array. Function then sends it to the server, and waits for a response. If the server responds correctly in interval of 2 seconds, the function prints the response message based on status code to the console. It also uses the `sigint_handler()` function as a signal handler for `CTRL+C` events. If any of send or receive fails or timeouts, the function prints an error and prompts user to try again.

### `connect_tcp(char **host,char **port)`

This function obtains the IP address of the server specified by hostname, and then creates a UDP socket with timeouts using the `addr_soc()` function. 
The function then establishes a connection to the server using the `connect()`.
After successful connection, it sets up a signal handler `sigint_handler()`. The function then enters an infinite loop in which it waits for user input from the command line using `fgets()`, sends the user's message to the server using `send()`, waits for a response from the server using `recv()`, and then prints the received message to the console using `printf()`. If the received message from server is `BYE\n`, the function closes the client socket and exits successfully using close and exit system calls, respectively. If any of send or receive fails or timeouts, the function prints an error and prompts user to try again.

### `main(int argc, char* argv[])`
The main function validates input parameters, and runs appropriate connection based on mode specified.

## Testing

For the purpose of testing, a shell script is available to test multiple cases of client behaviour on either mode.

There are two test scripts available `tests_nix.sh` and `tests.sh` with the difference that tests_nix.sh is using special nix shebang for testing in nixOS enviroment - the same enviroment that reference server __IPKPD__ uses.

For each test case, the script runs the __IPKCPC__ program with the specified host, port, and mode, passes it the input file as stdin, and then compares the output from the program to the expected output file. The script is also capable of testing `CTRL+C` or `C-c` behaviour of the program with test cases ending in `sigint_input/output.txt`. This capability is more suitable for testing tcp connection `C-c` behaviour, as all tests on udp mode already automatically end program with `C-c`.

All test cases can be found in the __/tests__ folder.

### Execution

To execute the test script, run the following command:

__`./tests_nix.sh [OPTIONS]`__

or

__`./tests.sh [OPTIONS]`__

You can provide the following options to the script:
- `--show-io`: Show input and output for each passed test. The default value is false.
- `-h <host>`:Set the host IPv4 address. The default value is 0.0.0.0.
- `-p <port>`: Set the port number. The default value is 2023.
- `-m <mode>`: Set the IPKC protocol mode (tcp or udp). The default value is tcp.

The script relies on that it will connect the client to working and correctly implemented IPKPD server and that there is existing successfully compiled IPKCPC binary file and tests folder in the same directory as script!

#### nixOS test setup

in case of problems with running `tests_nix.sh` on nixOS, try these commands before running the script(be sure that you are in the same working directory as client source files):

```
nix-shell -p gcc
g++ ipkcpc.cpp ipkcpc_functions.cpp -o ipkcpc // OR `make` if makefile is present
./tests_nix.sh
```

The reason is that there is possibility that nixOS cannot read the binary file `ipkcpc` if it's been compiled on different system.

### Testing timeout

Timeout behaviour had to be tested manually, first local server process is started, then client sends initial message to the server, the server process is then killed and client tries to send and receive message from server again.

![timeout testing](/images/timeout.png)

## BIBLIOGRAPHY

- Nishtha Thakur How do I catch a Ctrl+C event in C++? [online]. Publisher: Tutorialspoint, Jul 30, 2019, from https://www.tutorialspoint.com/how-do-i-catch-a-ctrlplusc-event-in-cplusplus

- Setting timeout for recv() function of a UDP socket [online]. Publisher: Stack Overflow, Apr 23, 2009 [cit. 2023-03-21], from https://stackoverflow.com/questions/16163260/setting-timeout-for-recv-fcn-of-a-udp-socket

- Vladimir Vesely DemoTcp [online]. Publisher: Brno University of Technology, Faculty of Information Technology, January 30th 2023. [cit. 2023-03-21]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoTcp. 

- Vladimir Vesely DemoUdp [online]. Publisher: Brno University of Technology, Faculty of Information Technology, January 30th 2023. [cit. 2023-03-21]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp.

- setsockopt() - set options on sockets [online]. Publisher: The Open Group, 2017 [cit. 2023-03-21]. Available at: https://man7.org/linux/man-pages/man3/setsockopt.3p.html

- inet_aton - convert IPv4 address from text to binary form [online]. Publisher: The Open Group, Mar 22, 2021 [cit. 2023-03-21]. Available at: https://man7.org/linux/man-pages/man3/inet_aton.3.html

- How to catch SIGINT within a Bash subshell? [online]. Publisher: Stack Overflow, Jan 6, 2022 [cit. 2023-03-21]. Available at: https://stackoverflow.com/questions/70609841/how-to-catch-sigint-within-a-bash-subshell
