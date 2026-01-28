# Argument Parsing Module TODO

Module for parsing command-line arguments with support for flags, options, positional arguments, and subcommands.

## Core Data Structures

- [ ] ApeArgParser - Main parser structure
- [ ] ApeArg - Argument definition (name, type, description, default, etc.)
- [ ] ApeArgResult - Parsed argument results
- [ ] ApeArgType - Argument type enum (flag, string, int, float, list, etc.)

## Parser Setup

- [ ] ape_args_new - Create new argument parser
- [ ] ape_args_free - Free parser resources
- [ ] ape_args_set_program - Set program name (for help text)
- [ ] ape_args_set_description - Set program description
- [ ] ape_args_set_version - Set program version
- [ ] ape_args_set_epilog - Set help epilog text

## Argument Definition

- [ ] ape_args_add_flag - Add boolean flag (-v, --verbose)
- [ ] ape_args_add_string - Add string option (--output FILE)
- [ ] ape_args_add_int - Add integer option (--count N)
- [ ] ape_args_add_float - Add float option (--threshold F)
- [ ] ape_args_add_list - Add list option (can be specified multiple times)
- [ ] ape_args_add_positional - Add positional argument
- [ ] ape_args_add_positional_list - Add positional argument that collects remaining args

## Argument Properties

- [ ] ape_arg_required - Mark argument as required
- [ ] ape_arg_default_str - Set default string value
- [ ] ape_arg_default_int - Set default int value
- [ ] ape_arg_choices - Restrict to specific choices
- [ ] ape_arg_env - Also read from environment variable
- [ ] ape_arg_hidden - Hide from help text

## Parsing

- [ ] ape_args_parse - Parse argc/argv
- [ ] ape_args_parse_or_exit - Parse and exit on error with usage
- [ ] ape_args_error - Report parsing error

## Result Access

- [ ] ape_args_get_flag - Get boolean flag value
- [ ] ape_args_get_string - Get string option value
- [ ] ape_args_get_int - Get integer option value
- [ ] ape_args_get_float - Get float option value
- [ ] ape_args_get_list - Get list of values
- [ ] ape_args_get_positional - Get positional argument
- [ ] ape_args_get_positionals - Get remaining positional arguments
- [ ] ape_args_has - Check if argument was provided
- [ ] ape_args_count - Count how many times argument was provided

## Subcommands

- [ ] ape_args_add_subcommand - Add subcommand with its own parser
- [ ] ape_args_get_subcommand - Get which subcommand was invoked
- [ ] ape_args_subcommand_parser - Get parser for subcommand

## Help/Usage

- [ ] ape_args_print_help - Print help message
- [ ] ape_args_print_usage - Print brief usage
- [ ] ape_args_print_version - Print version
- [ ] ape_args_help_string - Get help as string

## Built-in Arguments

- [ ] Auto-add --help/-h flag
- [ ] Auto-add --version/-V flag (if version set)
