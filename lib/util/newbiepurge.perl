#!/usr/bin/perl

open(FLIST,"find ./* -name \'*.dat\' -mtime +20 |") || die ("Couldn't exec find!\n");
$count = 0;

while (<FLIST>) {
    chop;
    @filelist[$count] = $_ ;
    $count++ ;
}
close(FLIST);

foreach $name (@filelist) {
    open(CHECKFILE,"./showlvl $name |") || die ("Couldn't exec ptest $name!\n");
    $moveit = 0;
    
    while (<CHECKFILE>)     {
	if ($_ =~ /^.*LVL : 1 .*$/ ) {
	    $moveit = 1 ;
	}
    }
    close(CHECKFILE);

    if ($moveit == 1) {
	system("mv $name ../PURGENEWBIE");
	}
}
