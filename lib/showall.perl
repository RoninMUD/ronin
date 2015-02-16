#!/usr/bin/perl

open (FILELIST,"ls ./*.dat |") || die ("Couldn't ls ./ !\n");

$count = 0;

while (<FILELIST>) {
	@filelist[$count] = $_ ;
	$count++;
}

foreach $name (@filelist) {
	open (TMPFL,"../rent/showlvl $name |");
	while (<TMPFL>) {
		print ("$_");
	}
	close (TMPFL);
}

