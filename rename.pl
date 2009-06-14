#!/usr/bin/perl
#
# Windows/DOS habitually capitalize or uppercase the names.
# This fixes that
#
# - Jon
open(T,"find |") || die;
while (my $ent = <T>) {
	chomp $ent;
	my $nn = lc($ent);
	if ($nn ne $ent) {
		print "Fixing: $ent\n";
		print "     -> $nn\n";
		rename($ent,$nn);
	}
}
close(T);

