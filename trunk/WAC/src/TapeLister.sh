#!/bin/csh 

####################################################

@ ListingStatus = 0				## zero means that it is a good listing, any other value means that it didn't complete successfully...
@ tarStatus = 0					## not boolean variable, but a status variable
@ somethingweird = 0					## boolean variable
@ FileSkip = 0					## boolean variable
##@ Blockdd = 1572864					## some tapes need this size
@ Blockdd = 2097152					## other tapes need this size

set SkipAt = ""						## A string that will hold the file numbers where skips occur.

##  I am asking for the system name to help keep track which tapes were listed where in case that information is wanted to be
##  known in the future.  Recording which system the tapes were listed on started in the second week of June, 2004, so many tapes do not have this info
##  recorded.  If the tape is a Stream 2, NPNIDS, RAWCONV, RAWCONVRAWGRIB, VIS5D, NOWRAD, or FSLProfiler and does not have a location recorded, it was 
##  most likely listed on taper.  Otherwise, it was probably done on sheeba.


set USAGE = "TapeLister.sh [-s=SYSTEM] [-v=VOLUMENAME] [-d=DRIVE] [-m=MEDIATYPE] [--help] [--syntax]"
set SystemName = ""
set DRIVE = ""
set VolumeName = ""
set TapeType = ""

@ ArgIndex = 1

while ($ArgIndex <= ${#argv})
  switch ($argv[$ArgIndex])
    case --syntax:
    case --help:
      echo "usage:"
      echo "$USAGE"
      exit 1
      breaksw
    case -s=*:
      set SystemName = `echo $argv[$ArgIndex] | sed 's/-s=//'`
      breaksw
    case -v=*:
      set VolumeName = `echo $argv[$ArgIndex] | sed 's/-v=//'`
      breaksw
    case -d=*:
      set DRIVE = `echo $argv[$ArgIndex] | sed 's/-d=//'`
      breaksw
    case -m=*:
      set TapeType = `echo $argv[$ArgIndex] | sed 's/-m=//'`
      breaksw
    default:
      echo "ERROR: Unknown arguement: $argv[$ArgIndex]"
      echo "$USAGE"
      exit 1
      breaksw
  endsw
  @ ArgIndex = $ArgIndex + 1
end

if ($SystemName == "") then
   echo " "
   echo "Please enter the name of the system the tape drive is on:"
   echo -n "--> "
   exit 1
endif

if ($DRIVE == "") then
   echo " "
   echo "No drive name given..."
   exit 1
endif

if ($VolumeName == "") then
   echo " "
   echo "No volume name given..."
   exit 1
endif

if ($TapeType == "") then
   echo " "
   echo "No medium type given..."
   exit 1
endif

set ARCHWORK = "$WAC_HOME"
set ListFile = "$VolumeName.tapelisting"
set JUNKFOLDER = "Junk$VolumeName"
set LogfileName = "${VolumeName}Logfile.finder"

if (!(-w $ARCHWORK)) then
   echo "ERROR: You do not have permissions to write to the $ARCHWORK directory..."
   exit 1
endif

   
mkdir -p $JUNKFOLDER

if ($status != 0) then
   echo "ERROR: You could not make the necessary Junkfolder in your current directory..."
   exit 1
endif

## just to make sure that there is nothing in there.
rm $JUNKFOLDER/*

@ BogusFile = 0		## 0 for false, 1 for true
@ LastTime = -1		## Set to -1 as primer.  Indicates what was the last filegroup number I dealt with
@ GotFirstFileNum = 0	## 0 for false, 1 for true
@ FirstNumOfRun = -1	## this is so I know what filenumber the listing program started its run at.  Sometimes, a problem occurs during
			## a run and the program stops in the middle of the tape.  I would restart the program with the tape in its current
			## position.  If the first file number isn't 0, I would know that that is what happened and I would know to go into
			## the tapelisting file to remove any extra stuff that was printed to the file.

@ MaxRetries = 3	## Maximum number of retries to get to the proper location on the tape

date >> $ARCHWORK/$ListFile
echo "---^^^---MediaLocation:" >> $ARCHWORK/$ListFile
echo "${TapeType}:${VolumeName},${SystemName},${DRIVE}" >> $ARCHWORK/$ListFile
echo " " >> $ARCHWORK/$ListFile

while ( $tarStatus != 3 )				## This check of the tarstatus might be obsolete.  Consider revising.
   @ ExpectFileNum = $LastTime + 1
   echo "*********************************************************************************************" | tee -a $ARCHWORK/$LogfileName
   echo "Listing Status so far:  File Skips = $FileSkip   Something Weird = $somethingweird" | tee -a $ARCHWORK/$LogfileName

   set tapesituation = `mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName`
   echo "$tapesituation"
   @ numlen = `expr length $tapesituation[6]` - 8
   @ BlockNumLen = `expr length $tapesituation[8]` - 8
   
   set filenum = `expr substr $tapesituation[6] 8 $numlen`
   set BlockLoc = `expr substr $tapesituation[8] 8 $BlockNumLen`

   if ($BlockLoc != 0) then
      echo "ERROR: Tape didn't start on the correct block!  Exiting program now." | tee -a $ARCHWORK/$LogfileName
      @ ListingStatus = 2
      goto EndOfProgram
   endif

   if ($filenum < 0) then                               ## I have encountered a situation where a negative number
                                                        ## is reported for the file number.  It was
                                                        ## determined that the drive was an older drive and
                                                        ## couldn't read the particular type of
                                                        ## tape that I was currently archiving.  Listing the tape
                                                        ## using a newer tape drive solves the problem.
                                                    ## Sometimes this can also be caused by things like the
                                                        ## tape wasn't loaded correctly or the drivers may
                                                        ## have acted up.  Sometimes rebooting the system solves some problems.
      echo "ERROR: Negative Filenumber. Maybe tape isn't compatible with this drive (or driver failure?)-----" | tee -a $ARCHWORK/$LogfileName
      mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName
      @ ListingStatus = 4
      goto EndOfProgram
   endif


   @ RetryCount = 0
   while ($filenum != $ExpectFileNum)
      @ RetryCount = $RetryCount + 1
      if ($RetryCount == 1) then
         @ FileSkip = $FileSkip + 1
         set SkipAt = "$SkipAt $ExpectFileNum"

         ## New Item added to tapelisting format on April 6, 2006
         ## Do not expect this to exist in listings made prior to this date
	 ## This is being added to the format to help flag potentially
	 ## incomplete listings.
	 ## Many different things can cause a file skip, with slightly different
	 ## results.  I am defining that the FileGroup number that is reported
	 ## by this item is the FileGroup number that was EXPECTED.  By definition,
	 ## that expected FileGroup number should be 1 more than what the last
	 ## FileGroup number that was listed.
	 ## In the case of dealing with the Ultrium tapes on the server ls2,
	 ## the most common reason for a file skip is that there was a hangup,
	 ## of sorts, on the SCSI bus, and the tape loses its sync with what
	 ## the mt program reports as the tape status.  So it seems that, for
	 ## this kind of situation, the listing of the FileGroup BEFORE the 
	 ## Expected FileGroup number is the one that is possibly incomplete.
         echo "---^^^---FileSkipAt #:  $ExpectFileNum" >> $ARCHWORK/$ListFile
	 echo " " >> $ARCHWORK/$ListFile

         echo "There was a file skip here!" | tee -a $ARCHWORK/$LogfileName
      else
            if ($RetryCount <= $MaxRetries) then
               echo "Lets retry this handling of the file skip!" | tee -a $ARCHWORK/$LogfileName
            else
               echo "ERROR: Too many retries to get to this spot: $ExpectFileNum  Exiting program now." | tee -a $ARCHWORK/$LogfileName
	       @ ListingStatus = 3
               goto EndOfProgram
            endif
      endif

## Since all kinds of things can cause a file skip, I want to go back and correct this mistake.
## So, I will rewind the tape (In case the tape already rewound, but the filecounter didn't)
## and move to the expected spot myself.
      mt -f $DRIVE rewind

      ## OK, I know this will seem REALLY STUPID, but trust me here...
      ## With all of these SCSI hangups, I just want to make sure that
      ## the tape is totally rewound.  And no, checking the status does
      ## not guarantee you the correct position when a SCSI hangup occurs.
      mt -f $DRIVE rewind

      ## THERE! I said it!  I repeated the rewind on purpose.  And you
      ## probably thought I wouldn't do that, now did you?!?

      mt -f $DRIVE fsf $ExpectFileNum

      set tapesituation = `mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName`
      @ numlen = `expr length $tapesituation[6]` - 8
      set filenum = `expr substr $tapesituation[6] 8 $numlen`
   end   ## end of the while loop for retries

   if ($GotFirstFileNum == 0) then
      @ FirstNumOfRun = $filenum
      @ GotFirstFileNum = 1
   endif

   echo "Doing dd"
   dd if=$DRIVE of=$JUNKFOLDER/header$$.tmp bs=10240 count=1 	## doing a 'dd' read of the beginning of the current file on 
							## tape and outputing it to a file bs=10240 allocates buffer 
							## for the dd read. Most data files have block sizes larger than this.
   							## Tape_Header_Files and Volume_Header_Files have this block size.
   @ ddStatus = $status
   echo "Done dd"

   if ($ddStatus != 0 && $ddStatus != 2) then
        echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
        echo "**************** We found a different value for the dd result ********************" | tee -a $ARCHWORK/$LogfileName
        echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
        echo "$ddStatus" | tee -a $ARCHWORK/$LogfileName
	@ somethingweird = 1
   endif

   set ddHeadstr = `ls -s $JUNKFOLDER/header$$.tmp`
   @ BogusFile = 0		## assume not bogus until proven otherwise.
   
   ## ******************** Start of the dd read file output analysis *******************************
   set CheckStatus = `mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName`
   @ Checknumlen = `expr length $CheckStatus[6]` - 8
   @ Blocknumlen = `expr length $CheckStatus[8]` - 8

   set CheckNum = `expr substr $CheckStatus[6] 8 $Checknumlen`
   set BlockNum = `expr substr $CheckStatus[8] 8 $Blocknumlen`

   if ($CheckNum > $filenum) then        ## just a tape mark in a bad place.  Essentially, the "file" was so small,
                                         ## that the 'dd' command to read the begining of the file resulted in it
                                         ## reading the entire file and ending up in the beginning of the next file.
                                         ## These "zero-byte" files may or may not be the result of the 'dd' command being
                                         ## unable to move the tape, but I still consider them as "Bogus Files"
                                         ## and I treat them as such.
      echo "---------CheckNum didn't equal filenum, must be a Bogus File-------------" | tee -a $ARCHWORK/$LogfileName
      if ($filenum == 0) then
         mt -f $DRIVE rewind
      else
         @ TapeMove = $CheckNum - $filenum + 1
         mt -f $DRIVE bsfm $TapeMove
      endif
      @ BogusFile = 1
   else if ($CheckNum < $filenum) then
      echo "---------CheckNum didn't equal filenum, must be a Bogus File-------------" | tee -a $ARCHWORK/$LogfileName
      @ TapeMove = $filenum - $CheckNum
      mt -f $DRIVE fsf $TapeMove
      @ BogusFile = 1
   else
      if ($filenum == 0) then
         mt -f $DRIVE rewind
      else
         mt -f $DRIVE bsfm 1
      endif
   endif

   ## at this point, the tape should be at file number $filenum

   if ($ddHeadstr[1] == 0) then			##zero byte file can mean a bunch of different things.  This structure figures it out
      echo "------------- I have a zero-byte file ------------------" | tee -a $ARCHWORK/$LogfileName
      ## the dd didn't cross files...
      ## now see if there were any problems with the dd.
      if ($BlockNum <= 0) then				##tape couldn't move forward, could mean the end of the tape
         echo "---------Tape couldn't move forward-----------------" | tee -a $ARCHWORK/$LogfileName
         echo "---------Moving Tape to the end for a check---------" | tee -a $ARCHWORK/$LogfileName

         mt -f $DRIVE eod
         set EndStatus = `mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName`
         @ Endnumlen = `expr length $EndStatus[6]` - 8
         set EndNum = `expr substr $EndStatus[6] 8 $Endnumlen`
                                                                                                 
         ## **************  Start of the 'End-Of-Device (Tape) Validation' if structure *****************
         if ($EndNum == $CheckNum) then			## Since the filenumber for the end of the tape 
                                                        ## and the file number of the current loop are the same
						        ## then it must be the final tapemark of the tape.
            echo "We have reached the end with no problems.  Exiting now." | tee -a $ARCHWORK/$LogfileName
            @ ListingStatus = 0
            goto EndOfProgram
         else			## since they didn't match, this is a situation where the drive landed on a "file" that has no
	       			## data within it, but it still takes up some physical space while still being considered
	        		## a "zero-byte" file.  I call these files "Bogus Files" and I still represent them in the
		        	## listing, since they have a file number, but there is no filename given and it is ignored
			        ## when creating the catalogue.
            echo "----------The check failed, must be a bogus file---------------------------" | tee -a $ARCHWORK/$LogfileName

            if ($filenum == 0) then
               mt -f $DRIVE rewind
            else
               @ TapeMove = $EndNum - $filenum + 1
               mt -f $DRIVE bsfm $TapeMove			## returns the tape back to the beginning of the current file being processed
            endif

            @ BogusFile = 1
         endif
         ## -------------  End of the 'End-Of-Device (Tape) Determination' if structure ----------------
      endif ## end of check for unusual block numbers
   else
      ## The 'dd' read was a positive test for useful data
      echo "---------- The file was not a zero-byte file, might have some useful info in it --------------" | tee -a $ARCHWORK/$LogfileName
   endif ## end of check for unusual file numbers
   ## ---------------- End of the dd read file output analysis -------------------------------

   ## note, at this point, I am at the beginning of the file on the tape.

   echo "------- Taring header file in the junk folder -------"
   tar -b 20 -xvf $JUNKFOLDER/header$$.tmp -C $JUNKFOLDER
   set aftertar = $status

   echo "-------- Done taring header file ------------"
   if ($aftertar != 0 && $aftertar != 2) then
	echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
	echo "**************** We found a different value for the tar result *******************" | tee -a $ARCHWORK/$LogfileName
	echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
	echo "$aftertar" | tee -a $ARCHWORK/$LogfileName
	@ somethingweird = 1
   endif

   rm $JUNKFOLDER/header$$.tmp


   echo "--------------- Now we are determining what kind of file it is ----------------" | tee -a $ARCHWORK/$LogfileName

   if ($BogusFile == 0) then				## Since the current file is not a Bogus File, it 
							## should contain useful data

      if ( $ddStatus == 0 ) then  			## either volume or tape header or "small" file

     	 ## This checks for existance of certain kinds of files that could
         ## have been extracted from the header file that we just deleted.

         if ( -e $JUNKFOLDER/\$TAPE_VOLUME_HEADER ) then  		## check for Volume Header file
	    echo "Going into PrintVHF" | tee -a $ARCHWORK/$LogfileName
	    goto PrintVHF
         else if ( -e $JUNKFOLDER/\$TAPE_DIRECTORY_FILE ) then  	## check for Tape Header file
	    echo "Going into PrintTHF" | tee -a $ARCHWORK/$LogfileName
	    goto PrintTHF
         else						
	    ## Since neither of the other filenames are found 
	    ## and it isn't a Bogus File, assume it is data
	    echo "Going into PrintDF from a successful dd.  Blockdd = $Blockdd" | tee -a $ARCHWORK/$LogfileName
	    goto PrintDF
         endif

      else
	 ## Assume it is a data file, since no useful info 
	 ## could be obtained from the 'dd' read
         echo "Going into PrintDF from a failed dd.  Blockdd = $Blockdd" | tee -a $ARCHWORK/$LogfileName
	 goto PrintDF
      endif
   else							## Previous checks indicated that the file at filenum is a Bogus File.
      echo "---We have ran into bogus file. Just printing an empty listing---" | tee -a $ARCHWORK/$LogfileName
      echo "---^^^---File #: $filenum" >> $ARCHWORK/$ListFile
      echo "---^^^---Listing:" >> $ARCHWORK/$ListFile
      echo " " >> $ARCHWORK/$ListFile

      goto EndOfPrint		## goes to the end of the while loop
   endif

#-----------------------------------------------------------------------------------------------
#----------------------------------------------Subroutines--------------------------------------
#-----------------------------------------------------------------------------------------------

#           -------------------PrintVHF--------------------
PrintVHF:
   mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName
   echo "---^^^---File #: $filenum" >> $ARCHWORK/$ListFile
   echo "---^^^---Volume_Header:" >> $ARCHWORK/$ListFile
   echo "---^^^---Listing:" >> $ARCHWORK/$ListFile

   echo "---- taring in PrintVHF -------"
   tar -b 126 -tvf $DRIVE >> $ARCHWORK/$ListFile
   @ tarStatus = $status

   echo "---- done taring ----------"
   if ($tarStatus != 0 && $tarStatus != 2) then
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo "**************** We found a different value for the tar result *******************" | tee -a $ARCHWORK/$LogfileName
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo "$tarStatus" | tee -a $ARCHWORK/$LogfileName
      @ somethingweird = 1
   endif

   echo "---^^^---Volume_Header_Contents:" >> $ARCHWORK/$ListFile
   cat $JUNKFOLDER/\$TAPE_VOLUME_HEADER >> $ARCHWORK/$ListFile
   rm $JUNKFOLDER/\$TAPE_VOLUME_HEADER

   echo " " >> $ARCHWORK/$ListFile
   mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName

   echo "Leaving PrintVHF" | tee -a $ARCHWORK/$LogfileName
   goto EndOfPrint			## goes to the end of the main while loop
#      ------------------------------------------------------


#           ----------------PrintTHF---------------------
## This subroutine assumes that the current working directory
## is the same directory that the listing file is in.

PrintTHF:
   echo "---^^^---File #: $filenum" >> $ARCHWORK/$ListFile
   echo "---^^^---Data_Header:" >> $ARCHWORK/$ListFile
   echo "---^^^---Listing:" >> $ARCHWORK/$ListFile

   echo "----- taring in PrintTHF ------"
   tar -b 126 -tvf $DRIVE >> $ARCHWORK/$ListFile

   @ tarStatus = $status
   echo "---- done tarring ------"

   if ($tarStatus != 0 && $tarStatus != 2) then
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo "**************** We found a different value for the tar result *******************" | tee -a $ARCHWORK/$LogfileName
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo $tarStatus | tee -a $ARCHWORK/$LogfileName
      @ somethingweird = 1
   endif


   echo "---^^^---Data_Header_Contents:" >> $ARCHWORK/$ListFile
   cat $JUNKFOLDER/\$TAPE_DIRECTORY_FILE >> $ARCHWORK/$ListFile
   rm $JUNKFOLDER/\$TAPE_DIRECTORY_FILE

   echo " " >> $ARCHWORK/$ListFile
   mt -f $DRIVE status >> $ARCHWORK/$LogfileName

   echo "Leaving PrintTHF" | tee -a $ARCHWORK/$LogfileName
   goto EndOfPrint			## goes to the end of the main while loop
#      -------------------------------------------------------


#           ----------------PrintDF----------------------
## This subroutine assumes that the current working directory is the JUNKFOLDER directory
## It also assumes that the current file on the tape is not a Bogus File

PrintDF:
   dd if=$DRIVE of=$JUNKFOLDER/header$$.tmp bs=$Blockdd count=1	
						## does another 'dd' read of the current file on tape, reads only 1 block

   set Headerstr = `ls -g $JUNKFOLDER/header$$.tmp`
   echo "This is the header$$.tmp listing:" >> $ARCHWORK/$LogfileName
   echo $Headerstr >> $ARCHWORK/$LogfileName

   rm $JUNKFOLDER/header$$.tmp

   @ BlkSz = $Headerstr[4]			## gets the reported file size of the file outputed by the 'dd' 
					## read, which is representative of a single block of the 
					## current file on tape.

   if ($BlkSz == 0) then	
				## Currently, it seems that zero blocksize files on tapes indicates a serious readability problem
					## and it must be addressed.  Right now, we will just stop the listing and put the tape
					## elsewhere for further work.
				## I should note that occasionally, I would get a zero blocksize file error, which would stop
					## the tape.  Then, I would try running the program again, and sometimes the error doesn't
					## happen again.  This may be something that needs more research, but for now, this error
					## is hard to reproduce.
				## 07/28/2004 - I am starting to be able to track this error.  It seems to sometimes occur when two
					## tape drives on the same system are both doing tars,  Although, this doesn't seem consistant.
					## more research is necessary, but this bug could possibly result in incomplete listing 
					## of a tar file if the listing is forced to cancel early.
				## More Note:  It appears the problem might have been that when one tape starts to do a dd 
					## for DF while the other tape drive was already doing a tar for a VHF or THF, 
					## problems would occur with memory allocation.  I think I solved the problem by 
					## reducing the buffer size allocation for those tars of THF and VHF.
	
      echo "ERROR: We have a zero blocksize.  Exiting Program now." | tee -a $ARCHWORK/$LogfileName
      echo "Status of the tape at bad exit:" | tee -a $ARCHWORK/$LogfileName
      mt -f $DRIVE status | tee -a $ARCHWORK/$LogfileName
      echo "Please see $ARCHWORK/$LogfileName for more info."
      @ ListingStatus = 5
      goto EndOfProgram
   endif

   @ BlkTar = $BlkSz / 512
   @ Remain = $BlkSz % 512

   if ($Remain != 0) then					
					## this was put here just for a debugging situation like misreading 
					## the blocksize into $BlkSz or something of that nature.
      echo "***^^^***There is something wrong with the blocksize  " $BlkSz | tee -a $ARCHWORK/$ListFile
      echo "***^^^***Can not go further.  Exiting the Program at file #" $filenum | tee -a $ARCHWORK/$ListFile
      echo "BlkSz: $BlkSz  BlkTar: $BlkTar  Remain: $Remain" | tee -a $ARCHWORK/$LogfileName
      echo "!!!!!!--------There is an error with the blocksize, exiting program now.-------!!!!!!!!!!!!!" | tee -a $ARCHWORK/$LogfileName
      @ListingStatus = 6
      goto EndOfProgram
   endif

   if ($filenum > 0) then
      mt -f $DRIVE bsfm 1
   else
      mt -f $DRIVE rewind
   endif

   echo "---^^^---File #: " $filenum >> $ARCHWORK/$ListFile
   echo "---^^^---Data:" >> $ARCHWORK/$ListFile
   echo "---^^^---Listing:" >> $ARCHWORK/$ListFile
   echo "----- Starting taring in PrintDF BlkTar = $BlkTar ---"
   tar -b $BlkTar -tvf $DRIVE >> $ARCHWORK/$ListFile			## sends the listing of the contents of the TAR to the listing file
   @ tarStatus = $status

   echo "----- Done taring in PrintDF --------"			## should end off at the end of the current file
   echo " " >> $ARCHWORK/$ListFile

   if ($tarStatus != 0 && $tarStatus != 2) then
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo "**************** We found a different value for the tar status *******************" | tee -a $ARCHWORK/$LogfileName
      echo "**********************************************************************************" | tee -a $ARCHWORK/$LogfileName
      echo "$tarStatus" | tee -a $ARCHWORK/$LogfileName
      @ somethingweird = 1
   endif

   mt -f $DRIVE status >> $ARCHWORK/$LogfileName
   echo "Leaving PrintDF" | tee -a $ARCHWORK/$LogfileName
   goto EndOfPrint			## goes to the end of the main while loop
#      ------------------------------------------------------


#           ----------------EndOfPrint----------------
EndOfPrint:
   @ LastTime = $filenum
   echo "LastTime = $LastTime" >> $ARCHWORK/$LogfileName

   mt -f $DRIVE fsf 1					## this line can be a source of problems.  
							## It assumes that the tar command a few lines ago 
							## left the tape at the end of the file.  Sometimes 
							## errors in the tape causes the tar command to 
							## leave off at the beginning of the next file, 
							## essentially causing the lister to skip a file.

end			## This end command is for the end of the while loop in the main program.  
			## It sends the program back up to the beginning of that loop
#      ------------------------------------------------------


#           -------------EndOfProgram-----------------
## This subroutine assumes that the current working directory
## is the same directory where the listing file is.

EndOfProgram:

rm $JUNKFOLDER/*			## cleaning up the work area.
rmdir $JUNKFOLDER

echo " " >> $ARCHWORK/$ListFile			## putting the final touches on the listing, whether it 
					## was completed succesfully or suffered from
					## a failure of some sort.
echo "---^^^---End Of Tape" >> $ARCHWORK/$ListFile
echo "Listing ended at:  " `date` >> $ARCHWORK/$ListFile
echo "The tarstatus at end = $tarStatus" >> $ARCHWORK/$ListFile
echo " "

if ($FileSkip >= 1) then
   ## Adding this to the tapelisting file just so there is a quick reference at the end
   ## of a listing.  This is not intended to be in any official format, so don't
   ## assume there were no skips if this doesn't show up.  Rely on the FileSkipAt items that appear in the listing.
   echo "Possible tape issues at around filegroup number(s) $SkipAt" >> $ARCHWORK/$ListFile
   echo "Tape skips appear to have occured at file number(s) $SkipAt" | tee -a $ARCHWORK/$LogfileName
endif

echo "By the way, the file was called $ListFile" | tee -a $ARCHWORK/$LogfileName
echo "The first filegroup number for this run was $FirstNumOfRun" | tee -a $ARCHWORK/$LogfileName
set TempStatus = `mt -f $DRIVE status`
echo "This is the tape's info here: $TempStatus[30]"
echo " "

if ($somethingweird == 1) then
   echo "^&^&^&^&^--Please check out the different values of the tar status we obtained--^&^&^&^&^" | tee -a $ARCHWORK/$LogfileName
endif

exit $ListingStatus
#      ------------------------------------------------------
#################################################################
