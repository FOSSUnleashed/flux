/*
------------------------------------------------------------------------------
rand.c: By Bob Jenkins.  My random number generator, ISAAC.  Public Domain.
MODIFIED:
  960327: Creation (addition of randinit, really)
  970719: use context, not global variables, for internal state
  980324: added main (ifdef'ed out), also rearranged randinit()
  010626: Note that this is public domain
------------------------------------------------------------------------------
*/
#define FLUX_DISABLE_RAW_NAMES
#include <flux/isaac.h>

#define ind32(T,mm,x)  (*(T *)((uint8_t *)(mm) + ((x) & ((FLUX_ISAAC_RANDSIZ-1)<<2))))
#define rngstep32(T,mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (a^(mix)) + *(m2++); \
  *(m++) = y = ind32(T,mm,x) + a + b; \
  *(r++) = b = ind32(T,mm,y>>FLUX_ISAAC_RANDSIZL) + x; \
}

void flux_isaac63(flux_isaac64_ctx *ctx) {
   register uint64_t a,b,x,y,*m,*mm,*m2,*r,*mend;
   mm=ctx->randmem; r=ctx->randrsl;
   a = ctx->randa; b = ctx->randb + (++ctx->randc);

   for (m = mm, mend = m2 = m+(FLUX_ISAAC_RANDSIZ/2); m<mend; )
   {
      rngstep32(uint64_t, a<<13, a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a>>6 , a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a<<2 , a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a>>16, a, b, mm, m, m2, r, x);
   }
   for (m2 = mm; m2<mend; )
   {
      rngstep32(uint64_t, a<<13, a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a>>6 , a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a<<2 , a, b, mm, m, m2, r, x);
      rngstep32(uint64_t, a>>16, a, b, mm, m, m2, r, x);
   }
   ctx->randb = b; ctx->randa = a;
}

#define mix32(a,b,c,d,e,f,g,h) \
{ \
   a^=b<<11; d+=a; b+=c; \
   b^=c>>2;  e+=b; c+=d; \
   c^=d<<8;  f+=c; d+=e; \
   d^=e>>16; g+=d; e+=f; \
   e^=f<<10; h+=e; f+=g; \
   f^=g>>4;  a+=f; g+=h; \
   g^=h<<8;  b+=g; h+=a; \
   h^=a>>9;  c+=h; a+=b; \
}

/* if (flag==TRUE), then use the contents of randrsl[] to initialize mm[]. */
void flux_isaac63_init(flux_isaac64_ctx *ctx, int flag) {
   int i;
   uint64_t a,b,c,d,e,f,g,h;
   uint64_t *m,*r;
   ctx->randa = ctx->randb = ctx->randc = 0;
   m=ctx->randmem;
   r=ctx->randrsl;
   a=b=c=d=e=f=g=h=0x9e3779b9;  /* the golden ratio */

   for (i=0; i<4; ++i)          /* scramble it */
   {
     mix32(a,b,c,d,e,f,g,h);
   }

   if (flag) 
   {
     /* initialize using the contents of r[] as the seed */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
       e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
     /* do a second pass to make all of the seed affect all of m */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
       e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   }
   else
   {
     /* fill in m[] with messy stuff */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   }

   flux_isaac63(ctx);            /* fill in the first set of results */
   ctx->randcnt=FLUX_ISAAC_RANDSIZ;  /* prepare to use the first set of results */
}

void flux_isaac32(flux_isaac32_ctx *ctx) {
   register uint32_t a,b,x,y,*m,*mm,*m2,*r,*mend;
   mm=ctx->randmem; r=ctx->randrsl;
   a = ctx->randa; b = ctx->randb + (++ctx->randc);

   for (m = mm, mend = m2 = m+(FLUX_ISAAC_RANDSIZ/2); m<mend; )
   {
      rngstep32(uint32_t, a<<13, a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a>>6 , a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a<<2 , a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a>>16, a, b, mm, m, m2, r, x);
   }
   for (m2 = mm; m2<mend; )
   {
      rngstep32(uint32_t, a<<13, a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a>>6 , a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a<<2 , a, b, mm, m, m2, r, x);
      rngstep32(uint32_t, a>>16, a, b, mm, m, m2, r, x);
   }
   ctx->randb = b; ctx->randa = a;
}

/* if (flag==TRUE), then use the contents of randrsl[] to initialize mm[]. */
void flux_isaac32_init(flux_isaac32_ctx *ctx, int flag) {
   int i;
   uint64_t a,b,c,d,e,f,g,h;
   uint32_t *m,*r;
   ctx->randa = ctx->randb = ctx->randc = 0;
   m=ctx->randmem;
   r=ctx->randrsl;
   a=b=c=d=e=f=g=h=0x9e3779b9;  /* the golden ratio */

   for (i=0; i<4; ++i)          /* scramble it */
   {
     mix32(a,b,c,d,e,f,g,h);
   }

   if (flag) 
   {
     /* initialize using the contents of r[] as the seed */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
       e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
     /* do a second pass to make all of the seed affect all of m */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
       e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   }
   else
   {
     /* fill in m[] with messy stuff */
     for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8)
     {
       mix32(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   }

   flux_isaac32(ctx);            /* fill in the first set of results */
   ctx->randcnt=FLUX_ISAAC_RANDSIZ;  /* prepare to use the first set of results */
}
/*
------------------------------------------------------------------------------
isaac64.c: My random number generator for 64-bit machines.
By Bob Jenkins, 1996.  Public Domain.
------------------------------------------------------------------------------
*/
#define ind64(mm,x)  (*(uint64_t *)((uint8_t *)(mm) + ((x) & ((FLUX_ISAAC_RANDSIZ-1)<<3))))
#define rngstep64(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (mix) + *(m2++); \
  *(m++) = y = ind64(mm,x) + a + b; \
  *(r++) = b = ind64(mm,y>>FLUX_ISAAC_RANDSIZL) + x; \
}

void flux_isaac64(flux_isaac64_ctx *ctx) {
	register uint64_t a,b,x,y,*m,*m2,*r,*mend;

	r = ctx->randrsl;
	a = ctx->randa; b = ctx->randb + (++ctx->randc);

	for (m = ctx->randmem, mend = m2 = m+(FLUX_ISAAC_RANDSIZ/2); m<mend; ) {
		rngstep64(~(a^(a<<21)), a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a>>5)  , a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a<<12) , a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a>>33) , a, b, ctx->randmem, m, m2, r, x);
	}

	for (m2 = ctx->randmem; m2<mend; ) {
		rngstep64(~(a^(a<<21)), a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a>>5)  , a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a<<12) , a, b, ctx->randmem, m, m2, r, x);
		rngstep64(  a^(a>>33) , a, b, ctx->randmem, m, m2, r, x);
	}

	ctx->randb = b;
	ctx->randa = a;
}

#define mix64(a,b,c,d,e,f,g,h) \
{ \
   a-=e; f^=h>>9;  h+=a; \
   b-=f; g^=a<<9;  a+=b; \
   c-=g; h^=b>>23; b+=c; \
   d-=h; a^=c<<15; c+=d; \
   e-=a; b^=d>>14; d+=e; \
   f-=b; c^=e<<20; e+=f; \
   g-=c; d^=f>>17; f+=g; \
   h-=d; e^=g<<14; g+=h; \
}

void flux_isaac64_init(flux_isaac64_ctx *ctx, int flag) {
	int i;
	uint64_t a,b,c,d,e,f,g,h;

	ctx->randa = ctx->randb = ctx->randc = (uint64_t)0;

	a=b=c=d=e=f=g=h=0x9e3779b97f4a7c13LL;  /* the golden ratio */

	for (i=0; i<4; ++i) {                   /* scramble it */
		mix64(a,b,c,d,e,f,g,h);
	}

	for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8) {  /* fill in ctx->randmem[] with messy stuff */
		if (flag) {                 /* use all the information in the seed */
			a+=ctx->randrsl[i  ]; b+=ctx->randrsl[i+1]; c+=ctx->randrsl[i+2]; d+=ctx->randrsl[i+3];
			e+=ctx->randrsl[i+4]; f+=ctx->randrsl[i+5]; g+=ctx->randrsl[i+6]; h+=ctx->randrsl[i+7];
		}
		mix64(a,b,c,d,e,f,g,h);
		ctx->randmem[i  ]=a; ctx->randmem[i+1]=b; ctx->randmem[i+2]=c; ctx->randmem[i+3]=d;
		ctx->randmem[i+4]=e; ctx->randmem[i+5]=f; ctx->randmem[i+6]=g; ctx->randmem[i+7]=h;
	}

	if (flag) {        /* do a second pass to make all of the seed affect all of ctx->randmem */
		for (i=0; i<FLUX_ISAAC_RANDSIZ; i+=8) {
			a+=ctx->randmem[i  ]; b+=ctx->randmem[i+1]; c+=ctx->randmem[i+2]; d+=ctx->randmem[i+3];
			e+=ctx->randmem[i+4]; f+=ctx->randmem[i+5]; g+=ctx->randmem[i+6]; h+=ctx->randmem[i+7];
			mix64(a,b,c,d,e,f,g,h);
			ctx->randmem[i  ]=a; ctx->randmem[i+1]=b; ctx->randmem[i+2]=c; ctx->randmem[i+3]=d;
			ctx->randmem[i+4]=e; ctx->randmem[i+5]=f; ctx->randmem[i+6]=g; ctx->randmem[i+7]=h;
		}
	}

	flux_isaac64(ctx);          /* fill in the first set of results */
	ctx->randcnt = FLUX_ISAAC_RANDSIZ;    /* prepare to use the first set of results */
}
