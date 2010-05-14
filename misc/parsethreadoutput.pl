#!/usr/local/bin/perl -w

use strict;
use Time::ParseDate;
use Getopt::Long;

# Usage is described properly in sub usage()

(my $cmdName = $0) =~ s/^.*\///;
(my $cmdBase = $cmdName) =~ s/\..*//;

my $help        = 0;
my $inputFile   = undef;

sub usage {
    my $msg = shift;
    print STDERR "$msg\n" if ($msg);
    print STDERR 
"Usage: $cmdName\n\n",
"The output from the clusterlib logs go into a single file.  This program\n",
" will split the output per thread.\n\n",
"Options:\n",
"  [-help]\n",
"  [-inputFile] <file to split the output>\n";
    exit(1);
}

# Parse the options
GetOptions('help'          => \$help,
           'inputFile=s'   => \$inputFile,
)
    or usage();

usage() if $help || !defined($inputFile);

# Parse the input file into multiple thread based files
my $unknownThreadKey = "unknown";
my %threadHash = ();
my %threadDateHash = ();
my %threadNameHash = ();
my %threadIdHash = ();
open(FH, "< $inputFile") || die "Couldn't open file $inputFile";
my $count = 0;
my $tmpOutputFilename = "";
my $threadKey = "";
while (<FH>) {
    # $1 = date $2 = time $3 = milliseconds $4 = thread
    if ($_ =~ m/^(\d+\-\d+\-\d+) (\d+\:\d+\:\d+),(\d+) \[(\w+)\]/) {
	$threadKey = $4;
    }
    else {
	$threadKey = $unknownThreadKey;
    }

    if ($threadKey ne $unknownThreadKey) {
	$threadDateHash{$threadKey} = parsedate("$1 $2");
    }

    # Main user thread
    if ($_ =~ /msecs, thread: (\d+)/) {
        $threadNameHash{$threadKey} = "main()";
        $threadIdHash{$threadKey} = $1;
    }

    # Ancillary thread
    if ($_ =~ m/Starting thread with (\w+::\w+\(\))/) {
        $threadNameHash{$threadKey} = $1;
        if ($_ =~ m/thread: (\d+)/) {
            $threadIdHash{$threadKey} = $1;
        }
    }

    if (!exists $threadHash{$threadKey}) {
	$tmpOutputFilename = $inputFile.".".$threadKey;
	print STDOUT "Found new thread $threadKey\n";
	open ($threadHash{$threadKey}, "> $tmpOutputFilename") || 
	    die "Couldn't open file $tmpOutputFilename";
    }
    print {$threadHash{$threadKey}} $_;
}
close(FH) || die "Couldn't close file $inputFile";
for $threadKey (keys %threadHash) {
    close($threadHash{$threadKey}) || 
	die "Couldn't close file for key $threadKey";
}
print STDOUT "\n\n";

# Sort the last dates and print which threads have been not reporting
# for awhile
my @dateArr;
for $threadKey (keys %threadDateHash) {
    push @dateArr, $threadDateHash{$threadKey};
}
@dateArr = sort(@dateArr);
my $tmpThreadFunction = "";
my $tmpThreadId = "";
for (my $i = 0; $i < scalar(@dateArr); ++$i) {
    for $threadKey (keys %threadDateHash) {
	if ($threadDateHash{$threadKey} == $dateArr[$i]) {
            if (exists $threadNameHash{$threadKey}) {
                $tmpThreadFunction = $threadNameHash{$threadKey};
            }
            else {
                $tmpThreadFunction = "unknown";
            }
            if (exists $threadIdHash{$threadKey}) {
                $tmpThreadId = $threadIdHash{$threadKey};
            }
            else {
                $tmpThreadId = "";
            }
	    $tmpOutputFilename = $inputFile.".".$threadKey;
	    print STDOUT "Thread $threadKey\n  file: $tmpOutputFilename\n",
	    "  last output date: ", scalar(localtime($dateArr[$i])), 
            "\n  thread function: ", $tmpThreadFunction,
            "\n  thread id: ", $tmpThreadId,
	    "\n\n";
	    delete $threadDateHash{$threadKey};
	    last;
	}
    }
}

exit(0);
