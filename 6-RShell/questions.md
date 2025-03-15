1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client can determine when output is fully received using: Delimiters: A predefined sequence marks the end. The client reads until the delimiter is found. Length Prefixing: The server sends the output length first. The client reads until the specified bytes are received.Connection Closure: The server closes the connection after sending data, but this prevents persistent connections. To handle partial reads, the client must buffer incoming data and check against the expected length or delimiter iteratively.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

Since TCP lacks message boundaries, the protocol must enforce them using: Delimiters, use a unique terminator. Commands must not contain the delimiter unless escaped. Length Prefixing, prefix each command with its byte count. The server reads the length first, then the exact number of bytes. Without proper handling, commands may be truncated or merged, causing errors or security risks like injection attacks.

3. Describe the general differences between stateful and stateless protocols.

Stateful: Maintains client state between requests. Requires server resources to track sessions, complicating scalability and fault tolerance.

Stateless: Each request is independent. Simplifies server design, improves scalability, but may require clients to re-transmit data.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

UDP is preferred when low latency is critical, small, frequent messages are sent, broadcast/multicast is needed, and application-layer reliability is sufficient.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The socket API is the primary OS abstraction for network communication.