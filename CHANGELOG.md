## Changelog
chronologically, only including important commits...

|   hash  |                                      commit message                                       |
| --- | --- |
| 103c3ef | added implementation of TCP connection with sigint handler                                |
| 9b68623 | updated tcp signal hander to wait for and print respose message before closing connection |
| 12b880a | added IPv4 checker into argument parser                                                   |
| 7961b16 | added UDP implementation, cmmnts and few minor changes                                     |
| 5f79b67 | added makefile, fixed udp connection, few edgecases and compiler warnings.                 |
| fe4d0da | changed IPv4 check from inet_pton() to inet_aton()                                          |
| 6ce4c56 | full project restructulization, few changes to connect_udp(), new files and new Makefile    |
| c98ce34 | changed fgets() behaviour                                                                   |
| 2b54948 | added test scripts for nixos/unix and test cases                                            |
| 8aa16f3 | reworked tests                                                                              |
| 74ac4ef | created new function addr_soc(), added timeouts to tcp and udp                              |
| dc82adb | fixed client behaviour when fgets fails/eof                                                 |
| 5b1d340 | fixed udp ERROR: to ERR:                                                                   |

## Known limitations: 
- This program is for UNIX-like systems only, as the source code uses headers which are part of POSIX standard.