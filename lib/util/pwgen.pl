#!/usr/bin/perl -w

########################################################################
##                                                                    ##
##  File: pwgen.pl                                                    ##
##  Orig. Date: Sat Dec 30 02:31:45 EST 2000                          ##
##  Author: Eric Sakowski (Lem) <ski@signalpost.com>                  ##
##  Project: Password protected website                               ##
##  Purpose: Get enc pass and other data from pfiles and sort into a  ##
##           web-usable format                                        ##
##                                                                    ##
########################################################################

use strict;

my $NLPDIR = '/ronin/imperial/lib/rent';
my $UTILDIR = '/ronin/imperial/lib/util';
my $RENTDIR = '/ronin/imperial/lib/rent';


my @files;

foreach (split(//,'ABCDEFGHIJKLMNOPQRSTUVWXYZ')) {
    my $x = $_;
    opendir(DIR,"$RENTDIR/$x") or die "Couldn't opendir $RENTDIR/$x: $!\n";
    my @dir = readdir(DIR);
    closedir DIR;
    foreach (@dir) {
	push(@files,"$RENTDIR/$x/$_") if ($_ =~ /\.dat/);
    }
}


my %info;

foreach (@files) {
    chomp;
    my $foo = `$NLPDIR/nlp $_`;
    chomp($foo);
    my ($name, $level, $passwd, $active) =  split(/\s+/,$foo);
    $info{$name}{level} = $level;
    $info{$name}{passwd} = $passwd;
    $info{$name}{active} = $active;
}

foreach (keys %info) {
    print "$_ $info{$_}{level} $info{$_}{passwd} $info{$_}{active}\n";
}

