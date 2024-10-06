#!/bin/sh

echo "Configure les sources de C6809 pour LINUX..."

echo "# generated by fixlinux.sh" > makefile
echo "MAKEFILE_INC = makefile.lnx" >> makefile
echo "include makefile.all" >> makefile

# Convert files from Windows/MS-DOS (CR+LF) or Macintosh (CR) to Unix (LF)
# Written by Prehisto
# with the help of Postmortem from https://forum.ubuntu-fr.org/
# Please make tests before using

find . -type d '(' -name '.hg' -prune ')' -o \
       -type f '(' -name 'fixunix.sh' -prune ')' -o \
       -type f '(' -name '*.c' -o \
                   -name '*.h' -o \
                   -name 'makefile.*' -o \
                   -name '*.txt' -o \
                   -name '*.bat' -o \
                   -name '*.BAT' \
                   ')' \
       -exec sh -c 'printf '\'\\r%s%15s\\r\'' {} '\'\'';
                    grep -q $(printf '\'\\r\'') {};
                    if [ $? -eq 0 ];
                    then
                        mv {} _tmpfile;
                        tr -d '\'\\n\'' < _tmpfile > _tmpfile2;
                        tr '\'\\r\'' '\'\\n\'' < _tmpfile2 > {};
                        touch -r _tmpfile {};
                        rm _tmpfile2;
                        rm _tmpfile;
                    fi' \;

find . -type f '(' -name 'fixunix.sh' -prune ')' -o \
       -type f '(' -name '*.sh' -o \
                   -name '*.exe' -o \
                   -name 'c6809' \
                   ')' \
       -exec sh -c 'printf '\'\\r%80s\\r%s\'' '\'\'' {};
                    chmod +x {}' \;
echo
