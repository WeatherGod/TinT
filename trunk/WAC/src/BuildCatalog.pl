#! /usr/bin/perl -w

use Getopt::Long;

if (!GetOptions(
	'help|h'	=> sub { PrintHelp();
				 exit(1); },
	'syntax|s'	=> sub { PrintSyntax();
				 exit(2);}      ))
{
	PrintSyntax();
	exit(2);
}

############################################################ Building the Listing RegularExpression ##############################################
#  I am putting this here as a way of self documenting the RegularExpression string that I have built for this program.
#  It should also make it easier to edit and improve upon if it is up here.
my $PermissionRegEx = '([-dl][-r][-w][-x][-r][-w][-x][-r][-w][-x])';
my $HardLnkRegEx = '(\s+\d+)?';
my $OwnerGroupRegEx = '(\w+)[\/\s]+(\w+)';
my $FileSizeRegEx = '(\d+)';
my $DateStrRegEx = '([a-zA-Z]{3}\s+[\d]{1,2}\s+\d\d:\d\d|[a-zA-Z]{3}\s+[\d]{1,2}\s+\d\d\d\d|\d\d\d\d-\d\d-\d\d\s+\d\d:\d\d:\d\d)';
my $FileNameRegEx = '(.+)';
my $ListingRegEx = "^${PermissionRegEx}${HardLnkRegEx}\\s+$OwnerGroupRegEx\\s+$FileSizeRegEx\\s+$DateStrRegEx\\s+$FileNameRegEx\$";
###################################################################################################################################################

my $WACDir = $ENV{WAC_HOME};
if ($WACDir eq "")
{
        print STDERR "WARNING: WAC_HOME environment variable is not set!  Defaulting to '.'\n";
        $WACDir = '.';
}


print "\nStarting Processing!\n";
my @Time = gmtime;
print "GMT: $Time[2]:$Time[1].$Time[0]\n";

opendir (WORK, "$WACDir/ToBeCatalogued") || Error ('open', 'directory');
my @WorkList = readdir(WORK);

open (CATA, ">>$WACDir/ToBeSorted/Unsorted.catalogue") || Error('open', 'catalogue');	# opens for writing by appending
open (LOCATION, ">>$WACDir/ToBeLoaded/Volumes.locator") || Error('open', 'Volumes.locator');	# opens for writing by appending

foreach my $FileNme (@WorkList)
{
   if (index($FileNme, "listing") != -1)
   {
      my $KeepRead = 1;				#'boolean' variable.  True = 1 = keep reading, False = end of file
      my $SkipNextRead = 0;                     #'boolean' variable.  True = 1 = don't read next line and use the
						#		current value of $FileLine in the next iteration.
      my $FileLine = "";
      my $FileGrpNum = -1;
      my $MediaType = "DLT";		# assume DLT.  If otherwise, it will get changed.  Listing files that do not have a medium identifier
					#		were listings of the DLTs.  Any new listings have the medium identifier.
      my $SysName = "sheeba.met.psu.edu";	# assume sheeba, although many were done on the system taper, but it has been decommissioned.
      my $Drive = "/dev/nst0";		# assume /dev/nst0.  This will not be correct for many tapes, but it is impossible to retrieve this info
					# since it was never recorded at first.
      my $BlockSize = 0;		# assume this blocksize in bytes.  Treated as a property of the volume, but really should be a
					# property of the file group.
					# So, I will just look for the largest BlockSize and record that.  If no blocksize is found,
					# then the blocksize will be recorded as NULL.



      open (LIST, "<$WACDir/ToBeCatalogued/$FileNme") or die "\nERROR: Open failed: $!\n";

      my $VolumeName = $FileNme;
      $VolumeName =~ s/\.(tape|disk)listing//;	#takes the extension off of the file name to be used as the volume name					

      while ($KeepRead == 1)
      {
         if ($SkipNextRead == 0)
         {
            $FileLine = <LIST>;
         }
         else				
         {
	 #the value in the $FileLine variable has not been processed and needs to be used
            $SkipNextRead = 0;	#next time around, there will be a read from the file.
         }

   
         if (index($FileLine, '---^^^---End Of Tape') > -1)
         {
            $KeepRead = 0;
         }
         elsif (index($FileLine, '---^^^---File #:') > -1)
         {
	    # File # really should be FileGroup #, but I made this before I had a full understanding of the structure.
      
            my @SplitLine = split(/:/, $FileLine);		
            $FileGrpNum = $SplitLine[$#SplitLine];		#grabs the last word of the line, which should be the number
            $FileGrpNum =~ s/^\s+//;
	    $FileGrpNum =~ s/\s+$//;				# strips leading and trailing whitespace
         }
         elsif (index($FileLine, '---^^^---Data:') > -1)
	 {
	    $FileLine = <LIST>;
	    
	    if (index($FileLine, '---^^^---Listing:') > -1)
            {
               my $ListingLine = <LIST>;
	       chomp($ListingLine);

	       while (index($ListingLine, '---^^^---') == -1) # keep looping until the program sees this
               {
	          if ($ListingLine =~ m/$ListingRegEx/)
	          {
	             my ($ListPermissionStr, $ListHardLinkCnt, $ListUserOwner, $ListUserGroup, $ListFileSize, $ListFileDateTime, $ListFileName) =
		         $ListingLine =~ m/$ListingRegEx/;
	       	     #NOTE: it is possible for $ListHardLinkCnt to be undefined.  So check if it is defined before using it, if you do use it.
               
               
                     if (!($ListPermissionStr =~ /^d/))  #will ignore directories
	             {
		        #Printing to the catalogue file
		     		     
		        # The Sorter program uses a dumb delimiter algorithm to break up the line based on
		        # commas.  I will assume that the volume name has no commas.
		        # If the ListFileName has commas, it won't be a problem as the
		        # Sorter program will grab the last 3 string elements, and whatever is left
		        # will form the filename.  If you are confused about what I am talking about,
		        # please see the code in Sorter.C
	             
		        print CATA "$ListFileName,$FileGrpNum,$VolumeName,$ListFileSize\n";
                     	#prints the file group number of the file within the tape
	             	#prints the external tape name for that is useful for data requests.
		     	#prints the listed file's filesize in bytes (hopefully)

	             }   #end if(the file is not a directory)
	          }  #end if(ListingLine matches the ListingRegEx)
	          elsif (!($ListingLine =~ m/^total (\d+)(\.[\d]+)?/) && $ListingLine ne ' ')
	          {
	             #Essentially, if the listing line isn't one of two other expected possibilities, then do this block.
	             #I am trying to flag any possible mistakes if the Listing RegEx doesn't match something that it should.
	             print "Should this be matching?  |$ListingLine| Volume: $VolumeName\n";
	          }
	

                  $ListingLine = <LIST>;
	          chomp($ListingLine);
               }# end of while loop

               $FileLine = $ListingLine;
            }# end of if it is a listing label.

            $SkipNextRead = 1;
	    $FileGrpNum = -1;
         }
         elsif (index($FileLine, '---^^^---MediaLocation:') > -1)
         {
            my $TempLine = <LIST>;
	    chomp($TempLine);
	    ($MediaType, $VolumeName, $SysName, $Drive) = $TempLine =~ m/^(.+):(.+),(.+),(.+)$/;
	    chomp($SysName);
            $KeepRead = 1;
            $SkipNextRead = 0;
         }
	 elsif ($FileLine =~ m/^using\s+.+\s+on\s+.+$/)
	 {
	    ## this block exists as a backwards compatibility feature.
	    # Many old tape listings had a line in the file indicating which system and which drive was
	    # used for the listing.  This was before I started to use the MediaLocation line.
	    # Note, though, not all tape listings have this, either!
	    ($Drive, $SysName) = $FileLine =~ m/^using\s+(.+)\s+on\s+(.+)$/;
	    $KeepRead = 1;
	    $SkipNextRead = 0;
	 }
         elsif ($FileLine =~ m/\$TAPE_DIRECTORY_FILE BLOCKSIZE=\d+/)
         {
            ## This block exists as a way to find the blocksize of a file group.
            # admittedly, it will record the blocksize as the blocksize for the entire
            # volume, but that is why I search for the largest value.
            my ($TempSize) = $FileLine =~ m/\$TAPE_DIRECTORY_FILE BLOCKSIZE=(\d+)/;
            if ($TempSize > $BlockSize)
            {
               $BlockSize = $TempSize;
            }

            $KeepRead = 1;
            $SkipNextRead = 0;
         }
         else					#nothing of significance was found, moving on to next line.
         {
            #print "Nothing matched, keep reading\n";
            $KeepRead = 1;
            $SkipNextRead = 0;		#just to make sure things get read around here.
         }
      }
      close (LIST);

      if ($BlockSize == 0)
      {
         $BlockSize = "\\N";	## indicates NULL.
      }

      print LOCATION "$VolumeName,$SysName,$Drive,$MediaType,$BlockSize,Catalogued\n";

      rename("$WACDir/ToBeCatalogued/$FileNme", "$WACDir/Catalogued/$FileNme");
   }# end of if (index($FileNme, "listing") > 0)
}# end foreach statement

print "Finished Processing!\n";
@Time = gmtime;
print "GMT: $Time[2]:$Time[1].$Time[0]\n\n";
close (LOCATION);
close (CATA);

exit;
################################################ End of Program ###############################################

sub PrintSyntax
{
	print STDERR "BuildCatalogue.pl   [--help | -h] [--syntax | -s]\n\n";
}

sub PrintHelp
{
	PrintSyntax();

	print STDERR "This script processes the listing files in the ToBeCatalogued directory\n";
	print STDERR "and generates a file called Unsorted.catalogue in the ToBeSorted directory.\n";
	print STDERR "The processed listing files are then moved to the Catalogued directory.\n";
	print STDERR "Also, a file containing information on the volumes processed is placed in\n";
	print STDERR "a file called Volumes.locator in the Loaded directory.  This file reflects\n";
	print STDERR "the Volume_Info table in the WAC and is useful for backup purposes.\n\n";
}


