#

```
int flux_str_parse_ctl(
	const char * data,
	char * buf,
	uint32_t sz,
	uint32_t bsz,
	char ** errstr)
```

Copies data from `data` to `buf`, nul-terminating the key and the value, this removes the whitespace between the key and value.  This is meant to help impliment `ctl` 9p files.

# RETURNS

Returns the key size, or -1 on error.
