# NAME

 ws_request_key - generates an unique value for Sec-WebSocket-Key field

# SYNOPSIS

```c
#include <libdill.h>

int ws_request_key(
    char* request_key);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 This is a helper function that can be used when doing a custom HTTP handshake for the WebSocket protocol (see **WS_NOHTTP** flag). It generates an unique RFC-compliant key to be filled into **Sec-WebSocket-Key** of the HTTP request.

 The generated key is null-terminated.

 **request_key**:                     Buffer to hold the generated request key. It must be at least                    **WS_KEY_SIZE** bytes long.                

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EINVAL**: Invalid argument.

# EXAMPLE

```c
char request_key[WS_KEY_SIZE];
ws_request_key(request_key);
http_sendfield(s, "Sec-WebSocket-Key", request_key, -1);
```

# SEE ALSO

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_done**(3) **ws_recv**(3) **ws_recvl**(3) **ws_response_key**(3) **ws_send**(3) **ws_sendl**(3) **ws_status**(3) 

