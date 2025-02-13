1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice because it reads input line by line, which matches how shells process commands. It also prevents buffer overflow by limiting input size to ARG_MAX, handles EOF cleanly by returning NULL, and preserves newline characters for input validation.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  While the solution uses a fixed array (char cmd_buff[SH_CMD_MAX]), malloc() would be necessary if we needed to handle dynamic input sizes larger than SH_CMD_MAX, implement multi-line command support, or enable shell scripting with variable-length inputs. The fixed array was chosen for simplicity and to match the problem constraints.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming spaces is necessary because untrimmed spaces can lead to invalid commands (e.g., " ls " being interpreted as a command with spaces), empty commands (e.g., echo " "), pipe errors (e.g., cmd1 | cmd2), and argument corruption (e.g., echo "test" being split into multiple arguments).

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: Three essential redirections to implement are: cmd > file (redirect stdout to a file), which requires handling file creation and truncation. cmd 2> err.log (redirect stderr to a file), which involves separating output streams. cmd < input.txt (redirect stdin from a file), which requires substituting the input stream. Challenges include preserving file descriptors across fork() and exec(), handling concurrent input/output redirections, and managing appending (>>) versus overwriting (>).

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Piping connects processes using anonymous pipes (e.g., cmd1 | cmd2), while redirection connects a process to a file (e.g., cmd > file). Piping enables bidirectional data flow between processes, while redirection is unidirectional. Piping requires concurrent execution, whereas redirection can work with a single process.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  Keeping STDERR and STDOUT separate is important because it isolates error messages from regular output, allows selective processing (e.g., piping stdout while logging stderr), preserves error context for debugging, and ensures compliance with POSIX standards.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  The custom shell should handle errors by: Separating streams by default (stderr to console, stdout to pipeline). Providing a merge option via 2>&1 syntax to combine stderr and stdout. Preserving errors by buffering stderr during execution, displaying it after command completion, and returning nonzero exit codes for failed commands.