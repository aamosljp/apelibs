# Logging Module TODO

Module for configurable logging with levels, colors, file output, and timestamps.

## Log Levels

- [ ] ApeLogLevel - Log level enum
  - [ ] APE_LOG_TRACE - Most verbose, for debugging details
  - [ ] APE_LOG_DEBUG - Debug information
  - [ ] APE_LOG_INFO - General information
  - [ ] APE_LOG_WARN - Warnings
  - [ ] APE_LOG_ERROR - Errors
  - [ ] APE_LOG_FATAL - Fatal errors (may exit)
  - [ ] APE_LOG_OFF - Disable all logging

## Basic Logging

- [ ] ape_log - Log message at specified level
- [ ] ape_log_trace - Log at TRACE level
- [ ] ape_log_debug - Log at DEBUG level
- [ ] ape_log_info - Log at INFO level
- [ ] ape_log_warn - Log at WARN level
- [ ] ape_log_error - Log at ERROR level
- [ ] ape_log_fatal - Log at FATAL level

## Formatted Logging

- [ ] ape_logf - Log formatted message (printf-style)
- [ ] ape_log_tracef - Formatted TRACE
- [ ] ape_log_debugf - Formatted DEBUG
- [ ] ape_log_infof - Formatted INFO
- [ ] ape_log_warnf - Formatted WARN
- [ ] ape_log_errorf - Formatted ERROR
- [ ] ape_log_fatalf - Formatted FATAL

## Configuration

- [ ] ApeLogConfig - Logger configuration structure
- [ ] ape_log_init - Initialize logging system
- [ ] ape_log_shutdown - Shutdown logging system
- [ ] ape_log_set_level - Set minimum log level
- [ ] ape_log_get_level - Get current log level
- [ ] ape_log_set_output - Set output file (default stderr)
- [ ] ape_log_set_file - Log to file by path
- [ ] ape_log_add_output - Add additional output destination
- [ ] ape_log_set_quiet - Suppress console output (file only)

## Formatting Options

- [ ] ape_log_set_colors - Enable/disable colored output
- [ ] ape_log_set_timestamps - Enable/disable timestamps
- [ ] ape_log_set_timestamp_fmt - Set timestamp format string
- [ ] ape_log_set_show_level - Show/hide level prefix
- [ ] ape_log_set_show_file - Show/hide source file:line
- [ ] ape_log_set_prefix - Set custom prefix string

## Color Configuration

- [ ] ApeLogColors - Color scheme structure
- [ ] ape_log_set_color_scheme - Set custom colors for each level
- [ ] ape_log_default_colors - Get default color scheme
- [ ] Color detection for terminal support

## Context/Tagging

- [ ] ape_log_push_context - Push context tag (e.g., "[build]")
- [ ] ape_log_pop_context - Pop context tag
- [ ] ape_log_with_tag - Log with specific tag

## Utility

- [ ] ape_log_level_name - Get name string for level
- [ ] ape_log_level_from_name - Parse level from string
- [ ] ape_log_flush - Flush log output

## Build System Integration

- [ ] ape_log_cmd - Log a command being executed (CMD: prefix)
- [ ] ape_log_build - Log build step (BUILD: prefix)
- [ ] ape_log_link - Log link step (LINK: prefix)
- [ ] ape_log_success - Log success message (green)
- [ ] ape_log_failure - Log failure message (red)

## Macros

- [ ] APE_LOG(level, msg) - Basic log macro
- [ ] APE_LOGF(level, fmt, ...) - Formatted log macro
- [ ] APE_TRACE, APE_DEBUG, APE_INFO, APE_WARN, APE_ERROR, APE_FATAL - Level-specific macros
- [ ] Macros should include __FILE__ and __LINE__ when enabled
