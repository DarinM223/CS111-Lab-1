#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
#start script
cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo failed
(cat ../README > out || echo nope.avi) && echo swag
(cat filenotfound || echo noticemesenpaiii) |
sort ; echo ORLY && echo YARLY 
TROLOLOL || 




echo NOWAI && echo FUUUUU
( (cat < ../README | sort |
 sed -n /Darin/p > protestcasewriter.txt) ;)
cat protestcasewriter.txt ; echo the guy who loves writing dumb test cases ; rm protestcasewriter.txt

EOF

cat >test.exp <<'EOF'
swag
noticemesenpaiii
ORLY
YARLY
NOWAI
FUUUUU
Darin Minamoto < 704140102 >
the guy who loves writing dumb test cases
EOF

../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
#test ! -s test.err || {
#  cat test.err
#  exit 1
#}

) || exit

rm -fr "$tmp"
