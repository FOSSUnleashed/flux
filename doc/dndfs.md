# D&D File Server

The D&D File Server (dndfs) is a data broker for a collection of clients to
run and play a Dungeons and Dragons game.  The main operating principle is to
enable a simple interface for a multitude of small services to handle small
parts of running and playing a D&D game.  As 9p file server, the interface can
be mounted to a file tree, allowing API access through basic file operations,
increasing the options for which langauges can handle individual parts.

Data can be extracted from files as textual representations of that data (for
example a creature's /hp file would return "50 64\n" if that creature had 50
current HP, and a maximum of 64 HP).  In certain cases, it is possible to
inform dndfs that the connection can work with binary data, in which case
certain files will offer binary data instead of textual representations of it.

Complex objects are represented as a directory with multiple sub-files, thus
each creature is available as a directory with the same base structure, and
each item is likewise represented as a directory with a separate structure.
In certain cases, a file might be available in multiple places, such as the
game map, there is a possibility that reading the same file in different
places will provide different information.  Each creature's "view" of the game
map will provide only information that creature would know.  Likewise, a
creature directory will contain a list of creatures it is aware of, these views
of the creatures will only contain information that the viewing creature could
know or infer.  These views of the other creatures will not contain further
views.

# File Behaviors

## State file

A state file is a read-only file that provides access to a variable in the
associated structure.  For example, each creature would have /hp.current and
/hp.max files, in addition to the convenience /hp file.

## Variable file

A variable file is a read-write file that provides access to a variable in the
associated structure.  A write to the file in the same format as the read
result will allow a client to change the value of that variable.  A write error
**will** occur if the value is malformed or out of range.

## Multi-state file

A read-only file that provides access to multiple variables in a structure.  It
is possible the specific values in this file also have their own dedicated state
or variable files, just as it is possible for any provided variable to not have
such an associated file.  The format of a multi-state file is
`<key><spaces><value>\n` for each exposed variable, where `key` is one or more
alpha-numeric or underscore characters, `spaces` is one or more space or tab
characters, and `value` is one or more alpha-numeric, space, tab, or special
ascii character.

## Multi-variable file

A file much like a multi-state file.  Single variables can be written to the
file in the same format as they are read, not all variables need to be provided
in a write operation, however each single write operation must only contain
complete lines for writing, with a terminating new-line.

## Event file

An event file is a read-only file that provides a single "event" per read
operation.  The format of the event is usually a single new-line terminated
line, but other formats are allowed.  The specific structure of the event is
determined by the specific event file.

## RPC file

A read-write file that can behave like a multi-variable file or an event-file
on read, but accepts line-delimited input as "commands" or "events".  In some
cases the commands or events sent to the file on write might be passed to all
readers as-is.  The specifics of the file behavior will be documented per-file.

## Pure-RPC file

As an RPC file, however no events written to this file are passed out as-is.
This designation is mostly to make behaviors clearer in documentation.

## Gate file

A read-only file which will only EOF on read.  The EOF will only occur when
a specific state has changed.  These files are to be used by clients for
basic event-based control.  Each creature has a /ready file, this file will
only EOF while it is that creature's turn to act.  In certain cases, it is
possible for the read to fail (such as the creature has died), this generally
indicates that the client program needs to handle an abnormal state, related to
the event being read.

# Directory Trees (WIP)

## Root

* **$creature/**: A directory for each creature
* **$item/**: `DM Only` A directory for each item
* **maps/**: `DM Only` A directory of all maps
* **chat**: An RPC file, all writes are passed to readers as-is as events.
* **map**: `DM Only` The base map file (read only)
* **ctl**: Multi-Variable file controlling the connection-level variables.
	* `binary_map`: Boolean, default false, are reads of `map` returning a textual
		(false) or binary (true) representation of the map data?
	* `DM`: Boolean, read-only, is this connection seen by the server as a DM
		connection?

## Creature

* **hp**: Int[2]:State Effective current and maximum HP, space delim'd
* **hp.current**: Int:State current real HP
* **hp.max**: Int:State maximum real HP
* **hp.tmp**: Int:State current temporary HP
* **ability**: Int[4]:Multi-state/RPC: Ability table, in order: Base, Modifier,
	Temporary, Effective Modifier.  Writes are accepted to apply level-up
	Ability Score bonuses only.
* **$ability**: Int:State Current effective ability score.
* **$ability_mod**: Int:State Current effective ability modifier.
* **skills**: Int[3]+Boolean:Multi-state/RPC: Skills table, in order: Score, Ranks,
	Modifiers, Class-skill
* **level**: Int:State Current creature level/max-HD.
* **classes/**:
	* **level**: Int:State Current level in the specified class
	* **caster**: Int:State Effective caster level (if any) for the class
* **map**: State.  Creature's view of the map, affected by `binary_map` in `ctl`.
* **try**: Variable.  A valid action that the creature wishes to try.  This
	file and files that make use of it are AI-helpers.  Player clients are also
	allowed to make use of these files.
* **line.sight**: State, new-line delim'd list of creatures that are available
	for line-of-sight effects.  Affected by `try`
* **line.effect**: State, as `line.sight` but for line-of-effect effects.
* **spells/**: Directory of spells available for casting, or other actions if
	any.
