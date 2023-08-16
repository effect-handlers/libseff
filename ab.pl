#!/usr/bin/perl

# 
# Copyright (c) 2023 Huawei Technologies Co., Ltd.
# 
# libseff is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2. 
# You may obtain a copy of Mulan PSL v2 at:
# 	    http://license.coscl.org.cn/MulanPSL2 
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.  
# See the Mulan PSL v2 for more details.
#
# A tiny script for tracking performance changes over different revisions.
# You should probably not use this, and it will probably get deleted.
#

use 5.010;
use strict;
use warnings;
use autodie;
use File::Basename;
use HTTP::Date;
use List::Util qw(max);
use Getopt::Long;
use Term::ANSIColor;

my $WARNING = colored(["yellow"], "Warning:");
my $ERROR = colored(["red"], "Error:");

sub prompt {
    my ($query, $verifier, $result_fn) = @_;
    local $| = 1;
    my $answer;
    do {
	print $query;
	chomp($answer = <STDIN>);
    } while (not $verifier->($answer));
    return $result_fn->($answer);
}

sub yes_no {
    my ($query) = @_;
    my @valid = ('Y', 'N');
    my $match = grep(/$_/, @valid);
    return prompt($query,
		  sub { grep(/$_[0]/, ('Y', 'y', 'N', 'n', '')) },
		  sub { grep(/$_[0]/, ('Y', 'y', '')) });
}

sub rev_hash {
    my $rev = shift;
    my %options = @_;

    if ($rev eq 'NOW') { return 'NOW' };

    my $hash = '';
    if ($options{'abbrev'} && $options{'abbrev'} == 1) {
	chomp($hash = `git rev-parse --abbrev-ref $rev`);
	die("Unknown revision $rev") if $?;
    }
    if ($hash eq '') {
	chomp($hash = `git rev-parse --short $rev`);
	die("Unknown revision $rev") if $?;
    }
    return $hash;
}

sub build_branch {
    my ($rev, $hash, $build_args) = @_;

    if (-d "tests-$hash") {
	if ($rev ne 'NOW') {
	    say "tests-$hash already exists, skip building $rev";
	    return;
	} else {
	    chomp(my $last_src_modification_time = `find ./src/ ./asm/ ./tests/ ./decls/ -type f -exec stat \\{} --printf="%y\\n" \\; | sort -n -r | head -n 1`);
	    chomp(my $first_test_modification_time = `find ./tests-NOW -type f -exec stat \\{} --printf="%y\\n" \\; | sort -n -r | head -n 1`);
	    $last_src_modification_time = str2time($last_src_modification_time);
	    $first_test_modification_time = str2time($first_test_modification_time);
	    if ($last_src_modification_time < $first_test_modification_time) {
		say "tests-$hash is newer than sources, skip building $rev";
		return;
	    }
	}
    } 
    say "Building $rev ($hash)";

    if ($rev ne 'NOW') {
	system("git checkout $hash 1>/dev/null 2>&1") == 0 or (unstash() and die);
    };
    system("make clean 1> /dev/null 2>&1") == 0 or (unstash() and die);
    say("make $build_args");
    system("make $build_args 1> /dev/null 2>&1") == 0 or say "$WARNING errors when building $rev";
    system("rm -f output/tests/*.o 1> /dev/null 2>&1") == 0 or (unstash() and die);
    system("mv -f output/tests tests-$hash 1> /dev/null 2>&1") == 0 or (unstash() and die);
}

sub collect_tests {
    my ($hash) = @_;
    return map { basename($_) } glob("tests-$hash/*");
}

sub print_help {
    say if $_ = shift;
    say "Usage: ab.pl SOURCE TARGET [OPTIONS...]";
    say "\t--help\t\t\tshow usage";
    say "\t--summary\t\tsummarize performance changes";
    say "\t--fast\t\t\tsingle-run mode (less precise)";
    say "\t--make-args\t\targuments to 'make'";
    say "\t--exclude testname\texclude the given test-case (can be used multiple times)";
    say "\t--select testname\tselect the given test-case (can be used multiple times)";
    exit();
}
my ($summary, $fast, @excluded_tests, @selected_tests);
my $build_args = "BUILD=release";
GetOptions(
    summary => \$summary,
    fast => \$fast,
    'exclude=s' => \@excluded_tests,
    'select=s' => \@selected_tests,
    'make-args=s' => \$build_args,
    help => sub { print_help; },
) or print_help;
my ($source, $target) = (shift @ARGV, shift @ARGV);
$source or print_help("Must specify a source hash/revision");
$target or print_help("Must specify a target hash/revision");

(scalar(@excluded_tests) == 0 || scalar(@selected_tests) == 0)
    or print_help("Cannot use --select with --exclude");

my $head_hash = rev_hash "HEAD", "abbrev" => 1;
my $source_hash = rev_hash $source;
my $target_hash = rev_hash $target;

build_branch($source, $source_hash, $build_args) if $source eq 'NOW';
build_branch($target, $target_hash, $build_args) if $target eq 'NOW';

`git diff --quiet`;
my $stashed = ($? != 0);
if ($stashed) {
    yes_no("You have unstashed changes, do you wish to stash them and continue? (Y/n) ") or die "Aborting";
    `git stash`;
}
sub unstash {
    `git checkout $head_hash`;
    if ($stashed) {
	`git stash pop`;
    }
}

build_branch($source, $source_hash, $build_args) unless $source eq 'NOW';
build_branch($target, $target_hash, $build_args) unless $target eq 'NOW';

`git checkout $head_hash`;
if ($stashed) {
    `git stash pop`;
}

my @tests_source = collect_tests($source_hash);
my @tests_target = collect_tests($target_hash);

if (scalar(@selected_tests) == 0) {
    my %all_tests;
    @all_tests{@tests_source} = @tests_source x 1;
    @all_tests{@tests_target} = @tests_target x 1;
    @selected_tests = sort keys %all_tests;
}

sub make_cmd {
    my ($program, %args) = @_;
    my $arg_string = "";
    while (my ($flag, $value) = each(%args)) {
	$arg_string = "$arg_string $flag $value";
    }
    return "$program $arg_string";
}
my %hyperfine_common_args = (
    '-N' => '',
);
my %hyperfine_summarize_output_args = (
    '--style' => 'none',
    '--export-csv' => '/dev/stdout'
);
my %hyperfine_full_output_args = (
    '--style' => 'full'
);
my %hyperfine_quick_args = (
    '--warmup' => 0,
    '--min-runs' => 1,
    '--max-runs' => 1,
);
my %hyperfine_detailed_args = (
    '--warmup' => 3,
    '--min-runs' => 15
);

my $test_name_max_length = max (map {length $_} @selected_tests);
sub format_summary_line {
    my ($test, $mean1, $mean2) = @_;
    $mean1 = $mean1 + 0.0;
    $mean2 = $mean2 + 0.0;
    my $ratio = $mean2 / $mean1;
    my $comparison;
    if ($mean1 > $mean2) {
	my $percent = (1.0 - $ratio) * 100.0;
	$percent = sprintf("%.2f", $percent);
	$comparison = colored(["green"], "$percent% better");
    } else {
	my $percent = ($ratio - 1.0) * 100.0;
	$percent = sprintf("%.2f", $percent);
	$comparison = colored(["red"], "$percent% worse");
    }
    printf("%-*s :  %.5fs → %.5fs  (%s)\n", $test_name_max_length, $test, $mean1, $mean2, $comparison);
}

my $hyperfine_cmd = make_cmd "hyperfine",
    %hyperfine_common_args,
    ($summary? %hyperfine_summarize_output_args: %hyperfine_full_output_args),
    ($fast? %hyperfine_quick_args: %hyperfine_detailed_args);

for my $test (@selected_tests) {
    if (grep /$test/, @excluded_tests
	|| !grep /$test/, @tests_source
	|| !grep /$test/, @tests_target) {
	say("$WARNING test $test does not exist in $source ($source_hash)")
	    unless grep /$test/, @tests_source;
	say("$WARNING test $test does not exist in $source ($source_hash)")
	    unless grep /$test/, @tests_target;

	say ("Skipping $test");
	next;
    }
    my $cmd = "$hyperfine_cmd \\
       './tests-$source_hash/$test' -n '$source - $test' \\
       './tests-$target_hash/$test' -n '$target - $test' \\
       2>/dev/null";
    # For some reason, when dumping csv output to stdout, some lines are duplicated
    if ($summary) {
	my $output = `$cmd`;
	if ($? != 0) {
	    say "$ERROR $test failed to run";
	} else {
	    my ($header, $source_times, $repeated, $header2, $target_times) = split(/\n/, $output);
	    my ($source_command, $source_mean) = split(/,/, $source_times);
	    my ($target_command, $target_mean) = split(/,/, $target_times);
	    format_summary_line $test, $source_mean, $target_mean;
	}
    } else {
	system($cmd);
    }
}
