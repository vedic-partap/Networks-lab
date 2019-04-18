# Networks-lab
Assignments ||  Network Lab, IIT Khargpur || 2018-2019 

Prof. : [Sandip Chakraborty](http://cse.iitkgp.ac.in/~sandipc/) , [Arobinda Gupta](http://cse.iitkgp.ac.in/~agupta/)

Assignments:

1. **UDP Socket**: Get familiar with datagram sockets using POSIX C programming. The target is to establish a communication between two computers (processes) using datagram socket. A datagram socket uses a simple communication paradigm to transfer short messages between two computers (processes) without ensuring any reliability

2. **TCP Socket**: Get familiar with stream sockets (also called TCP sockets) using POSIX C programming. A stream socket establishes a connection between the client and server, which remains there until one of them closes it (explicitly or implicitly on exit). The connection can be used to transfer ordered sequence of bytes between two computers (processes) reliably

3. **Concurrent TCP Server**: Implement a concurrent server where multiple clients can
requests for same or different services and the server serves them concurrently. The
implementation will help you to understand the functionality of the ​ select() system call used
for servicing multiple requests over different sockets

4. **Simplified File Transfer Protocol (FTP)**: Implement a simplified version of the file transfer protocol. The
subset of the commands that you have to implement is small, FTP has a much richer
functionality.

5. **File Transfer in Blocks**: Transfer files between two hosts with block based
transfer. We’ll learn a special flag in `recv()` call named `MSG_WAITALL`.

6. **Non-Blocking I/O**: Implement a concurrent server where multiple clients can
requests for same or different services and the server serves them concurrently with the help of
nonblocking I/O operations.

7. **Reliable Communication over Unreliable Channel**: Build support for reliable communication over an
unreliable link. The unreliable link will be implemented with a UDP socket.

8. **Traceroute**: Implement `mytraceroute` -- your version of the Linux `traceroute` tool for identifying the number of layer
3 (IP layer) hops from your machine to a given destination.

9. **Signal Driven I/O**: Simple UDP echo server using asynchronous, non-blocking I/O.

For any queries, feel free to ping [Vedic Partap](vedicpartap1999@gmail.com)
