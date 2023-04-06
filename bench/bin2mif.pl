#!/usr/bin/env perl

use strict;
use warnings;
use POSIX;

my $total_size = 0;
my $data = undef;
my $out_hex = "";
my $i = 0;

binmode(STDIN) or die "error while setting STDIN to binary mode";

while (my $num_bytes = read(STDIN, $data, 4)) {
	$total_size += $num_bytes;
	my ($s,$t) = unpack("S>S>*", $data);
	$t //= 0;
	$out_hex = $out_hex . sprintf("\t%8.8x : ", $i);
	$out_hex = $out_hex . sprintf("%4.4x", $s) . sprintf("%4.4x;\n", $t);
	$i = $i + 1;
}

print("WIDTH=32;\n");
print("ADDRESS_RADIX=HEX;\n");
print("DATA_RADIX=HEX;\n");
printf("DEPTH=%i;\n", ceil($total_size/4.0));
print("CONTENT BEGIN\n");
print($out_hex);
print("END;\n");

