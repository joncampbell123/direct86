#!/usr/bin/perl
#
# clean up build cruft
system("./rename.pl");
system("rm -Rfv Debug Release");

