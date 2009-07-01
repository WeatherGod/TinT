#! /usr/bin/perl


use DBI;				# for database connectivity
use strict;

my $WAC_ADMIN_SYS = 'ls2.meteo.psu.edu';

use sigtrap 'handler' => \&CleanAndExit, 'INT', 'ABRT', 'QUIT', 'TERM';

use Getopt::Long;			# for command-line parsing
$Getopt::Long::ignorecase = 0;		# options are case-sensitive

#default parameters all set to undef
my ($VolumeName, $MediaType, $ServerName, $DirName) = (undef, undef, undef, undef);

if (!GetOptions(
	"volume|v=s"	=> \$VolumeName,
	"media|m=s"	=> \$MediaType,
	"server|s=s"	=> \$ServerName,
	"directory|d=s"	=> \$DirName,
	"help|h"	=> sub { PrintHelp();
				 exit(2); },
	"syntax|x"	=> sub { PrintSyntax();
				 exit(1); }))
{
	PrintSyntax();
	exit(1);
}

if (!defined($VolumeName) || !defined($MediaType) || !defined($DirName) || !defined($ServerName))
{
	print STDERR "ERROR: VolumeName, MediaType, ServerName and Directory/Drive names are all required\nat the command line!\n";
	PrintSyntax();
	exit(1);
}

my $SysName = `/bin/uname -n`;
chomp($SysName);
if ($SysName ne $WAC_ADMIN_SYS)
{
	print STDERR "ERROR: You must be on $WAC_ADMIN_SYS system in order to administer the WAC database\n";
	exit(1);
}

my $WACDir = $ENV{WAC_HOME};
if ($WACDir eq "")
{
	print STDERR "WARNING: WAC_HOME environment variable is not set!  Defaulting to '.'\n";
	$WACDir = '.';
}



#I want to make this connection a global connection, so I can disconnect in case of a sigint.
#  Note, that for some reason, I can't make this global without explicitly stating its package name.
$main::DBHandle = DoConnection();

if (!defined($main::DBHandle))
{
	print STDERR "ERROR: Connection failed!\n";
	exit(1);
}


my $VolumeNameQuoted = $main::DBHandle->quote($VolumeName);
my $MediaTypeQuoted = $main::DBHandle->quote($MediaType);
my $ServerNameQuoted = $main::DBHandle->quote($ServerName);
my $DirNameQuoted = $main::DBHandle->quote($DirName);


if ($main::DBHandle->do("SELECT VolumeName FROM Volume_Info WHERE VolumeName = $VolumeNameQuoted && Status >= 2") > 0)
{
	print STDERR "ERROR: This Volume has already been listed successfully!\n";
	$main::DBHandle->disconnect();
	exit(1);
}



# The IGNORE is in case I have to relist the volume before a successful conclusion to the listing.
if (!defined($main::DBHandle->do("INSERT IGNORE Volume_Info (VolumeName, SystemName, Directory, MediumType, BlockSize, Status) VALUES($VolumeNameQuoted,$ServerNameQuoted,$DirNameQuoted,$MediaTypeQuoted,\\N,'New')")))
{
	print STDERR "ERROR: Could not add the volume to the database!\n";
	$main::DBHandle->disconnect();
	exit(1);
}

my @SysArgs = Esc("-v=$VolumeName", "-m=$MediaType", "-s=$ServerName", "-d=$DirName");
my $BaseCommand = '';
my $ErrDivision = 1;

if ($SysName ne $ServerName)
{
	print "When prompted, enter your user login password for the system $ServerName\n";

	# need to quote the arguements so that it can pass through ssh without losing the escapings
	@SysArgs = Quote(@SysArgs);

	$BaseCommand = "ssh $ServerName";
	$ErrDivision = 256;
}

my $ListingStatus = -1;

if ($MediaType eq 'DLT' || $MediaType eq 'Ultrium')
{
	$ListingStatus = system("$BaseCommand $WACDir/Commands/TapeLister.sh @SysArgs") / $ErrDivision;
}
elsif ($MediaType eq 'Disk')
{
	$ListingStatus = system("$BaseCommand $WACDir/Commands/DiskLister.pl @SysArgs") / $ErrDivision;
}
else
{
	print STDERR "ERROR: Unknown media type: $MediaType\n";
	$main::DBHandle->disconnect();
	exit(1);
}

if ($ListingStatus == 0)
{
	## Note, by here, $VolumeName should already have been taken through the quoter
	if ($main::DBHandle->do("UPDATE Volume_Info SET Status = 'Listed' WHERE VolumeName = $VolumeNameQuoted") != 1)
	{
        	print STDERR "ERROR: Could not update the status of the volume in the database!\n";
	        $main::DBHandle->disconnect();
        	exit(1);
	}
}
else
{
	print STDERR "ERROR: Problem with the lister.  ErrCode: $ListingStatus\n";
	$main::DBHandle->disconnect();
	exit(1);
}


$main::DBHandle->disconnect();
exit(0);



#############################################################################################################################
# DoConnection()
# Purpose -- returns a connection handle to the mysql database
# tries to do it securely, but I am not trained to do secure programming.
sub DoConnection
{
	my $Pass;
	open (TTY, "/dev/tty") or die "\nERROR: Cannot open terminal\n";
	system("stty -echo < /dev/tty");
	print STDERR "Enter WAC database Password: ";
	chomp ($Pass = <TTY>);
	system("stty echo < /dev/tty");
	close(TTY);
	print STDERR "\n";

	my $DataSource = "DBI:mysql:WX_ARCHIVE_CATALOG;mysql_read_default_file=$ENV{WAC_HOME}/ConfigFiles/.WACmysql.cnf;mysql_read_default_group=admin";

#Connect to server...
	my $DBHandle = DBI->connect($DataSource, "", $Pass);

	# probably a million better ways to do this, but, oh well...
	$Pass = '0000000000000000000000000000000000000000000000000000000000000000000000000000';

	return($DBHandle);
}

sub CleanAndExit
{
	# This function assumes a global variable $DBHandle has been declared.
	print STDERR "\nCaught signal interrupt, cleaning up connection...\n";
	if (defined($main::DBHandle))
	{
		$main::DBHandle->disconnect();
	}

	exit(1);
}


sub PrintSyntax
{
	print STDERR "ListVolume.pl --volume | -v VolumeName  --media | -m MediaType\n";
	print STDERR "              --directory | -d DirName --server | -s ServerName\n";
	print STDERR "              [--help | -h] [--syntax | -x]\n\n";
}



sub PrintHelp
{
	PrintSyntax();

	print STDERR "This program will prepare WAC for a new volume and will list that volume.\n\n";
}


sub Esc {
	foreach my $Word (@_)
	{
		$Word =~ s/([;<>\*\|`&\$!#\(\)\[\]\{\}\\:'" ])/\\$1/g;
	}

	return(@_);
}

sub Quote {
	foreach my $Word (@_)
	{
		$Word = "'$Word'";
	}

	return(@_);
}
