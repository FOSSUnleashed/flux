# NAME

 ws_response_key - generates a WebSocket response key for a given request key

# SYNOPSIS

```c
#include <libdill.h>

int ws_response_key(
    const char* request_key,
    char* response_key);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 This is a helper function that can be used when doing a custom HTTP handshake for the WebSocket protocol (see **WS_NOHTTP** flag). On the server, it can be used to  generates an RFC-compliant response key, to be filled into **Sec-WebSocket-Accept** field, for a request key received from the client. On the client side, it can be used to verify that the response key received from the server is correct.

 The generated key is null-terminated.

 **request_key**: The request key, as passed in **Sec-WebSocket-Key** field.

 **response_key**:                     Buffer to hold the generated response key. It must be at                    least **WS_KEY_SIZE** bytes long.                

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EINVAL**: Invalid argument.
* **EPROTO**: Supplied request key is not RFC 6455 compliant.

# EXAMPLE

```c
char name[256];
char value[256];
http_recvfield(s, name, sizeof(name), value, sizeof(value), -1);
assert(strcasecmp(name, "Sec-WebSocket-Key") == 0);
ws_response_key(value, response_key);
http_sendfield(s, "Sec-WebSocket-Accept", response_key, -1);
```

# SEE ALSO

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_done**(3) **ws_recv**(3) **ws_recvl**(3) **ws_request_key**(3) **ws_send**(3) **ws_sendl**(3) **ws_status**(3) 

