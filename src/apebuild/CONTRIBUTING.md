# Apebuild Development Guidelines

## File Structure

```
src/apebuild/
├── apebuild_api.h      # Public API (structs, typedefs, function declarations)
├── apebuild_internal.h # Private definitions (DO NOT EDIT MANUALLY)
├── ape_str.c           # Strings module implementation
├── ape_fs.c            # Filesystem module implementation
├── ape_cmd.c           # Command execution module implementation
├── ape_log.c           # Logging module implementation
├── test.c              # Unit tests
└── TODO*.md            # Planning documents
```

## Rules

### apebuild_api.h
- Contains all **public** API definitions
- Struct definitions and typedefs go here
- Function declarations (no prefixes) go inside the `extern "C" { }` block
- Public includes should go at the start of the `extern "C"` block

### apebuild_internal.h
- Contains private definitions
- **NEVER EDIT MANUALLY** - there is a custom tool for this
- Only modify when explicitly instructed

### Module Implementation Files (ape_*.c)
- Include `apebuild_internal.h` (NOT `apebuild_api.h`)
- Private includes go here
- Function implementations go here:
  - **Public functions**: Preceded with `APEBUILD_DEF`
  - **Private functions**: Preceded with `APEBUILD_PRIVATE`

### test.c
- Include `apebuild_api.h` (NOT `apebuild_internal.h`)
- Write unit tests for all modules
- No other restrictions

## Example

### In apebuild_api.h:
```c
extern "C" {

#include <stddef.h>  /* public includes at start */

typedef struct {
    size_t capacity;
    size_t count;
    char *items;
} ApeStrBuilder;

void ape_sb_init(ApeStrBuilder *sb);
char *ape_str_dup(const char *str);

}
```

### In ape_str.c:
```c
#include "apebuild_internal.h"
#include <string.h>  /* private includes */

APEBUILD_PRIVATE void some_helper(void) {
    /* private helper function */
}

APEBUILD_DEF void ape_sb_init(ApeStrBuilder *sb) {
    sb->capacity = 0;
    sb->count = 0;
    sb->items = NULL;
}

APEBUILD_DEF char *ape_str_dup(const char *str) {
    /* implementation */
}
```

### In test.c:
```c
#include "apebuild_api.h"
#include <stdio.h>

int main() {
    /* tests */
    return 0;
}
```
