# Filesystem Module TODO

Module for filesystem operations: reading/writing files, directory iteration, path manipulation, and file metadata.

## File Reading/Writing

- [ ] ape_fs_read_file - Read entire file into memory (returns allocated buffer)
- [ ] ape_fs_read_file_lines - Read file as array of lines
- [ ] ape_fs_write_file - Write buffer to file
- [ ] ape_fs_append_file - Append buffer to file
- [ ] ape_fs_copy_file - Copy file from source to destination
- [ ] ape_fs_rename - Rename/move file with error handling

## Directory Operations

- [ ] ape_fs_mkdir - Create directory (single level)
- [ ] ape_fs_mkdir_p - Create directory recursively (like mkdir -p)
- [ ] ape_fs_rmdir - Remove empty directory
- [ ] ape_fs_rmdir_r - Remove directory recursively
- [ ] ape_fs_exists - Check if path exists
- [ ] ape_fs_is_file - Check if path is a regular file
- [ ] ape_fs_is_dir - Check if path is a directory

## Directory Iteration

- [ ] ApeDir - Directory handle structure
- [ ] ApeDirEntry - Directory entry structure (name, type, etc.)
- [ ] ape_fs_opendir - Open directory for iteration
- [ ] ape_fs_readdir - Read next entry from directory
- [ ] ape_fs_closedir - Close directory handle
- [ ] ape_fs_iterdir - Iterate directory with callback
- [ ] ape_fs_iterdir_r - Iterate directory recursively with callback
- [ ] ape_fs_glob - Find files matching a pattern

## File Metadata

- [ ] ApeFileStat - File stat structure (size, mtime, mode, etc.)
- [ ] ape_fs_stat - Get file metadata
- [ ] ape_fs_mtime - Get file modification time
- [ ] ape_fs_size - Get file size
- [ ] ape_fs_is_newer - Check if file A is newer than file B
- [ ] ape_fs_needs_rebuild - Check if output needs rebuild based on input mtimes

## Path Manipulation

- [ ] ape_fs_join - Join path components
- [ ] ape_fs_dirname - Get directory portion of path
- [ ] ape_fs_basename - Get filename portion of path
- [ ] ape_fs_extension - Get file extension
- [ ] ape_fs_stem - Get filename without extension
- [ ] ape_fs_change_extension - Change file extension
- [ ] ape_fs_normalize - Normalize path (resolve . and ..)
- [ ] ape_fs_absolute - Get absolute path
- [ ] ape_fs_relative - Get relative path from one path to another
- [ ] ape_fs_is_absolute - Check if path is absolute

## Temporary Files

- [ ] ape_fs_temp_dir - Get system temp directory
- [ ] ape_fs_temp_file - Create temporary file
- [ ] ape_fs_temp_mkdir - Create temporary directory

## Utility

- [ ] ape_fs_cwd - Get current working directory
- [ ] ape_fs_chdir - Change current working directory
- [ ] ape_fs_home - Get user home directory
