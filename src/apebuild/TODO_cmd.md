# Command Execution Module TODO

Module for building, executing, and managing shell commands with support for synchronous and asynchronous execution.

## Core Data Structures

- [ ] ApeCmd - Command structure (dynamic array of arguments)
- [ ] ApeCmdList - List of commands
- [ ] ApeProcHandle - Opaque handle for tracking async processes (NOT a pointer)
- [ ] ApeProcStatus - Process status enum (running, completed, failed, etc.)
- [ ] ApeProcResult - Result structure (exit code, signal, etc.)

## Command Building

- [ ] ape_cmd_new - Create new empty command
- [ ] ape_cmd_from - Create command from program name
- [ ] ape_cmd_append - Append single argument to command
- [ ] ape_cmd_append_many - Append multiple arguments (variadic)
- [ ] ape_cmd_append_list - Append arguments from string list
- [ ] ape_cmd_prepend - Prepend argument to command
- [ ] ape_cmd_clone - Clone/duplicate a command
- [ ] ape_cmd_free - Free command resources
- [ ] ape_cmd_clear - Clear command (reuse without freeing)

## Command Display/Debugging

- [ ] ape_cmd_render - Render command to string (for display)
- [ ] ape_cmd_render_quoted - Render with proper shell quoting
- [ ] ape_cmd_print - Print command to stderr
- [ ] ape_cmd_log - Log command with prefix (CMD:, etc.)

## Synchronous Execution

- [ ] ape_cmd_run - Run command synchronously, return success/failure
- [ ] ape_cmd_run_status - Run command, return exit status
- [ ] ape_cmd_run_capture - Run command, capture stdout to buffer
- [ ] ape_cmd_run_capture_all - Run command, capture stdout and stderr
- [ ] ape_cmds_run_seq - Run list of commands sequentially (stop on failure)
- [ ] ape_cmds_run_all - Run list of commands sequentially (continue on failure)

## Asynchronous Execution

- [ ] ape_cmd_start - Start command asynchronously, return handle
- [ ] ape_proc_poll - Check if process is still running (non-blocking)
- [ ] ape_proc_wait - Wait for process to complete (blocking)
- [ ] ape_proc_wait_timeout - Wait with timeout
- [ ] ape_proc_status - Get current status of process
- [ ] ape_proc_result - Get result of completed process
- [ ] ape_proc_kill - Kill/terminate process
- [ ] ape_proc_signal - Send signal to process

## Parallel Execution

- [ ] ApeProcPool - Pool of running processes
- [ ] ape_pool_new - Create process pool with max parallelism
- [ ] ape_pool_submit - Submit command to pool
- [ ] ape_pool_wait_any - Wait for any process to complete
- [ ] ape_pool_wait_all - Wait for all processes to complete
- [ ] ape_pool_free - Free pool resources
- [ ] ape_cmds_run_parallel - Run list of commands in parallel (convenience)

## Handle Management

- [ ] APE_INVALID_HANDLE - Invalid handle sentinel
- [ ] ape_proc_handle_valid - Check if handle is valid
- [ ] ape_proc_handle_release - Release handle (cleanup tracking)

## Environment

- [ ] ape_cmd_set_env - Set environment variable for command
- [ ] ape_cmd_clear_env - Clear environment for command
- [ ] ape_cmd_set_cwd - Set working directory for command

## Piping/Redirection

- [ ] ape_cmd_pipe - Pipe output of one command to input of another
- [ ] ape_cmd_redirect_stdout - Redirect stdout to file
- [ ] ape_cmd_redirect_stderr - Redirect stderr to file
- [ ] ape_cmd_redirect_stdin - Redirect stdin from file
