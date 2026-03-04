### ape_lz
Lightweight LZ4-style byte compressor. Fast compression, very fast
decompression. Raw block API — no framing format, no checksums, no streaming.
Compress a buffer, get a smaller buffer. Decompress it back. Suitable for asset
bundles, save files, and network packet payloads.

**Key types:** `ApeLzResult`
