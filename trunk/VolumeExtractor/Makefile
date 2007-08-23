CC = gcc
CFLAGS = -O2
OBJECTS = FileGroup.o FileGroupID_t.o FileID_t.o FileType.o JobClass.o VolID_t.o VolumeClass.o VolumeInfo.o

DEFFLAGS = -D _DOLINKING_

####################################################################################
INSTALL_LIB = /usr/lib
INSTALL_INC = /usr/include
INSTALL_BIN = /usr/bin


BUTILS_LIB_PATH = /usr/lib
BUTILS_INC_PATH = /usr/include/BUtils
####################################################################################


LIBS = -l BUtils -l stdc++
LIBFLAGS = -L $(BUTILS_LIB_PATH) -L lib/
INCFLAGS = -I $(BUTILS_INC_PATH) -I include/


all : libShump.so MakeJob ListJob ModifyJob shump

MakeJob : bin/MakeJob

bin/MakeJob : src/MakeJob.C lib/libShump.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/MakeJob.C -o bin/MakeJob $(DEFFLAGS) $(INCFLAGS) $(LIBFLAGS) -l Shump $(LIBS)

ListJob : bin/ListJob

bin/ListJob : src/ListJob.C lib/libShump.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/ListJob.C -o bin/ListJob $(DEFFLAGS) $(INCFLAGS) $(LIBFLAGS) -l Shump $(LIBS)

ModifyJob : bin/ModifyJob

bin/ModifyJob : src/ModifyJob.C lib/libShump.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/ModifyJob.C -o bin/ModifyJob $(DEFFLAGS) $(INCFLAGS) $(LIBFLAGS) -l Shump $(LIBS)

shump : bin/shump

bin/shump : src/shump.C lib/libShump.so
	$(CC) $(CFLAGS) $(LDFLAGS) src/shump.C -o bin/shump $(DEFFLAGS) $(INCFLAGS) $(LIBFLAGS) -l Shump $(LIBS)

libShump.so : lib/libShump.so

lib/libShump.so : $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) -o lib/libShump.so $(LIBFLAGS) -l stdc++

FileGroup.o : include/FileGroup.h src/FileGroup.C
	$(CC) $(CFLAGS) -fPIC -c src/FileGroup.C -o FileGroup.o $(DEFFLAGS) $(INCFLAGS)

FileGroupID_t.o : include/FileGroupID_t.h src/FileGroupID_t.C
	$(CC) $(CFLAGS) -fPIC -c src/FileGroupID_t.C -o FileGroupID_t.o $(DEFFLAGS) $(INCFLAGS)

FileID_t.o : include/FileID_t.h src/FileID_t.C
	$(CC) $(CFLAGS) -fPIC -c src/FileID_t.C -o FileID_t.o $(DEFFLAGS) $(INCFLAGS)

FileType.o : include/FileType.h src/FileType.C
	$(CC) $(CFLAGS) -fPIC -c src/FileType.C -o FileType.o $(DEFFLAGS) $(INCFLAGS)

JobClass.o : include/JobClass.h src/JobClass.C
	$(CC) $(CFLAGS) -fPIC -c src/JobClass.C -o JobClass.o $(DEFFLAGS) $(INCFLAGS)

VolID_t.o : include/VolID_t.h src/VolID_t.C
	$(CC) $(CFLAGS) -fPIC -c src/VolID_t.C -o VolID_t.o $(DEFFLAGS) $(INCFLAGS)

VolumeClass.o : include/VolumeClass.h src/VolumeClass.C
	$(CC) $(CFLAGS) -fPIC -c src/VolumeClass.C -o VolumeClass.o $(DEFFLAGS) $(INCFLAGS)

VolumeInfo.o : include/VolumeInfo.h src/VolumeInfo.C
	$(CC) $(CFLAGS) -fPIC -c src/VolumeInfo.C -o VolumeInfo.o $(DEFFLAGS) $(INCFLAGS)

##### Still need to install the header files.
$(INSTALL_INC)/FileGroup.h : include/FileGroup.h
	cp -f include/FileGroup.h $(INSTALL_INC)/FileGroup.h

$(INSTALL_INC)/FileGroupID_t.h : include/FileGroupID_t.h
	cp -f include/FileGroupID_t.h $(INSTALL_INC)/FileGroupID_t.h

$(INSTALL_INC)/FileID_t.h : include/FileID_t.h
	cp -f include/FileID_t.h $(INSTALL_INC)/FileID_t.h

$(INSTALL_INC)/FileType.h : include/FileType.h
	cp -f include/FileType.h $(INSTALL_INC)/FileType.h

$(INSTALL_INC)/JobClass.h : include/JobClass.h
	cp -f include/JobClass.h $(INSTALL_INC)/JobClass.h

$(INSTALL_INC)/VolID_t.h : include/VolID_t.h
	cp -f include/VolID_t.h $(INSTALL_INC)/VolID_t.h

$(INSTALL_INC)/VolumeClass.h : include/VolumeClass.h
	cp -f include/VolumeClass.h $(INSTALL_INC)/VolumeClass.h

$(INSTALL_INC)/VolumeInfo.h : include/VolumeInfo.h
	cp -f include/VolumeInfo.h $(INSTALL_INC)/VolumeInfo.h




install : $(INSTALL_LIB)/libShump.so \
          $(INSTALL_INC)/FileGroup.h $(INSTALL_INC)/FileGroupID_t.h $(INSTALL_INC)/FileID_t.h \
          $(INSTALL_INC)/FileType.h $(INSTALL_INC)/JobClass.h $(INSTALL_INC)/VolID_t.h \
          $(INSTALL_INC)/VolumeClass.h $(INSTALL_INC)/VolumeInfo.h \
          $(INSTALL_BIN)/MakeJob $(INSTALL_BIN)/ListJob $(INSTALL_BIN)/ModifyJob $(INSTALL_BIN)/shump

$(INSTALL_LIB)/libShump.so : lib/libShump.so
	cp -f lib/libShump.so $(INSTALL_LIB)/libShump.so

$(INSTALL_BIN)/MakeJob : bin/MakeJob
	cp -f bin/MakeJob $(INSTALL_BIN)/MakeJob

$(INSTALL_BIN)/ListJob : bin/ListJob
	cp -f bin/ListJob $(INSTALL_BIN)/ListJob

$(INSTALL_BIN)/ModifyJob : bin/ModifyJob
	cp -f bin/ModifyJob $(INSTALL_BIN)/ModifyJob

$(INSTALL_BIN)/shump : bin/shump
	cp -f bin/shump $(INSTALL_BIN)/shump


clean :
	- rm -f $(OBJECTS)
	- rm -f lib/libShump.so
	- rm -f bin/MakeJob
	- rm -f bin/ListJob
	- rm -f bin/ModifyJob
	- rm -f bin/shump
	- rm -f VolumeExtractor.tar
	- rm -f VolumeExtractor.tar.gz
	- rm -f VolumeExtractor.tar.bz2

uninstall : remove

remove :
	- rm -f $(INSTALL_LIB)/libShump.so
	- rm -f $(INSTALL_BIN)/MakeJob
	- rm -f $(INSTALL_BIN)/ListJob
	- rm -f $(INSTALL_BIN)/ModifyJob
	- rm -f $(INSTALL_BIN)/shump
	- rm -f $(INSTALL_INC)/FileGroup.h
	- rm -f $(INSTALL_INC)/FileGroupID_t.h
	- rm -f $(INSTALL_INC)/FileID_t.h
	- rm -f $(INSTALL_INC)/FileType.h
	- rm -f $(INSTALL_INC)/JobClass.h
	- rm -f $(INSTALL_INC)/VolID_t.h
	- rm -f $(INSTALL_INC)/VolumeClass.h
	- rm -f $(INSTALL_INC)/VolumeInfo.h

tar : VolumeExtractor.tar

tar.gz : VolumeExtractor.tar.gz

tar.bz2 : VolumeExtractor.tar.bz2

VolumeExtractor.tar.gz : VolumeExtractor.tar
	gzip -f VolumeExtractor.tar

VolumeExtractor.tar.bz2 : VolumeExtractor.tar
	bzip2 -f VolumeExtractor.tar

VolumeExtractor.tar : include/FileGroup.h include/FileGroupID_t.h include/FileID_t.h include/FileType.h \
                      include/JobClass.h include/VolID_t.h include/VolumeClass.h include/VolumeInfo.h \
                      src/FileGroup.C src/FileGroupID_t.C src/FileID_t.C src/FileType.C \
                      src/JobClass.C src/VolID_t.C src/VolumeClass.C src/VolumeInfo.C \
                      src/ListJob.C src/MakeJob.C src/ModifyJob.C src/shump.C \
                      lib bin Makefile README
	-rm -f VolumeExtractor.tar
	tar -cf VolumeExtractor.tar --no-recursion \
                ../VolumeExtractor/include/FileGroup.h ../VolumeExtractor/include/FileGroupID_t.h \
                ../VolumeExtractor/include/FileID_t.h ../VolumeExtractor/include/FileType.h \
                ../VolumeExtractor/include/JobClass.h ../VolumeExtractor/include/VolID_t.h \
                ../VolumeExtractor/include/VolumeClass.h ../VolumeExtractor/include/VolumeInfo.h \
                ../VolumeExtractor/src/FileGroup.C ../VolumeExtractor/src/FileGroupID_t.C ../VolumeExtractor/src/FileID_t.C \
                ../VolumeExtractor/src/FileType.C ../VolumeExtractor/src/JobClass.C ../VolumeExtractor/src/VolID_t.C \
                ../VolumeExtractor/src/VolumeClass.C ../VolumeExtractor/src/VolumeInfo.C \
                ../VolumeExtractor/src/ListJob.C ../VolumeExtractor/src/MakeJob.C \
                ../VolumeExtractor/src/ModifyJob.C ../VolumeExtractor/src/shump.C \
                ../VolumeExtractor/lib ../VolumeExtractor/bin ../VolumeExtractor/Makefile ../VolumeExtractor/README
