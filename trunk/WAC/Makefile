CC = gcc

WAC_DATABASE_NAME = WX_ARCHIVE_CATALOG
WAC_ADMIN_SYS = ls2.meteo.psu.edu
WAC_ADMIN_NAME = wxarchadmin
WAC_USER_NAME = wxarchuser


SPECIALDEFS = -D '_WAC_DATABASE_NAME_="$(WAC_DATABASE_NAME)"' -D '_WAC_ADMIN_SYS_="$(WAC_ADMIN_SYS)"' \
-D '_WAC_ADMIN_NAME_="$(WAC_ADMIN_NAME)"' -D '_WAC_USER_NAME_="$(WAC_USER_NAME)"'

################################################################################
MYLIB_PATH = /home/meteo/bvroot/usr/lib
MYINC_PATH = /home/meteo/bvroot/usr/include

# Make sure that the WAC_HOME environment variable is set!
INSTALLPATH = $(WAC_HOME)


BUTILS_INC_PATH = /home/meteo/bvroot/usr/include
BUTILS_LIB_PATH = /home/meteo/bvroot/usr/lib

MYSQLPP_INC_PATH = /home/meteo/bvroot/usr/include
MYSQLPP_LIB_PATH = /home/meteo/bvroot/usr/lib

REGEX_INC_PATH = /home/meteo/bvroot/usr/include
REGEX_LIB_PATH = /home/meteo/bvroot/usr/lib



MYSQL_INC = -I /usr/include/mysql
################################################################################


LIBS = -l BUtils -l stdc++
INCFLAGS = -I $(BUTILS_INC_PATH) -I include/
LIBFLAGS = -L $(BUTILS_LIB_PATH) -L lib/



OBJECTS = Reader.o DataFile.o WACUtil.o

## All the things that needs to be built or created without installing
all : lib/libWAC.so Sorter TheAdmins WACmysql.cnf

# may still need to add some more to this, like a database maker, and some config makings?
install : TheDirs TheListers $(INSTALLPATH)/Commands/Sorter $(INSTALLPATH)/Commands/Load_Sorted_Data \
          $(INSTALLPATH)/Commands/Load_Volume_Data $(INSTALLPATH)/Commands/Make_Archive_Tables \
          $(INSTALLPATH)/Commands/ListVolume.pl $(INSTALLPATH)/Commands/BuildCatalog.pl \
          $(MYINC_PATH)/Reader.h $(MYINC_PATH)/DataFile.h $(MYINC_PATH)/WACUtil.h \
          $(INSTALLPATH)/ConfigFiles/.WACmysql.cnf

########## Available listers #################################
LISTERS = $(INSTALLPATH)/Commands/TapeLister.sh $(INSTALLPATH)/Commands/DiskLister.pl
TheListers : $(LISTERS)

$(INSTALLPATH)/Commands/TapeLister.sh : src/TapeLister.sh
	cp -f src/TapeLister.sh $(INSTALLPATH)/Commands/TapeLister.sh

$(INSTALLPATH)/Commands/DiskLister.pl : src/DiskLister.pl
	cp -f src/DiskLister.pl $(INSTALLPATH)/Commands/DiskLister.pl

##############################################################


TheAdmins : Load_Sorted_Data Load_Volume_Data Make_Archive_Tables

TheDirs : $(INSTALLPATH)/ToBeCatalogued $(INSTALLPATH)/ToBeSorted $(INSTALLPATH)/ToBeLoaded \
          $(INSTALLPATH)/Catalogued $(INSTALLPATH)/Sorted $(INSTALLPATH)/Loaded \
          $(INSTALLPATH)/Commands $(INSTALLPATH)/ConfigFiles

libWAC.so : lib/libWAC.so

Sorter : bin/Sorter

Load_Sorted_Data : bin/Load_Sorted_Data
Load_Volume_Data : bin/Load_Volume_Data
Make_Archive_Tables : bin/Make_Archive_Tables

WACmysql.cnf :
	echo '[admin]' > WACmysql.cnf; \
        echo 'database=$(WAC_DATABASE_NAME)' >> WACmysql.cnf; \
        echo 'host=$(WAC_ADMIN_SYS)' >> WACmysql.cnf; \
        echo 'user=$(WAC_ADMIN_NAME)' >> WACmysql.cnf
##############################################################
# Compiling library and programs


lib/libWAC.so : $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) -o lib/libWAC.so $(LIBFLAGS) $(LIBS)

Reader.o : include/Reader.h src/Reader.C
	$(CC) $(CFLAGS) -fPIC -c src/Reader.C -o Reader.o $(DEFFLAGS) $(INCFLAGS)

DataFile.o : include/DataFile.h src/DataFile.C
	$(CC) $(CFLAGS) -fPIC -c src/DataFile.C -o DataFile.o $(DEFFLAGS) $(INCFLAGS)

WACUtil.o : include/WACUtil.h src/WACUtil.C
	$(CC) $(CFLAGS) -fPIC -c src/WACUtil.C -o WACUtil.o $(DEFFLAGS) $(SPECIALDEFS) $(INCFLAGS)


bin/Sorter : src/Sorter.C lib/libWAC.so
	$(CC) -O2 $(CFLAGS) $(LDFLAGS) src/Sorter.C -o bin/Sorter $(DEFFLAGS) $(INCFLAGS) -I $(REGEX_INC_PATH) \
              $(LIBFLAGS) -L $(REGEX_LIB_PATH) -l WAC -l boost_regex $(LIBS)


bin/Load_Sorted_Data : src/Load_Sorted_Data.C lib/libWAC.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/Load_Sorted_Data.C -o bin/Load_Sorted_Data \
              $(DEFFLAGS) $(INCFLAGS) -I $(MYSQLPP_INC_PATH) $(MYSQL_INC) \
              $(LIBFLAGS) -L $(MYSQLPP_LIB_PATH) -l WAC -l mysqlpp $(LIBS)


bin/Load_Volume_Data : src/Load_Volume_Data.C lib/libWAC.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/Load_Volume_Data.C -o bin/Load_Volume_Data \
              $(DEFFLAGS) $(INCFLAGS) -I $(MYSQLPP_INC_PATH) $(MYSQL_INC) \
              $(LIBFLAGS) -L $(MYSQLPP_LIB_PATH) -l WAC -l mysqlpp $(LIBS)


bin/Make_Archive_Tables: src/Make_Archive_Tables.C lib/libWAC.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/Make_Archive_Tables.C -o bin/Make_Archive_Tables \
              $(DEFFLAGS) $(INCFLAGS) -I $(MYSQLPP_INC_PATH) $(MYSQL_INC) \
              $(LIBFLAGS) -L $(MYSQLPP_LIB_PATH) -l WAC -l mysqlpp $(LIBS)


###############################################################################


###############################################################################
#  making directories


$(INSTALLPATH)/ToBeCatalogued :
	mkdir $(INSTALLPATH)/ToBeCatalogued

$(INSTALLPATH)/ToBeSorted :
	mkdir $(INSTALLPATH)/ToBeSorted

$(INSTALLPATH)/ToBeLoaded :
	mkdir $(INSTALLPATH)/ToBeLoaded

$(INSTALLPATH)/Catalogued :
	mkdir $(INSTALLPATH)/Catalogued

$(INSTALLPATH)/Sorted :
	mkdir $(INSTALLPATH)/Sorted

$(INSTALLPATH)/Loaded :
	mkdir $(INSTALLPATH)/Loaded

$(INSTALLPATH)/Commands :
	mkdir $(INSTALLPATH)/Commands

$(INSTALLPATH)/ConfigFiles :
	mkdir $(INSTALLPATH)/ConfigFiles
###################################################################################

###################################################################################
# Installers
$(MYLIB_PATH)/libWAC.so : lib/libWAC.so
	cp -f lib/libWAC.so $(MYLIB_PATH)/libWAC.so

$(INSTALLPATH)/Commands/Sorter : bin/Sorter
	cp -f bin/Sorter $(INSTALLPATH)/Commands/Sorter

$(INSTALLPATH)/Commands/Make_Archive_Tables : bin/Make_Archive_Tables
	cp -f bin/Make_Archive_Tables $(INSTALLPATH)/Commands/Make_Archive_Tables

$(INSTALLPATH)/Commands/Load_Sorted_Data : bin/Load_Sorted_Data
	cp -f bin/Load_Sorted_Data $(INSTALLPATH)/Commands/Load_Sorted_Data

$(INSTALLPATH)/Commands/Load_Volume_Data : bin/Load_Volume_Data
	cp -f bin/Load_Volume_Data $(INSTALLPATH)/Commands/Load_Volume_Data

$(INSTALLPATH)/Commands/ListVolume.pl : src/ListVolume.pl $(INSTALLPATH)/ConfigFiles/.WACmysql.cnf
	cp -f src/ListVolume.pl $(INSTALLPATH)/Commands/ListVolume.pl

$(INSTALLPATH)/Commands/BuildCatalog.pl : src/BuildCatalog.pl
	cp -f src/BuildCatalog.pl $(INSTALLPATH)/Commands/BuildCatalog.pl


$(MYINC_PATH)/Reader.h : include/Reader.h
	cp -f include/Reader.h $(MYINC_PATH)/Reader.h

$(MYINC_PATH)/DataFile.h : include/DataFile.h
	cp -f include/DataFile.h $(MYINC_PATH)/DataFile.h

$(MYINC_PATH)/WACUtil.h : include/WACUtil.h
	cp -f include/WACUtil.h $(MYINC_PATH)/WACUtil.h

$(INSTALLPATH)/ConfigFiles/.WACmysql.cnf : WACmysql.cnf
	cp -f WACmysql.cnf $(INSTALLPATH)/ConfigFiles/.WACmysql.cnf
####################################################################################

####################################################################################
# Cleaners/Uninstallers
clean :
	- rm -f $(OBJECTS)
	- rm -f lib/libWAC.so
	- rm -f WACmysql.cnf
	- rm -f	bin/Sorter
	- rm -f bin/Make_Archive_Tables
	- rm -f bin/Load_Sorted_Data
	- rm -f bin/Load_Volume_Data

uninstall : remove

remove :
	- rm -f $(MYLIB_PATH)/libWAC.so
	- rm -f $(INSTALLPATH)/ConfigFiles/.WACmysql.cnf
	- rm -f $(INSTALLPATH)/Commands/Sorter
	- rm -f $(INSTALLPATH)/Commands/Make_Archive_Tables
	- rm -f $(INSTALLPATH)/Commands/Load_Sorted_Data
	- rm -f $(INSTALLPATH)/Commands/Load_Volume_Data
	- rm -f $(INSTALLPATH)/Commands/ListVolume.pl
	- rm -f $(INSTALLPATH)/Commands/BuildCatalog.pl
	- rm -f $(LISTERS)
	- rm -f $(MYINC_PATH)/Reader.h
	- rm -f $(MYINC_PATH)/DataFile.h
	- rm -f $(MYINC_PATH)/WACUtil.h
# Maybe add some other removers?
###################################################################################


