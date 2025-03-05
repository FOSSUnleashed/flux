<|./bin/build
<mk/xs
<mk/c

MKSHELL = rcxs

ARFLAGS	= -rv
AR	= ar

BIN = $BUILD/o.pread $BUILD/o.fact $BUILD/o.s $BUILD/o.9dnd
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

build/any/include/tpl/%.h: build/any/include/tpl/ tpl/%.html
	set reqs <={filter {|x| ~ $x *.html} $prereq}
	sed -Ez 's/\n/\\n/g; s/"/\\"/g; s/&/&amp;/g' $reqs | sed 's/.*/#define TPL_'^$stem^' "&"/' > $target

mktest:V:
	set foo bar
	do false
	echo $foo
	var target alltarget pid nproc prereq

http: http.c build/any/include/tpl/index.h
	$CC $CFLAGS $LDFLAGS <={filter {|x| ~ $x *.c} $prereq } $LDLIBS -o $target

tprox: tprox.c
	$CC $CFLAGS $LDFLAGS $prereq $LDLIBS -o $target

$BUILD/o.%: $BUILD/cmd/%.o $LIBS
	$CC $LDFLAGS $TARGET_ARCH <={filter {|x| ~ $x *.o} $prereq } $LDLIBS -o $target

$BUILD/cmd/%.o: cmd/%.c include/new.h
	access -d $BUILD/cmd/^`{dirname $stem} || mkdir -p $BUILD/cmd/^`{dirname $stem}
	$CC $CFLAGS $TARGET_ARCH -c -o $target <={filter {|x| ~ $x *.c} $prereq }

build/%/:
	mkdir -p build/$stem

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

all:V: build/any/ $DIRS $LIBS $BIN $MAN
