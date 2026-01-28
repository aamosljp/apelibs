# Archive Module TODO

Module for creating and extracting archive files (tar, zip, etc.).

**Priority: Low** - Focus is on local builds for now.

## Archive Types

- [ ] ApeArchiveType - Archive type enum
  - [ ] APE_ARCHIVE_TAR - Plain tar
  - [ ] APE_ARCHIVE_TAR_GZ - Gzip compressed tar
  - [ ] APE_ARCHIVE_TAR_BZ2 - Bzip2 compressed tar
  - [ ] APE_ARCHIVE_TAR_XZ - XZ compressed tar
  - [ ] APE_ARCHIVE_ZIP - Zip archive

## Archive Detection

- [ ] ape_archive_detect - Detect archive type from file
- [ ] ape_archive_detect_ext - Detect type from extension

## Extraction

- [ ] ape_archive_extract - Extract archive to directory
- [ ] ape_archive_extract_file - Extract single file from archive
- [ ] ape_archive_list - List contents of archive

## Extraction Options

- [ ] ApeExtractOptions - Extraction options
  - [ ] overwrite - Overwrite existing files
  - [ ] preserve_permissions - Preserve file permissions
  - [ ] preserve_ownership - Preserve file ownership (requires root)
  - [ ] strip_components - Strip N leading path components
  - [ ] filter - Callback to filter which files to extract
- [ ] ape_extract_options_default - Get default options

## Creation

- [ ] ape_archive_create - Create archive from files/directories
- [ ] ape_archive_create_tar - Create tar archive
- [ ] ape_archive_create_tar_gz - Create gzipped tar
- [ ] ape_archive_create_zip - Create zip archive

## Creation Options

- [ ] ApeCreateOptions - Creation options
  - [ ] compression_level - Compression level (0-9)
  - [ ] include_hidden - Include hidden files
  - [ ] follow_symlinks - Follow symbolic links
  - [ ] base_dir - Base directory for relative paths

## Streaming/Incremental

- [ ] ApeArchiveWriter - Writer for creating archives incrementally
- [ ] ape_archive_writer_new - Create new archive writer
- [ ] ape_archive_writer_add_file - Add file to archive
- [ ] ape_archive_writer_add_buffer - Add buffer as file
- [ ] ape_archive_writer_add_dir - Add directory
- [ ] ape_archive_writer_close - Finalize and close archive
- [ ] ape_archive_writer_free - Free writer

## Reading

- [ ] ApeArchiveReader - Reader for reading archives
- [ ] ApeArchiveEntry - Entry in archive (name, size, type, etc.)
- [ ] ape_archive_reader_open - Open archive for reading
- [ ] ape_archive_reader_next - Get next entry
- [ ] ape_archive_reader_read - Read current entry data
- [ ] ape_archive_reader_skip - Skip current entry
- [ ] ape_archive_reader_close - Close reader
- [ ] ape_archive_reader_free - Free reader

## In-Memory Archives

- [ ] ape_archive_extract_to_memory - Extract to memory (hash map of name -> buffer)
- [ ] ape_archive_create_from_memory - Create archive from memory buffers

## Error Handling

- [ ] ApeArchiveError - Error codes
  - [ ] APE_ARCHIVE_OK
  - [ ] APE_ARCHIVE_ERR_OPEN - Failed to open archive
  - [ ] APE_ARCHIVE_ERR_READ - Read error
  - [ ] APE_ARCHIVE_ERR_WRITE - Write error
  - [ ] APE_ARCHIVE_ERR_CORRUPT - Corrupt archive
  - [ ] APE_ARCHIVE_ERR_UNSUPPORTED - Unsupported format

## Implementation Notes

- Consider using libarchive as backend (optional dependency)
- Fallback to system tar/unzip commands if libarchive unavailable
- Pure C implementations for simple cases (tar without compression)
