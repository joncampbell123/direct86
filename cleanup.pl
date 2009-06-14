#!/usr/bin/perl
#
# clean up build cruft
system("./rename.pl");
system("cd win95 && rm -Rfv Debug Release debug release *.opt *.plg *.ncb");

