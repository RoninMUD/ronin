#!/usr/bin/perl

####################################
##  rent.pl by Lem, Aug 23, 1998  ##
##  Args are of the form Aug 23   ##
####################################

$todaymon = shift @ARGV;
$todaynum = shift @ARGV;

if ($todaynum =~ /[0-9][0-9]/) {
    $today = "\"$todaymon $todaynum\"";
} elsif ($today =~ /[0-9]/) {
    $today = "\"$todaymon  $todaynum\"";
}

system("/usr/bin/grep $today syslog syslog.last | /usr/bin/grep 'has rent' > tmp.lem");

open (TMP, "tmp.lem") || die ("Couldn't open tmp.lem!\n");

while (<TMP>) {
    @tmp = split(/ /,$_);
    $total += @tmp[14];
}
print "Total rent for $today so far was $total\n";
close TMP;

system "/usr/bin/cat /dev/null > tmp.lem";
exit 0;


