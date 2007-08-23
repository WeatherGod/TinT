all : TinT

TinT :
	cd VolumeExtractor; make all
	cd DEAF; make all

clean : 
	-rm -f TinT.tar
	-rm -f TinT.tar.gz
	-rm -f TinT.tar.bz2
	cd VolumeExtractor; make clean
	cd DEAF; make clean

uninstall : remove

remove :
	cd VolumeExtractor; make remove
	cd DEAF; make remove

tar : TinT.tar

tar.gz : TinT.tar.gz

tar.bz2 : TinT.tar.bz2

TinT.tar.gz : TinT.tar
	gzip -f TinT.tar

TinT.tar.bz2 : TinT.tar
	bzip2 -f TinT.tar

TinT.tar : VolumeExtractor/VolumeExtractor.tar DEAF/DEAF.tar Makefile
	-rm -f TinT.tar
	tar -cf TinT.tar Makefile
	tar -Af TinT.tar VolumeExtractor/VolumeExtractor.tar
	tar -Af TinT.tar DEAF/DEAF.tar

VolumeExtractor/VolumeExtractor.tar :
	cd VolumeExtractor; make VolumeExtractor.tar

DEAF/DEAF.tar :
	cd DEAF; make DEAF.tar 
