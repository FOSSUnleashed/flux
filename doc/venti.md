
```
 VtThello(4) tag[1] version[s] uid[s] strength[1] crypto[n] codec[n]
 VtRhello(5) tag[1] sid[s] rcrypto[1] rcodec[1]

 VtTping(2) tag[1]
 VtRping(3) tag[1]

 VtTread(12) tag[1] score[20] type[1] pad[1] count[2]
 VtRread(13) tag[1] data[]

 VtTwrite(14) tag[1] type[1] pad[3] data[]
 VtRwrite(15) tag[1] score[20]

 VtTsync(16) tag[1]
 VtRsync(17) tag[1]

 VtRerror(1) tag[1] error[s]

 VtTgoodbye(6) tag[1]
```

```
00000000: 7665 6e74 692d 3034 3a30 322d 6c69 6276  venti-04:02-libv
00000010: 656e 7469 0a00 0000 1404 0000 0230 3400  enti.........04.
00000020: 0961 6e6f 6e79 6d6f 7573 0000 00         .anonymous...
```

```
VtRoot:
u16: ver (0002)
[128]: name
[128]: type (vac)
[20]: score -> VtDirectory
u16: block size
[20]: previous score -> VtRoot

VtDirectory/VtEntry: (multiple in one block)
u32: gen
u16: psize // numOfScoresInBSZ * ScoreSize
u16: dsize // Block Size
u8: flags
u8[5]: pad
u48: size
score:

 MetaMagic = 0x5656fc79
 MetaHeaderSize = 12,
 MetaIndexSize = 4,
 IndexEntrySize = 8,
 DirMagic = 0x1c4d9072


VtMetaBlock:
u32: magic (5656 fc79)
u16: size
u16: free
u16: maxIndex
u16: nindex

VtEntryBlock?
u16: offset
u16: en???????

VtDir:
u32: magic (DirMagic)
u16: version (7 - 9)
2str:	element name (file name)
u32: entry? (index into corresponding VtEntry table)
if (9 > version)
	gen = 0
	mentry = entry + 1
	mgen = 0
else
	u32: gen
	u32: mentry
	u32: mgen
// size is gotten from DirEntry?????
u64: qid
if (7 == version)
	score
2str: uid
2str: gid
2str: muid
u32:	mtime
u32:	mcount
u32:	ctime
u32:	atime
u32:	mode
(optional) -> {
	u8: type
	u16: sz
	switch (type) {
	DirPlan9Entry:
		if (9 > version) {
			dir->plan9 = 1
			u64: p9path
			u32: p9ver
			mcount = mcount || p9ver
		}
	DirGenEntry:
		u32: gen
	DirQidSpaceEntry:
		qidspace = 1
		u64: qidoffset
		u64: qidmax
	}
}

   ModeOtherExec = (1<<0),
   ModeOtherWrite = (1<<1),
   ModeOtherRead = (1<<2),
   ModeGroupExec = (1<<3),
   ModeGroupWrite = (1<<4),
   ModeGroupRead = (1<<5),
   ModeOwnerExec = (1<<6),
   ModeOwnerWrite = (1<<7),
   ModeOwnerRead = (1<<8),
   ModeSticky = (1<<9),
   ModeSetUid = (1<<10),
   ModeSetGid = (1<<11),
   ModeAppend = (1<<12),      /* append only file */
   ModeExclusive = (1<<13),   /* lock file - plan 9 */
   ModeLink = (1<<14),     /* sym link */
   ModeDir  = (1<<15),     /* duplicate of DirEntry */
   ModeHidden = (1<<16),      /* MS-DOS */
   ModeSystem = (1<<17),      /* MS-DOS */
   ModeArchive = (1<<18),     /* MS-DOS */
   ModeTemporary = (1<<19),   /* MS-DOS */
   ModeSnapshot = (1<<20),    /* read only snapshot */
   ModeDevice = (1<<21),      /* Unix device */
   ModeNamedPipe = (1<<22) /* Unix named pipe */

   DirPlan9Entry = 1,   /* not valid in version >= 9 */
   DirNTEntry,    /* not valid in version >= 9 */
   DirQidSpaceEntry,
   DirGenEntry    /* not valid in version >= 9 */
```
