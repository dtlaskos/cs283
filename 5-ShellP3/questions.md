1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

The shell tracks all child process IDs and uses waitpid() in sequence to ensure each completes before accepting new input. Forgetting waitpid() would leave zombie processes, risk resource leaks, and allow race conditions if subsequent commands depend on prior pipeline output.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Unused pipe ends must be closed after dup2() to prevent file descriptor exhaustion and ensure proper EOF signaling. Leaving them open could stall processes waiting for input that never arrives or leak resources system-wide.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in because external processes cannot modify the parent shell's environment. If run externally, directory changes would only affect the child process, leaving the shell's working directory unchanged.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support unlimited commands, replace fixed arrays with dynamically allocated structures that grow as needed. Trade-offs include increased memory management complexity versus flexibility, balancing safety (fixed bounds) with real-world usability (arbitrary pipelines).