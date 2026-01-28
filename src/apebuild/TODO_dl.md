# Download Module TODO

Module for downloading files from URLs, useful for fetching dependencies.

**Priority: Low** - Focus is on local builds for now.

## Basic Downloads

- [ ] ape_dl_fetch - Download URL to file
- [ ] ape_dl_fetch_to_buffer - Download URL to memory buffer
- [ ] ape_dl_fetch_string - Download URL as string (for text content)

## Download Options

- [ ] ApeDownloadOptions - Download configuration
  - [ ] timeout - Connection timeout (seconds)
  - [ ] follow_redirects - Follow HTTP redirects
  - [ ] max_redirects - Maximum redirects to follow
  - [ ] verify_ssl - Verify SSL certificates
  - [ ] user_agent - Custom user agent string
  - [ ] headers - Custom HTTP headers
- [ ] ape_dl_options_default - Get default options

## Progress Tracking

- [ ] ApeDownloadProgress - Progress information
  - [ ] total_bytes - Total file size (if known)
  - [ ] downloaded_bytes - Bytes downloaded so far
  - [ ] speed - Current download speed
- [ ] ApeDownloadCallback - Progress callback function type
- [ ] ape_dl_fetch_progress - Download with progress callback

## Async Downloads

- [ ] ApeDownloadHandle - Handle for async download
- [ ] ape_dl_start - Start async download
- [ ] ape_dl_poll - Check download status
- [ ] ape_dl_wait - Wait for download to complete
- [ ] ape_dl_cancel - Cancel in-progress download
- [ ] ape_dl_result - Get download result

## Caching

- [ ] ApeDownloadCache - Cache for downloaded files
- [ ] ape_dl_cache_new - Create download cache (specify cache dir)
- [ ] ape_dl_cache_free - Free cache
- [ ] ape_dl_cache_fetch - Fetch with caching
- [ ] ape_dl_cache_invalidate - Invalidate cache entry
- [ ] ape_dl_cache_clear - Clear entire cache
- [ ] Cache by URL hash or ETag/Last-Modified

## Verification

- [ ] ape_dl_fetch_verified - Download and verify checksum
- [ ] ape_dl_verify_md5 - Verify downloaded file MD5
- [ ] ape_dl_verify_sha256 - Verify downloaded file SHA256

## GitHub Integration

- [ ] ape_dl_github_release - Download latest release asset
- [ ] ape_dl_github_archive - Download repo archive (zip/tar.gz)
- [ ] ape_dl_github_file - Download single file from repo

## Error Handling

- [ ] ApeDownloadError - Error codes enum
  - [ ] APE_DL_OK - Success
  - [ ] APE_DL_ERR_NETWORK - Network error
  - [ ] APE_DL_ERR_DNS - DNS resolution failed
  - [ ] APE_DL_ERR_TIMEOUT - Connection timeout
  - [ ] APE_DL_ERR_HTTP - HTTP error (4xx, 5xx)
  - [ ] APE_DL_ERR_SSL - SSL/TLS error
  - [ ] APE_DL_ERR_WRITE - Failed to write file
- [ ] ape_dl_error_string - Get error description

## Implementation Notes

- Consider using libcurl as backend (optional dependency)
- Fallback to system wget/curl commands if libcurl unavailable
- Support both HTTP and HTTPS
