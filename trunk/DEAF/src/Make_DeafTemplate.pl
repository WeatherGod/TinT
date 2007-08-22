#! /usr/bin/perl -w

use strict;
use Getopt::Long;
$Getopt::Long::ignorecase = 0;	# options are case sensitive

my ($PluginName, $BeanName, $BeanSourceDir, $DestDir, $ConfigFile) = (undef, undef, undef, undef, undef);
my $IsNewBean = 0;			# 0 for false, 1 for true

if (!GetOptions(
	"help|h"	=> sub { PrintHelp();
				 exit(1); },
	"syntax|x"	=> sub { PrintSyntax();
				 exit(2); },
	"plugin|p=s"	=> \$PluginName,
	"bean|b=s"	=> \$BeanName,
	"source|s=s"	=> \$BeanSourceDir,
	"dest|d=s"	=> \$DestDir,
	"config|c=s"	=> \$ConfigFile ) )
{
	PrintSyntax();
	exit(2);
}

if (!defined($PluginName) || !defined($BeanName) || !defined($DestDir))
{
	print STDERR "ERROR: PluginName, BeanName and destination directory are all required\nat the command line!\n";
	PrintSyntax();
	exit(2);
}


######## Have a verifier to see if the BeanName 
######## and the PluginName are
######## valid C++ object names.



if (defined($BeanSourceDir) && $BeanSourceDir ne $DestDir)
{
	# the user supplied a bean source directory,
	# and it isn't the same as the destination
	# dir (This isn't guarranteed, though...).
	# so this plugin is going to use
	# a pre-existing data bean.

	if (-e "$BeanSourceDir/$BeanName.h" && -e "$BeanSourceDir/$BeanName.C")
	{
		# Well, the needed files exists.  I guess it is
		# pre-existing!
		$IsNewBean = 0;
	}
	else
	{
		print STDERR "The databean $BeanName does not exist in the directory $BeanSourceDir\n";
		print STDERR "Did you mean to create a new databean?\n\n";
		PrintHelp();
		exit(3);
	}
}
else
{
	# assume that the plugin will use a new bean.
	$IsNewBean = 1;
	$BeanSourceDir = "$DestDir/$PluginName";
}


if (-d "$DestDir/$PluginName")
{
	# Check to see if any source code already exists in this directory.
	# I don't care about the registration file or the Makefile,
	# or should I...?
	if (-e "$DestDir/$PluginName/$PluginName.h" 
	    || -e "$DestDir/$PluginName/$PluginName.C"
	    || ($IsNewBean && -e "$DestDir/$PluginName/$BeanName.h")
	    || ($IsNewBean && -e "$DestDir/$PluginName/$BeanName.C"))
	{
		print "The $PluginName plugin in that directory will be destroyed, if it exists.\n";
		if ($IsNewBean)
		{
			print "The $BeanName plugin in that directory will be destroyed, if it exists.\n";
		}

		print "Is this OK? (y/N): ";
		$| = 1;					# flushes the print statement.
		my $Response = uc(<STDIN>);

		if (index($Response, 'Y') != 0)		# Y was not found as the first letter,
							# assume that the user said no!
		{
			print "\nExiting program... No changes were made.\n";
			exit(4);
		}

		print "\n";
	}

	# If the directory exists, but no code is in it that will be
	# over-written, then just go ahead with it...
}
else
{
	mkdir "$DestDir/$PluginName" or die("Could not create directory: $DestDir/$PluginName\n");
}


my $NewEntry = "\t<Plugin Name='$PluginName' Registration='$DestDir/$PluginName/$PluginName.reg'/>\n";

if (defined($ConfigFile))
{
	if (!WriteConfigFile($ConfigFile, $PluginName, $DestDir, $NewEntry))
	{
		print STDERR "ERROR: Problem with the configuration file $ConfigFile\n";
		exit(3);
	}
}
else
{
        print "Insert this line into your configuration file...\n";
        print "\n$NewEntry\n";
}


if (!MakePluginTemplate($DestDir, $PluginName, $BeanName, $IsNewBean))
{
	print STDERR "ERROR: Problem with creating the $PluginName source codes.\n";
	exit(3);
}

if ($IsNewBean)
{
	if (!MakeBeanTemplate($DestDir, $PluginName, $BeanName))
	{
		print STDERR "ERROR: Problem with creating the $BeanName source codes.\n";
		exit(3);
	}
}

if (!MakeRegistration($DestDir, $PluginName, $BeanName))
{
	print STDERR "ERROR: Problem with creating the registration file.\n";
	exit(3);
}

if (!CreateMakefile($IsNewBean, $DestDir, $BeanSourceDir, $PluginName, $BeanName))
{
	print STDERR "ERROR: Problem with creating the Makefile.\n";
	exit(3);
}

print "\nSuccess!\nThe $PluginName plug-in source codes can be found in the directory:\n$DestDir/$PluginName\n";
print "\nIMPORTANT REMINDER!!\n";
print "You may still need to modify the configuration and registration entries to\n";
print "help the deaf program find the plugin correctly.\n";
print "Also, some of the directories in the Makefile may need to be corrected as well.\n\n";

exit(0);

#################################################################################################

sub PrintHelp
{
	PrintSyntax();

	print STDERR "HELP!\n";
}

sub PrintSyntax
{
	print STDERR "\nMake_DeafTemplate.pl --plugin | -p _PLUGINNAME_ --bean | -b _BEANNAME_\n";
	print STDERR "                       --dest | -d _DESTDIR\n";
	print STDERR "                       [--source | -s _BEANSOURCEDIR_]\n";
	print STDERR "                       [--config | -c _CONFIGFILE_]\n";
	print STDERR "                       [--syntax | -x] [--help | -h]\n\n";
}



sub WriteConfigFile
# if the config file already exists, read its contents until the </DeafConf>
# tag is found, and replaces the tag with $NewEntry and adds the
# </DeafConf> tag to the end.
# if the config file doesn't exist, then create a new one with an opening
# and closing <DeacConf> tag with $NewEntry as the only entry.
{
	my ($ConfigFile, $PluginName, $DestDir, $NewEntry) = @_;

	my $ConfigContents = '';

	if (-e $ConfigFile)
	{
		if (!open(CONFSTREAM, "<$ConfigFile"))
		{
			print STDERR "ERROR: Could not read file $ConfigFile\n";
			return(0);
		}

		my $LineRead = '';
		my $KeepRead = 1;

		while (defined($LineRead = <CONFSTREAM>) && $KeepRead)
		{
			if ($LineRead =~ m#</DeafConf>#)
			{
				$LineRead =~ s#</DeafConf>#$NewEntry#;
				$KeepRead = 0;
			}

			$ConfigContents .= $LineRead;
		}

		close CONFSTREAM;

		if ($KeepRead)
		{
			print STDERR "ERROR: Could not find the </DeafConf> tag\n";
			print STDERR "in the configuration file $ConfigFile\n";
			return(0);
		}
	}
	else
	{
		$ConfigContents = "<DeafConf>\n$NewEntry";
	}

	if (!open(NEWCONF, ">$ConfigFile"))
	{
		print STDERR "ERROR: Could not write to file $ConfigFile\n";
		return(0);
	}

	print NEWCONF $ConfigContents;
	print NEWCONF '</DeafConf>';

	close NEWCONF;

	return(1);
}


sub MakePluginTemplate
# creates the .C and the .h files for the plug-in
{
	my ($DirName, $PluginName, $BeanName, $IsNewBean) = @_;

	my $Filename = "$DirName/$PluginName/$PluginName.h";

	my $BeanInclude = '';

	if ($IsNewBean)
	{
		$BeanInclude = "\"$BeanName.h\"";
	}
	else
	{
		$BeanInclude = "<$BeanName.h>";
	}

	if (!open(FILESTREAM, ">$Filename"))
	{
		print STDERR "ERROR: Could not write to plug-in header file $Filename\n";
		return(0);
	}

	print FILESTREAM "#ifndef _".uc($PluginName)."_H
#define _".uc($PluginName)."_H

#include <iostream>
#include <string>
#include <mysql++/mysql++.h>
#include <DeafPlugin.h>
#include <DeafBean.h>

#include $BeanInclude


// Add any other #include's that you need here.




class $PluginName : public DeafPlugin
//Extends the DeafPlugin base class
{
	public:
		// Default Constructor
		$PluginName();

		// Destructor
		virtual ~$PluginName();

		// Do not modify the parameter lists of these functions
		virtual bool From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer);
		virtual bool To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName);

		virtual bool From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer);
		virtual bool To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName);

		virtual bool From_Stream(istream &InStream, DeafBean &DataLayer);
		virtual bool To_Stream(const DeafBean &DataLayer, ostream &OutStream);

	private:
		// Feel free to add any private member functions and/or variables here.



	protected:
		// Do not change these functions.
		virtual $BeanName& ReCast(DeafBean& BeanRef) const;
		virtual const $BeanName& ReCast(const DeafBean& BeanRef) const;
};

#endif\n";

	close FILESTREAM;

	$Filename = "$DirName/$PluginName/$PluginName.C";

	if (!open(FILESTREAM, ">$Filename"))
	{
        	print STDERR "ERROR: Could not write to plug-in class file $Filename\n";
        	return(0);
	};

	print FILESTREAM "#ifndef _".uc($PluginName)."_C
#define _".uc($PluginName)."_C
using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <mysql++/mysql++.h>		// for the mysqlpp Query object

#include <DeafPlugin.h>
#include <DeafBean.h>

#include \"$PluginName.h\"
#include $BeanInclude


// Add any other #include's that you need here.




// Default Constructor
${PluginName}::$PluginName()
/* Initialize member variables here like so:
	:	myFooNum(0),
		myFooBool(false),
		myFooStr(\"\")
{
}
*/
{
}


// Destructor
${PluginName}::~$PluginName()
// Most likely, you don't have to modify this...
{
}\n\n";

	print FILESTREAM "
/* ----------------- Public Member functions -------------------------
   These six functions can be called by Deaf.
   Add code after the 'ReCast()' line, using the
   BeanLayer variable as the data-bean.

   Be sure to change the return value of the function
   from false before building the plug-in.

   Also, make sure that any functions you want available
   is noted in the plug-in's registration file like so:

	<Function Name=\"To_File\"/>

   to make the To_File() function available.
*/

// -------------------- File -------------------------------------
bool ${PluginName}::From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer)
{
	$BeanName& BeanLayer = ReCast(DataLayer);

	// Add Code here



        return(false);
}

bool ${PluginName}::To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName)
{
	const $BeanName& BeanLayer = ReCast(DataLayer);

	// Add Code here




	return(false);
}


// ------------------- MySQL --------------------------------------
bool ${PluginName}::From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer)
{
	$BeanName& BeanLayer = ReCast(DataLayer);

	// Add code here



	return(false);
}

bool ${PluginName}::To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName)
{
	const $BeanName& BeanLayer = ReCast(DataLayer);

	// Add Code here



	return(false);
}


// ------------------ Stream -------------------------------------
bool ${PluginName}::From_Stream(istream &InStream, DeafBean &DataLayer)
{
        $BeanName& BeanLayer = ReCast(DataLayer);

	// Add Code here



        return(false);
}

bool ${PluginName}::To_Stream(const DeafBean &DataLayer, ostream &OutStream)
{
        const $BeanName& BeanLayer = ReCast(DataLayer);

	// Add code here


        return(false);
}";

	print FILESTREAM "\n\n
// ---------------------- Private Member Functions ----------------------------------
// Add any of your private member functions here...




// ---------------------- Protected Member functions --------------------------------
// Do not modify these functions!
$BeanName& ${PluginName}::ReCast(DeafBean& BeanRef) const
{
	return(static_cast<$BeanName&>(BeanRef));
}

const $BeanName& ${PluginName}::ReCast(const DeafBean& BeanRef) const
{
	return(static_cast<const $BeanName&>(BeanRef));
}



// DON'T TOUCH!
extern \"C\" ${PluginName}* ${PluginName}Factory()
{
	return(new $PluginName);
}

// DON'T TOUCH!
extern \"C\" void ${PluginName}Destructor( ${PluginName}* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}

#endif\n";

	close FILESTREAM;

	return(1);
}

sub MakeBeanTemplate
{
	my ($DirName, $PluginName, $BeanName) = @_;

        my $Filename = "$DirName/$PluginName/$BeanName.h";

        if (!open(FILESTREAM, ">$Filename"))
	{
        	print STDERR "ERROR: Could not write the data-bean header file $Filename\n";
		return(0);
	}

	print FILESTREAM "#ifndef _".uc($BeanName)."_H
#define _".uc($BeanName)."_H


#include <DeafBean.h>

// Add any other #include's here for your bean.


class $BeanName : public DeafBean
//Extends the DeafBean base class
{
	public:
		// Default Constructor
		$BeanName();

		// Destructor
		virtual ~$BeanName();


		// Feel free to add any number of member functions here.


	private:
		// Feel free to add any number of member variables or
		// functions here.


};

#endif\n";

	close FILESTREAM;

	$Filename = "$DirName/$PluginName/$BeanName.C";

        if (!open(FILESTREAM, ">$Filename"))
	{
		print STDERR "ERROR: Could not write the data-bean class file $Filename\n";
		return(0);
	}


	print FILESTREAM "#ifndef _".uc($BeanName)."_C
#define _".uc($BeanName)."_C
using namespace std;


#include <DeafBean.h>


#include \"$BeanName.h\"

// Add any other #include's for your bean here.




// Default Constructor
${BeanName}::$BeanName()
/* Initialize member variables here like so:
	:	myFooNum(0),
		myFooBool(false),
		myFooStr(\"\")
{
}
*/
{
}



// Destructor
${BeanName}::~$BeanName()
{
	// Chances are, you don't have to do anything here.
}


// Add your member functions here.





// DON'T TOUCH!!!
extern \"C\" ${BeanName}* ${BeanName}Factory()
{
	return(new $BeanName);
}

// DON'T TOUCH!!!
extern \"C\" void ${BeanName}Destructor( ${BeanName}* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}


#endif\n";

	close FILESTREAM;

	return(1);
}

sub MakeRegistration
{
	my ($DirName, $PluginName, $BeanName) = @_;

	my $Filename = "$DirName/$PluginName/$PluginName.reg";

        if (!open(FILESTREAM, ">$Filename"))
	{
        	print STDERR "ERROR: Could not write the plug-in registration file $Filename\n";
                return(0);
	}

	print FILESTREAM "<Plugin Name='$PluginName' Bean='$BeanName' File='$DirName/$PluginName/lib${PluginName}.so'>
\t<File Input='' Output='' Filename=''/>
\t<MySQL InputHost='' InputUser='' InputHostPort=''/>
\t<MySQL OutputHost='' OutputUser='' OutputHostPort=''/>
\t<Function Name=''/>
\t<Function Name=''/>

\t<Author>Your Name Here</Author>
\t<Contact>Contact Info Here</Contact>
\t<Notes>Extra notes here</Notes>
\t<Description>Your Description Here</Description>
</Plugin>\n";

	close FILESTREAM;

	return(1);
}


sub CreateMakefile
{
	my ($IsNewBean, $DirName, $BeanSourceDir, $PluginName, $BeanName) = @_;

	my $Filename = "$DirName/$PluginName/Makefile";

	my @DeafDirs = GetDeafDirs();

	my $DeafDir = '.';

	foreach (@DeafDirs)
	{
		if (-d "$_/include")
		{
			$DeafDir = $_;
			last;
		}
	}

	## need to escape shell characters.
	($DeafDir, $BeanSourceDir) = Esc($DeafDir, $BeanSourceDir);


        if (!open(FILESTREAM, ">$Filename"))
	{
        	print STDERR "ERROR: Could not create the Makefile $Filename\n";
                return(0);
	}

	print FILESTREAM "CC = gcc
CFLAGS = -O2
DEAF_INC_PATH = ${DeafDir}/include

# for example: -I /home/Zaphod/include
MYINCS =

# for example: -L /home/Zaphod/lib
MYLIBS =


MYSQL_INC = -I /usr/include/mysql
LIB_LINKS = -l mysqlpp -l stdc++


MYBEAN_DIR = $BeanSourceDir
DEAFBUILD_DIR = \$(PWD)

#-----------------------------------------------------------------------

plugin : lib${PluginName}.so


lib${PluginName}.so : ${BeanName}.o ${PluginName}.o
\t\$(CC) \$(LDFLAGS) -shared ${PluginName}.o ${BeanName}.o  -o lib${PluginName}.so \$(MYLIBS) \$(LIB_LINKS)
";

	if ($IsNewBean)
	{
		# Have the Makefile build the bean directly to the location
		print FILESTREAM "
${BeanName}.o : ${BeanName}.C
\t\$(CC) \$(CFLAGS) -fPIC -c ${BeanName}.C -o \$(DEAFBUILD_DIR)/${BeanName}.o -I \$(DEAF_INC_PATH) \$(MYINCS)";
	}
	else
	{
		# Have the Makefile copy the object file to the location.
		# Have the Makefile go to the Bean's directory, and call its make command
		# (which would be the one we printed in the above block).
		print FILESTREAM "
${BeanName}.o : \$(MYBEAN_DIR)/${BeanName}.o
\tcp -u \$(MYBEAN_DIR)/${BeanName}.o ${BeanName}.o

\$(MYBEAN_DIR)/${BeanName}.o : \$(MYBEAN_DIR)/${BeanName}.C
\tcd \$(MYBEAN_DIR); \\
\tmake \"DEAFBUILD_DIR=\$(DEAFBUILD_DIR)\" ${BeanName}.o";
	}

	print FILESTREAM "\n
${PluginName}.o : ${PluginName}.C
\t\$(CC) \$(CFLAGS) -fPIC -c ${PluginName}.C -o ${PluginName}.o -I \$(DEAF_INC_PATH) \$(MYINCS) \$(MYSQL_INC)

clean :
\t- rm -f lib${PluginName}.so
\t- rm -f ${BeanName}.o
\t- rm -f ${PluginName}.o

remove :
\t- rm -f \$(DEAF_INC_PATH)/${PluginName}.h";

	if ($IsNewBean)
	{
		print FILESTREAM "\n\t- rm -f \$(DEAF_INC_PATH)/${BeanName}.h";
	}

	print FILESTREAM "\n\ninstall : lib${PluginName}.so ${PluginName}.h ";

	if ($IsNewBean)
	{
		print FILESTREAM "${BeanName}.h\n\tcp -f ${BeanName}.h \$(DEAF_INC_PATH)/${BeanName}.h";
	}

	print FILESTREAM "\n\tcp -f ${PluginName}.h \$(DEAF_INC_PATH)/${PluginName}.h\n";

	close FILESTREAM;

	return(1);
}

sub Esc 
{
        foreach my $Word (@_)
        {
                $Word =~ s/([;<>\*\|`&\$!#\(\)\[\]\{\}\\:'" ])/\\$1/g;
        }

        return(@_);
}

sub GetDeafDirs
{
	my $DeafHomeVal = $ENV{DEAF_HOME};

	if ($DeafHomeVal eq '')
	{
		print STDERR "ERROR: The DEAF_HOME environment variable is missing or empty!\n";
		print STDERR "     : Using '.' instead...\n";
		return(".");
	}

	my @DeafDirs = ();

	my $IsEscaped = 0;
	my $StrLength = length($DeafHomeVal);

	my $StartPos = 0;
	my $EndPos = 0;

	for ( ; $EndPos < $StrLength; $EndPos++)
	{
		if (substr($DeafHomeVal, $EndPos, 1) eq ':')
		{
			if ($IsEscaped)
			{
				$IsEscaped = 0;
			}
			else
			{
				push(@DeafDirs, substr($DeafHomeVal, $StartPos, $EndPos - $StartPos));
				$StartPos = $EndPos + 1;
			}
		}
		elsif (substr($DeafHomeVal, $EndPos, 1) eq '\\')
		{
			$IsEscaped = !$IsEscaped;
		}
		elsif ($IsEscaped)
		{
			print STDERR "ERROR: Bad value for DEAF_HOME environment variable: $DeafHomeVal\n";
			last;
		}
	}

	if ($EndPos != $StartPos)
	{
		push(@DeafDirs, substr($DeafHomeVal, $StartPos, $EndPos - $StartPos));
	}

	foreach (@DeafDirs)
	{
		$_ =~ s/\\\\/\\/g;
		$_ =~ s/\\:/\\/g;
	}

	return(@DeafDirs);
}
