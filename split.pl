#!/usr/bin/perl

# To avoid the example code and the documentation drifting
# out of sync, we keep the documentation as comments in the
# files.
# This script reads in a file and copies every non-comment
# line (these starting with "//: ", "/*: " or "#: ") to the
# source file in appskeleton/ and generates a markdown file
# with comments as markdown source and code snippets
# as highlighted code blocks in doc/
# To convert all files, install bash and run
# for i in *.cpp *.h CMakeLists.txt appveyor.yml .travis.yml .bintray.json; do ./split.pl $i; done;

use strict;

@ARGV[0] =~ m/^.*\.(.*)$/ || die;
my $outfilename = $&;
my $docblock = 0;
my $blockcomment = 0;
my $filetype = $1;
if($1 eq 'h') {
	$filetype = 'cpp';
} elsif($& eq 'CMakeLists.txt') {
	$filetype = 'cmake';
}

open(my $infile, '<', @ARGV[0]) || die;
open(my $outfile, '>', 'appskeleton/' . $outfilename) || die;
open(our $docfile, '>', 'doc/' . @ARGV[0] . '.md') || die;

our @blocks;

sub pushline {
	push(@blocks, @_);
}


while(<$infile>) {
	# /* */ comment in a single line
	if(/^\s*\/\*:(.*)\*\//) {
		pushline('doc', $1);
	} elsif(/^\s*\/\*: (.*)/) {
		pushline('doc', $1);
		$blockcomment = 1;
	} elsif($blockcomment) {
		/^\s*\*?\s(.*?)(?:\*\/\s*)?$/;
		pushline('doc', $1);
		$blockcomment = !/\*\//;
	} else {
		if(/^\s*$/) {
			pushline('empty', '');
		} elsif(/^\s*(?:\/\/|#): ?(.*)?/) {
			pushline('doc', $1);
		} else {
			chomp;
			pushline('content', $_);
		}
	}
}

my $lasttype='doc';
my $skipemptyline = 1;
while (my($type, $value) = splice(@blocks, 0, 2)) {
	# print('Type "' . $type . '": "' . $value . "\"\n");
	if($type eq 'empty') {
		print $outfile "\n" unless $skipemptyline;
		$skipemptyline = 0;
		next; # skip other conditions
	}
	if($type eq 'doc' and $lasttype ne $type) {
		print $docfile "```\n\n";
	}
	if($type eq 'content') {
		print $docfile "\n``` $filetype\n" unless $lasttype eq $type;
		print $outfile $value . "\n";
	}
	print $docfile $value . "\n";
	$skipemptyline = 1 unless $lasttype eq $type;
	$lasttype = $type;
}
