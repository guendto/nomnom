#!/bin/sh

# This script will commit ChangeLog temporarily so that git-archive
# picks the updated ChangeLog to the tarball.

project=nomnom
descr=`git describe --abbrev=4`
basename="$project-$descr"
tarball="$basename.tar.gz"

gen_manual()
{
  echo "Generate manual..."
  MAN=man1/nomnom.1 ; POD=$MAN.pod
  pod2man -c "nomnom manual" -n nomnom -s 1 -r $descr $POD $MAN
  return $?
}

echo "WARNING!

Please note that this script will call git-reset (--hard) during
this process so be sure to commit your changes before you run this
script.

Last chance to bail out (CTRL-C) before we call 'git reset --hard'."
read n1

echo "Generating $tarball ..."

gen_manual &&
git log --stat --name-only --date=short --abbrev-commit >ChangeLog &&
git commit -q man1/nomnom.1 ChangeLog -m 'foo' &&
git archive HEAD --prefix="$basename/" | gzip >"$tarball" &&
git reset -q --hard HEAD~1
