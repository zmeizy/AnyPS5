#!/bin/bash
# usage: ANYPS5_REPO=<clone> newconflicts.sh <branch> <pr numbers...>
# needs refs/remotes/upstream/main and refs/prm/<n> (fetched from +refs/pull/<n>/merge)
cd "${ANYPS5_REPO:-.}"
branch=$1; shift
for n in "$@"; do
  base=$(git rev-parse -q --verify refs/prm/$n^1) || continue
  a=$(git merge-tree --write-tree --name-only --merge-base=$base upstream/main refs/prm/$n 2>/dev/null | sed -n '2,$p' | grep -v '^$' | grep -v '^Auto-merging\|^CONFLICT' | sort -u)
  b=$(git merge-tree --write-tree --name-only --merge-base=$base $branch refs/prm/$n 2>/dev/null | sed -n '2,$p' | grep -v '^$' | grep -v '^Auto-merging\|^CONFLICT' | sort -u)
  new=$(comm -13 <(echo "$a") <(echo "$b") | grep -v '^$')
  [ -n "$new" ] && echo "PR $n: new conflicts: $new"
done
echo checked
