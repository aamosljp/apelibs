# Config Module TODO

Module for reading and writing configuration files in various formats.

**Priority: Low** - Build configuration is done in code for now.

## Supported Formats

- [ ] INI format (simple key-value with sections)
- [ ] Simple key=value format
- [ ] JSON format (optional, may require external dependency)

## Config Structure

- [ ] ApeConfig - Configuration container
- [ ] ApeConfigSection - Section within config (for INI)
- [ ] ApeConfigValue - Value that can be string, int, float, bool, or list

## Loading/Saving

- [ ] ape_config_new - Create empty config
- [ ] ape_config_free - Free config
- [ ] ape_config_load - Load from file (auto-detect format)
- [ ] ape_config_load_ini - Load INI file
- [ ] ape_config_load_kv - Load simple key=value file
- [ ] ape_config_load_json - Load JSON file
- [ ] ape_config_save - Save to file
- [ ] ape_config_save_ini - Save as INI
- [ ] ape_config_save_kv - Save as key=value
- [ ] ape_config_save_json - Save as JSON
- [ ] ape_config_parse - Parse config from string

## Getters

- [ ] ape_config_get_str - Get string value
- [ ] ape_config_get_str_default - Get string with default
- [ ] ape_config_get_int - Get integer value
- [ ] ape_config_get_int_default - Get integer with default
- [ ] ape_config_get_float - Get float value
- [ ] ape_config_get_float_default - Get float with default
- [ ] ape_config_get_bool - Get boolean value
- [ ] ape_config_get_bool_default - Get boolean with default
- [ ] ape_config_get_list - Get list of strings

## Setters

- [ ] ape_config_set_str - Set string value
- [ ] ape_config_set_int - Set integer value
- [ ] ape_config_set_float - Set float value
- [ ] ape_config_set_bool - Set boolean value
- [ ] ape_config_set_list - Set list value

## Sections (INI)

- [ ] ape_config_has_section - Check if section exists
- [ ] ape_config_add_section - Add new section
- [ ] ape_config_remove_section - Remove section
- [ ] ape_config_get_sections - Get list of section names
- [ ] ape_config_section_get_str - Get value from section
- [ ] ape_config_section_set_str - Set value in section

## Querying

- [ ] ape_config_has - Check if key exists
- [ ] ape_config_remove - Remove key
- [ ] ape_config_keys - Get all keys
- [ ] ape_config_clear - Clear all entries

## Iteration

- [ ] ApeConfigIter - Iterator for config entries
- [ ] ape_config_iter - Get iterator
- [ ] ape_config_iter_next - Get next entry
- [ ] ape_config_iter_section - Iterate within section

## Environment Integration

- [ ] ape_config_from_env - Create config from environment variables
- [ ] ape_config_get_env - Get value with env var fallback
- [ ] ape_config_expand_env - Expand $VAR in values

## Merge/Override

- [ ] ape_config_merge - Merge two configs
- [ ] ape_config_override - Override values from another config
