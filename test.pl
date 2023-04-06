#!/usr/bin/env perl

use strict;
use warnings;

my $ref_path = "./bench/ref/";
my $test_path = "./bench/";
my $sim = "./simulator/simulator";
my $conv = "./converter/converter";
my $escp = "./uart_escape/uart_escape";

# these testcases only generate output and don't need input
my %tests = (
	"hello", 400,
	#"mandelbrot", 1500000000,
	"qsort", 6000000,
	"md5", 60000,
	"md5_u", 60000,
	"sha256", 80000,
	#"sha256_u", 80000, # only -Os works; needs 17kB program memory for -O2 and 18kB for -O3
	"sha512", 200000,
	#"sha512_u", 400 needs 59kB program memory
);

my %io_tests = (
	"echo", 4000000,
	"calc", 4000000,
	"lz4_comp", 20000000,
	"lz4_dec",  4000000,
);

print "| test       | u c | compr. rate | compr. size | uncompr. size | compr. bw | uncompr. bw | bw rate |\n";
print "+------------+-----+-------------+-------------+---------------+-----------+-------------+---------+\n";

foreach my $test (sort(keys %tests)) {
	printf "| %-10s | ", $test;
	# run the simulator with the uncompressed, convert and run with the compressed
	my $ref = $ref_path . $test . ".out";
	my $binu = $test_path . $test . ".bin";
	my $binc = $test_path . $test . ".comp.bin";
	my $datau = $test_path . $test . ".data.bin";

	my $num_cycles = $tests{$test};

	# simulate the uncompressed binary
	my $outu = `$sim -n $num_cycles -t $test.u.trace $binu $datau`;

	open(REF_FILE, $ref) or die "Couldn't open reference output\n";

	$/ = undef;
	my $ref_out = <REF_FILE>;

	# convert 
	`$conv $binu $binc`;
	
	# simulate the compressed binary
	my $outc = `$sim -c -n $num_cycles -t $test.c.trace $binc $datau`;

	# size of the instruction binaries
	my $usize = -s $binu;
	my $csize = -s $binc;

	my $bandwidthu = -s "$test.u.trace";
	my $bandwidthc = -s "$test.c.trace";

	if ($outu ne $ref_out) {
		print "f ";
	} else {
		print "s ";
	}

	if ($outc ne $ref_out) {
		print "f ";
	} else {
		print "s ";
	}

	printf "|      %3.1f %% |      %6i |        %6i | %9i |   %9i |  %3.1f %% |\n",
		100.0 * ($csize / $usize), $csize, $usize, $bandwidthc, $bandwidthu,
		100.0 * ($bandwidthc / $bandwidthu);

}

# lz4_comp and lz4_dec need escaping
foreach my $test (sort(keys %io_tests)) {
	printf "| %-10s | ", $test;
	# run the simulator with the uncompressed, convert and run with the compressed
	my $ref = $ref_path . $test . ".out";
	my $ref_in = $ref_path . $test . ".in";
	my $binu = $test_path . $test . ".bin";
	my $binc = $test_path . $test . ".comp.bin";
	my $datau = $test_path . $test . ".data.bin";

	my $num_cycles = $io_tests{$test};

	# simulate the uncompressed binary
	my $outu = `cat $ref_in | $escp | $sim -n $num_cycles -t $test.u.trace $binu $datau`;

	open(REF_FILE, $ref) or die "Couldn't open reference output\n";

	$/ = undef;
	my $ref_out = <REF_FILE>;

	# convert 
	`$conv $binu $binc`;
	
	# simulate the compressed binary
	my $outc = `cat $ref_in | $escp | $sim -c -n $num_cycles -t $test.c.trace $binc $datau`;

	# size of the instruction binaries
	my $usize = -s $binu;
	my $csize = -s $binc;

	my $bandwidthu = -s "$test.u.trace";
	my $bandwidthc = -s "$test.c.trace";

	if ($outu ne $ref_out) {
		print "f ";
	} else {
		print "s ";
	}

	if ($outc ne $ref_out) {
		print "f ";
	} else {
		print "s ";
	}

	printf "|      %3.1f %% |      %6i |        %6i | %9i |   %9i |  %3.1f %% |\n",
		100.0 * ($csize / $usize), $csize, $usize, $bandwidthc, $bandwidthu,
		100.0 * ($bandwidthc / $bandwidthu);
}

