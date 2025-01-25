

We need to send JSON objects to twitch, and get JSON objects back.
There is meta-data in "POST" vs "GET" and the uri, and occasional the get params
We don't want our process to have the keys
It's fine if our process (`srv_t9`) has a key-proxy

## MUNGE

* Your process UID is the "key", so RCE is automatic authenticated

## Factotum

* Any process that can access the factotum instance can potentially make the connection
* Factotum can notify the user if the key(-proxy) has been requested (could be a ton of noise)

## Kerberos

* No idea how it works
