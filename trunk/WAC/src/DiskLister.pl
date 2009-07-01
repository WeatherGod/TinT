#! /usr/bin/perl -w

use strict;
use Getopt::Long;       # for command-line parsing
$Getopt::Long::ignorecase = 0;  # options are case-sensitive

#### This program takes the recursive listing result of a SINGLE VOLUME on the data server and makes it into a listing file for that volume.

my ($VolumeName, $ServerName, $DirName, $MediaType) = (undef, undef, undef, undef);

if (!GetOptions(
	"volume|v=s"	=> \$VolumeName,
	"server|s=s"	=> \$ServerName,
	"directory|d=s" => \$DirName,
	"media|m=s"	=> \$MediaType,
	"help|h"	=> sub { PrintHelp();
				 exit(1); },
	"syntax|x"	=> sub { PrintSyntax();
				 exit(2); }	))
{
	PrintSyntax();
	exit(2);
}


if (!defined($VolumeName) || !defined($ServerName) || !defined($DirName) || !defined($MediaType))
{
	print STDERR "ERROR: VolumeName, ServerName, Directory name, and Media type are all required at the command line!\n";
	PrintSyntax();
	exit(1);
}

my $WorkingDir = $ENV{WAC_HOME};
if ($WorkingDir eq '')
{
	print STDERR "WARNING: the WAC_HOME environment variable is not set!  Using the default './'\n";
	$WorkingDir = ".";
}


# May need to eliminate redundant slashes
my $ListingFileLoc = "$WorkingDir/ToBeCatalogued/";
my $VolumeLocation = "$DirName/$VolumeName/";
MakeVolumeListings($ServerName, $VolumeLocation, $ListingFileLoc, $VolumeName, $MediaType);

PrintExit();

exit(0);

##--------------------------- Functions ----------------------------------------------------------------------------------------------
sub PrintSyntax
{
	print STDERR "DiskLister -v | --volume VOLUMENAME   -s | --server SERVERNAME   -d | --directory DIR   -m | --media MEDIATYPE\n";
	print STDERR "           [--help | -h] [--syntax | -x]\n\n";
}

sub PrintHelp
{
	PrintSyntax();

	print STDERR "This program will produce a listing file for a volume stored on a disk server.\n\n";
}

sub PrintExit
##---------- May want to use this function to print out a conclusion of everything done for review
{
	print "\n\t\t****** Exiting the Disk Listing Program ************\n\n";
}

sub MakeVolumeListings
{
	my ($SystemName, $Directory, $ListingFileLoc, $VolumeName, $MediaType) = @_;

	my $LineRead = "";
	my $VolumeExists = 1;		## assume the volume exists until proven otherwise

	my ($Directory_Safe, $ListingFileLoc_Safe, $VolumeName_Safe) = Esc($Directory, $ListingFileLoc, $VolumeName);
	my $CommandStr = "ls -lR ${Directory_Safe} > ${ListingFileLoc_Safe}/VolumeLists_${VolumeName_Safe}.txt";

	if (-e $Directory)
	{
		`$CommandStr`;				## this will put the listing in the proper place
#		print "--^^ Found Volume\n";
		open(VOLLIST, "<$ListingFileLoc/VolumeLists_$VolumeName.txt") 
							or die "\nERROR: couldn't open the file: $ListingFileLoc/VolumeLists_$VolumeName.txt\n";

		my $FileName = "$ListingFileLoc/$VolumeName.disklisting";

		open(DISKLIST, ">$FileName") or die "\nERROR: Couldn't open the file: $FileName\n";

		PrintInitialInfo(*DISKLIST, $VolumeName, $SystemName, $Directory, $MediaType);

#		print "--^^ Printed Initial Information\n";
	
		$Directory =~ s/(\/)+$//;                     ## strips any directory slashes from the end of the name
		my $SequenceNumber = "";
		my $SubDirectoryName = "";

		while ($LineRead = <VOLLIST>)
		{
			chomp($LineRead);
#			print "LineRead: $LineRead\n";
			if ($LineRead =~ /^$Directory\/File(\d+)\/?:$/)  ## positive test for beginning of a listing for a sequenced file in the volume
        	        {
			
                	        $SequenceNumber = $1;
				$SubDirectoryName = "";
#				print "Found a sequenced file value: $SequenceNumber\n";

				PrintDataEntry(*DISKLIST, $SequenceNumber);
			}
			elsif ($LineRead =~ /^$Directory\/File(\d+)\/([\w\/]+?)\/?:$/)  ## positive test for sub-directory
			{
				$SubDirectoryName = $2;
				my $TempSequenceNumber = $1;
#				print "Found a sub-directory name: $SubDirectoryName\n";
				if ($TempSequenceNumber ne $SequenceNumber)
				{
#					print "--^^ Doing it this way\n";
					$SequenceNumber = $TempSequenceNumber;
					PrintDataEntry(*DISKLIST, $SequenceNumber);
				}

			}
			elsif ($LineRead =~ /^$Directory\/File(\D+)(\/?([\w\/]+?)\/?)?:$/)  ## positive test for an invalid directory
			{
				print STDERR "\nERROR: Bad Directory.\n$LineRead\n\n";
				exit(1);
			}
			elsif ($LineRead =~ /^[dl\-][r\-][w\-][x\-][r\-][w\-][x\-][r\-][w\-][x\-]/)
			{
				if ($SequenceNumber ne "")
				{
					my @LineList = split(/\s+/, $LineRead);
					my $ShortFileName = $LineList[$#LineList];	## file name will be the last item in the array.
					my $Replacement = "$SubDirectoryName"."/"."$ShortFileName";
					$Replacement =~ s/^(\/)+//;			## strips any directory slashes from the beginning of the name
#					print "Found shortfile name in $LineRead  : $ShortFileName\n";
#					print "The replacement is |||$Replacement|||\n\n";
			
					$LineRead =~ s/\b$ShortFileName/$Replacement/;  #Replacing the name in the `ls` listing
#					print "Now it is $LineRead\n\n";
			
					print DISKLIST "$LineRead\n";
				}
			}
		}  ## while loop reading the ls listing file

		close DISKLIST;
		close VOLLIST;
	}
	else
	{
		print STDERR "\nERROR: We could not find your volume, $VolumeName\n";
	}

}

#sub PrintFileListBeginning
#{
#	my ($FileName, $FileNumber) = @_;
#
#	open (DISKLIST, ">>$FileName");
#
#	print DISKLIST "---^^^---File #:  $FileNumber\n";
#	print DISKLIST "---^^^---Data:\n";
#	print DISKLIST "---^^^---Listing:\n";
#
#	close DISKLIST;
#}

sub PrintInitialInfo
{
	my ($FileHandle, $VolumeName, $SystemName, $Directory, $MediaType) = @_;

	print $FileHandle `date`."\n";

	print $FileHandle "---^^^---MediaLocation:\n";
	print $FileHandle "$MediaType:$VolumeName,$SystemName,$Directory\n\n";
}

sub PrintDataEntry
{
	my ($FileHandle, $SequenceNumber) = @_;                                                                                                                                                             
        print $FileHandle "\n---^^^---File #:  $SequenceNumber\n";
        print $FileHandle "---^^^---Data:\n";
        print $FileHandle "---^^^---Listing:\n";
}

sub Esc {
        foreach my $Word (@_)
        {
                $Word =~ s/([;<>\*\|`&\$!#\(\)\[\]\{\}\\:'" ])/\\$1/g;
        }

        return(@_);
}

