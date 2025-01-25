<|./bin/build
<mk/xs
<mk/c

MKSHELL = rcxs

ARFLAGS	= -rv
AR	= ar

BIN = $BUILD/o.pread $BUILD/o.fact
MAN	= man/ipc_connect.3 man/yield.3

# Not sure how much this is needed
TARGET_ARCH = 

LIBS	= $BUILD/lib/libdill.a $BUILD/lib/libflux.a
DIRS	= $BUILD/cmd/niine/

#       $alltarget    all the targets of this rule.
#       $target       the targets for this rule that need to be remade.
#
#       $prereq       all the prerequisites for this rule.
#       $newprereq    the prerequisites that caused this rule to execute.
#       $newmember    the prerequisites that are members of an aggregate that caused this rule to execute.
#                     When the prerequisites of a rule are members of an aggregate, $newprereq contains the
#                     name of the aggregate and out of date members, while $newmember contains only the name
#                     of the members.
#
#       $stem         if this is a meta-rule, $stem is the string that matched % or &.  Otherwise, it is empty.  For regular expression meta-rules (see below), the variables are set to the corresponding subexpressions.
#
#       $nproc        the process slot for this recipe.  It satisfies 0<=$nproc<$NPROC.
#       $pid          the process id for the mk executing the recipe.

all:V:

%.png: %.dot
	dot -Tpng -o $target $prereq

tpl/%.h: tpl/%.html
	sed -Ez 's/\n/\\n/g; s/"/\\"/g; s/&/&amp;/g' $prereq | sed 's/.*/#define TPL_'^$stem^' "&"/' > $target

mktest:V:
	set foo bar
	do false
	echo $foo

termtest: termtest.c
	$CC $CFLAGS $LDFLAGS <={filter {|x| ~ $x *.c} $prereq } $LDLIBS -o $target

http: http.c tpl/index.h
	$CC $CFLAGS $LDFLAGS <={filter {|x| ~ $x *.c} $prereq } $LDLIBS -o $target

tmp: tmp.c
	$CC $CFLAGS $LDFLAGS $prereq $LDLIBS -o $target

ws:V: ws.c
	$CC $CFLAGS $LDFLAGS $prereq $LDLIBS -o $target

tprox: tprox.c
	$CC $CFLAGS $LDFLAGS $prereq $LDLIBS -o $target

$BUILD/o.%: $BUILD/cmd/%.o $LIBS
	$CC $LDFLAGS $TARGET_ARCH <={filter {|x| ~ $x *.o} $prereq } $LDLIBS -o $target

$BUILD/cmd/%.o: cmd/%.c include/new.h
	access -d $BUILD/cmd/^`{dirname $stem} || mkdir -p $BUILD/cmd/^`{dirname $stem}
	$CC $CFLAGS $TARGET_ARCH -c -o $target <={filter {|x| ~ $x *.c} $prereq }

$BUILD/cmd/%/:
	mkdir -p $target


man/%.3: doc/%.md
	scdoc < $prereq > $target

clean:V:
	rm -f cscope.files cscope.out
	find build -type f -delete

<|./bin/srvs
<mk/tests
<mk/flux
<mk/dill
<mk/cscope

all:V: $DIRS $LIBS $BIN $MAN
