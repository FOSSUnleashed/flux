#define ARGBEGIN \
		int _argtmp=0, _inargv=0; char *_argv=NULL; \
		if(!argv0) {argv0=*argv; argv++, argc--;} \
		_inargv=1; USED(_inargv); \
		while(argc && argv[0][0] == '-') { \
			_argv=&argv[0][1]; argv++; argc--; \
			if(_argv[0] == '-' && _argv[1] == '\0') \
				break; \
			while(*_argv) switch(*_argv++)
#define ARGEND }_inargv=0;USED(_argtmp, _argv, _inargv)

#define EARGF(f) ((_inargv && *_argv) ? \
			(_argtmp=strlen(_argv), _argv+=_argtmp, _argv-_argtmp) \
			: ((argc > 0) ? \
				(--argc, ++argv, _used(argc), *(argv-1)) \
				: ((f), (char*)0)))
#define ARGF() EARGF(_used(0))

#ifndef KENC
static inline void _used(long a, ...) { if(a){} }
# define USED(...) _used((long)__VA_ARGS__)
#endif
