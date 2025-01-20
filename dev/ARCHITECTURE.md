# Architecture

* libdill -- Mostly original libdill, a coroutine/networking library
* libflux -- A compliment to libdill, extending available protocols, adding more utility functions

## Definitions

* coroutine
* handle
* bsock vs msock
* channels?

# Vendor Code

* ISAAC (include/flux/isaac.h libflux/isaac.c)
	* ISAAC PRNG
	* AUTHOR: Bob Jenkins
	* LICENSE: "Public Domain"

* ISAAC-Crypto (include/flux/isaacbhc.h libflux/isaacbhc.c)
	* ISAAC-crypto (originally from: https://github.com/BryceHowell/isaac-crypto)
	* AUTHOR: Bryce Howell
	* LICENSE: "Public Domain"
	* Meant to be a simple replacement for gpg, using ISAAC and a feed-forward algorithm

## NOTES:

Reference the following documents as this document gets filled out
Explain what's going on at a high level
Explain terminology needed to understand the systems
Explain the broad archetecture
Explain where to get started when reading the code

Ref: https://github.com/outline/outline/blob/main/docs/ARCHITECTURE.md
Ref: https://github.com/rust-lang/rust-analyzer/blob/master/docs/dev/architecture.md
