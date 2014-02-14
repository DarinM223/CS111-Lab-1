#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

#there is no parallelism in subshells so we are taking advantage that (echo hello ; echo world) will
#print out hello first then world
#also this test uses sleep a lot so it might take some time to run it


tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
#start script
(sleep 3 ; echo fail) ; (sleep 2 ; echo not)
(sleep 1; echo hello > a; echo yolo) ; (sleep 5 ; cat a)
echo hello > a; echo hi > a ; (cat a ; rm a)
(sleep 4 ; echo DUNDUNDUNNNNN) #is much much slower without parallelism
(echo this_goes_last > a ; cat a ; rm a)
echo this_goes_first

EOF

cat >test.exp <<'EOF'
this_goes_first
yolo
not
fail
DUNDUNDUNNNNN
hello
hi
this_goes_last
EOF

../timetrash -t test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
