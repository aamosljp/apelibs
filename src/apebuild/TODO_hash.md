# Hash Module TODO

Module for computing hashes of files and buffers, useful for content-based rebuild detection.

**Priority: Low** - For now we focus on mtime-based rebuild detection.

## Hash Types

- [ ] ApeHashType - Hash algorithm enum
  - [ ] APE_HASH_MD5 - MD5 (fast, not cryptographically secure)
  - [ ] APE_HASH_SHA1 - SHA-1
  - [ ] APE_HASH_SHA256 - SHA-256
  - [ ] APE_HASH_XXH64 - xxHash64 (very fast, non-crypto)
  - [ ] APE_HASH_XXH3 - xxHash3 (fastest, non-crypto)

## Hash Result

- [ ] ApeHash - Hash result structure
  - [ ] bytes - Raw hash bytes
  - [ ] len - Hash length in bytes
  - [ ] type - Hash algorithm used
- [ ] ape_hash_to_hex - Convert hash to hex string
- [ ] ape_hash_eq - Compare two hashes
- [ ] ape_hash_free - Free hash resources

## Buffer Hashing

- [ ] ape_hash_buffer - Hash a buffer
- [ ] ape_hash_string - Hash a string
- [ ] ape_hash_md5 - Hash buffer with MD5
- [ ] ape_hash_sha256 - Hash buffer with SHA256
- [ ] ape_hash_xxh64 - Hash buffer with xxHash64

## File Hashing

- [ ] ape_hash_file - Hash file contents
- [ ] ape_hash_file_md5 - Hash file with MD5
- [ ] ape_hash_file_sha256 - Hash file with SHA256
- [ ] ape_hash_file_xxh64 - Hash file with xxHash64

## Incremental Hashing

- [ ] ApeHashContext - Incremental hash context
- [ ] ape_hash_init - Initialize hash context
- [ ] ape_hash_update - Update hash with more data
- [ ] ape_hash_final - Finalize and get hash
- [ ] ape_hash_reset - Reset context for reuse

## Hash Cache (for build system)

- [ ] ApeHashCache - Cache of file hashes
- [ ] ape_hashcache_new - Create hash cache
- [ ] ape_hashcache_free - Free hash cache
- [ ] ape_hashcache_get - Get cached hash for file
- [ ] ape_hashcache_update - Update cache entry
- [ ] ape_hashcache_invalidate - Invalidate cache entry
- [ ] ape_hashcache_load - Load cache from file
- [ ] ape_hashcache_save - Save cache to file
- [ ] ape_hashcache_needs_rebuild - Check if file changed (by hash)

## Utility

- [ ] ape_hash_hex_to_bytes - Convert hex string to bytes
- [ ] ape_hash_bytes_to_hex - Convert bytes to hex string
