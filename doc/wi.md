# Info-face (WI)

## Wi

### Mindset

* One thing I notice is that people want a program that fully does the solution to a problem
	* I often more want a program that *HELPS* me solve the problem, or removes the tedium
	* Pluggable systems over discrete components

### Schedule

* This week just get twitch API fully working
* Next week dndfs? (Art Jan 31st, Game Feb 28th)
* Misc Jan stuff:
	* GDB
	* WoL
	* IPMI
* First of feb -- blox/dream

### Misc

* Way to mute via CLI
* Need a way to see the timer for pomo on my screen

### Stream/overlay ideas

* !suggest Suggestion box command
* !followage -- Command to print how long a user has followed
* !lurk
* !today Stream task command (not pomo mode)
* !task* Stream task commands (pomo mode)
* Group Luanti server?  TMW server/guild?
* Web and/or FTP interface to Rpub
* Variables Tanks -- I want overlay to be dynamic, controlled by meta-interaction
	* OwlkalineVT's stream
	* abby_the_lesbiab's stream
	* Tanks are numeric variables that decrease over time, increase on specific triggers
	* Bits --
	* Raid
	* Channel Point Redeems
		* Streamer Health
		* Display
		* Other
	* Chat Messages
	* Time??
	* Me talking (pull data from pngtuber software?)
	* Emotes
		* Cuteness
		* Specific other Streamers
		* MrGreen
		* Other
	* Subscriptions/Subpoints
	* Follow
	* Git Commits/PRs

### Wiki or git?
* Git:
	* wi view
	* wie edit
	* wig git commands
	* Something to build/maintain on stream
* Wiki:
	* People have lower barrier to contribute
	* Easier to display attribution/authorship on we
* Documentation
	* What projects I would maintain
		* XS
		* libdill
			* Xoodoo/Charm support
				* Enable it as a transport for the 9p layer
			* Spawn()
			* RabbitMQ
			* 0MQ
			* Graphana
			* Web
				* OBS
				* Twitch
				* Etherpad
				* Ethercalc
				* VPS Providers: DigitalOcean / Linode / vultr
				* Leantime
				* k8s
				* searx
				* Ko-Fi
				* Patreon
				* Youtube
				* Google
				* Github/Gitlab/Forje/src.ht
				* Discord
				* Slack
				* Mastodon
				* IRC
				* JSON-RPC
				* Keyoxide
				* Aria2c
				* Throne.com
				* osquery
				* BlueSky
				* At Protocol
				* s3
				* Webdav
			* Moose
			* lm-sensors?
			* gkrelm?
			* MPI
			* NTP
			* Redis
			* MariaDB/MySQL/Postgres
			* 9p
				* Plumber
				* Factotum
				* 9p Auth
				* Venti
			* MUNGE
			* FTP
			* Chirp
			* LDAP
			* Dream
			* QEMU (RPC)
			* Hypercore
			* tftp
			* fxp
			* whois
			* syslog
			* mongoDB
			* FastCGI
			* IMAP/POP/SMTP
			* GPG Agent
			* SSH Agent
			* SSH
			* JACK
			* sndio
			* Memchachd
			* Xoodoo
			* dbus
			* IRC DCC
			* IRC CTCP
			* APC
			- ---
			- [x] ULID
			- [ ]  UUID
			- [ ]  ndb
			* tdb
			* ldb
			* irc log db
			* runit
			* terminal (pty curses?)??????
			* glob
			* time delta
			* blox (dream)
			* RPG Maker files are they game maker instead?
			* Wake-on-LAN
			* IPMI
			* Monit functions
			* SSH Key encrpyt/decrypt
			* MIME (pull from mblaze)
			* TLS Date
			* Fake FUSE (emulate the FUSE API for 9p)
			* Fake libev (emulate libev)
			* Fake libuv (emulate libuv)
			* Namespaces???? (Like Linux ones?)
			* XDG
			- [x]  dial/listen
			- [x]  xxd function (Function to do a hexdump)
			* libdill fd pair (from pipe())
			* cscope file parsing (GNU globals)
			* str2num/num2str
			* xs serialization
			* Joysticks (probably linux only, using /dev)
		* Alloy -- Multi script multiplexer?
		* abudco?/dvtm?
		* XS-funcs
		* 9uifs?
		* ---
		* varfs -- 9p -- varxfs?  -- Simple variable meta-container?  Types of variables: pubsub (event) / timer (gate and event) / var / ram-file / Color ram/var files
		* niine -- 9p -- IRC based on ii -- Discord / Slack / Mastodon / Blue Sky / AtProto / Matrix / Twitch-API / libpurple
		* os9 -- 9p -- Pomodoro, Work Cycles -- 
		* scriptworx -- 9p -- Like n8n, but each step can be performed by a person.
		* d9 -- 9p -- Tag-based access to Dream
		* Dream -- Bulk archival tag-based file storage (CAS)
		* TorrentFS -- 9p -- WRite to a file, then signal for archival in to dream, venti, s3, or some other CAS
		* NetFS -- 9p -- Systems management (zabbix runit and monit)
		* srvfs -- 9p -- Mutli-proxy-mux for 9p plan's /srv
		* Grocery -- 9p -- Keep track of groceries (good on potatoes, have file that lists only groceries that need to be be re-uped)  What groceries need to be used soon
		* vmctl -- Virtual machine control scripts, possible 9p server as well
		* SurfFS -- 9p -- Control server in 9p for a bunch of surf instances, in the vein of tabfs extension for firefox
		* fortune -- 9p -- Tagged fortunes
		* dndfs -- 9p -- OWN SECTION -- Electronic CO-DM system
		* FossilX -- 9p -- Tra'ified version of Fossil, possibly using Dream instead of or in addtion to Venti
		* upas (mblaze) -- 9p -- Tagged as per the KWEST whitepaper
		* Job Queue -- 9p -- Temporary solution to ScriptWorx
		* Inventory Tasks -- 9p -- Inventory Management System as a TODO producer
		* Contacts DB -- 9p -- PIM
		* XAF Overlay -- 9p -- Namespaces in Linux, provides bind command
		* Finances -- 9p -- Financial tracker in 9p
		* NewSam -- 9p -- OWN SECTION
		* Pane -- 9p client -- Part of NewSam
		* Wallpaperd -- 9p
		* devdraw -- 9p -- /dev/draw as a 9p server
		* SnarfFS -- 9p -- xsel as a file server
		* Kanban -- 9p -- WHY?
		* sndio -- 9p -- Control sndio via 9p
		* JACK -- 9p -- Control JACK via 9p
		* ALSA -- 9p -- Alsa mixer control via 9p
		* OBS -- 9p -- Control and recieve OBS events via 9p
		* Read Only u9fs -- Patch u9fs to have a read-only mode
		* Pngtuber -- 9p
		* ArpWatch -- 9p -- Seeing Arp events on network via 9p
		* DHCPWatch -- 9p -- As above
		* OSD -- 9p -- libosd or sxd as a 9p file server
		* Venti -- 9p -- Replace http interface with 9p
		* Remind -- 9p -- Maybe?
		* RSS -- 9p
		* isaac-vault -- 9p -- secstore via ISAAC as ic?
		* Blox -- 9p -- Simple blox server
		* ---
		* ethercalc -- 9p -- interface to ethercalc?
		* chirp -- 9p --
		* cctools meta -- 9p --
		* cctools job -- 9p --
		* Parrot_run -- 9p -- 
		* ssh -- 9p -- Maybe?
		* Moose -- 9p
	* homenet / extnet / portnet
	* Power Save
	* Boot from Moose
	* Wake on LAN
	* IPMI
	* Redfish? https://en.wikipedia.org/wiki/Redfish_(specification)
	* CLI utils -- file-time-delta
	* vim remote control
		* Get the line at the cursor
		* Get the word at the curson
		* Create a window/buffer and read/write
		* Run a vim command?
	* Mail events --> feeds into factotum
* Stuff to look into
	* Ori
	- [x]  Tra
	* W.A.S.T.E.
	* KWEST
	* gnutella
	* FreeNet


## Community Systems

## Build container

## Build system (mk)

# Plan for year (near term)
# Plan for Q1 2025 (short term)
# Plan for future (long term)

# Marketting

## Why 9p

## What do I do that's worth watching/supporting
