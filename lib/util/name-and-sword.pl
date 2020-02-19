#!/usr/bin/perl

if (open(BIGLIST,"eq-allfiles")) {
    print("Successfully opened eq-allfiles.\n");
    if(open(SWORDSALL,">swords-all")) {
	print("Successfully opened new file swords-all.\n");
	$line = <BIGLIST>;
	while ($line ne "") {
	    if ( $line =~ /\.\/[A-Z].*$/ ) {
		print SWORDSALL "$line";
	    } 
	    elsif ( $line =~ /#11317.*$/ ) {
		    print SWORDSALL "$line";
		}
	    $line = <BIGLIST>;
	}
	close(SWORDSALL);
    }
    close(BIGLIST);
}
