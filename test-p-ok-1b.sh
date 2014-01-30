#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
#start script
cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo failed
(cat ../README || echo nope.avi) && echo swag
(cat filenotfound || echo 

noticemesenpaiii) | sort ; echo ORLY && echo YARLY 
TROLOLOL || 




echo NOWAI && echo FUUUUU
( (cat < ../README | sort
 | sed -n /Darin/p > protestcasewriter.txt) ;)
cat protestcasewriter.txt ; echo the guy who loves writing dumb test cases ; rm protestcasewriter.txt

EOF

cat >test.exp <<'EOF'
Lingyu Zhang < 404205755 >
Darin Minamoto < 704140102 >

Limitations:
- Our program treats semicolons after complete commands as syntax error.
  e.g. cat a;
       EOF
  Reports a syntax error at line 2.

Our choice of implementation if the spec is unclear:
- Newlines after complete commands separate 2 commands if outside of subshell.
  e.g. cat a
       echo b
  are two separate commands in command_stream.

- Multiple newlines are treated as a single semicolon if inside of subshell.
  e.g. (cat a
        echo b)
  is interpreted as (cat a; echo b).
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
