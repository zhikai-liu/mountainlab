#!/bin/sh

for test in tst_*
do
  ./$test $@
done
