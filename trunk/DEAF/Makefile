include include.mk

all : source plugins

source :
	cd src; make all

deaf :
	cd src; make deaf

libDEAF.so :
	cd src; make libDEAF.so

plugins :
	cd Plugins; make plugins


install : $(DEAF_INSTALL_PATH) all
	cd src; make install
	cd Plugins; make install

$(DEAF_INSTALL_PATH) :
	mkdir -P $(DEAF_INSTALL_PATH)


uninstall :
	cd src; make uninstall
	cd Plugins; make uninstall
	-rmdir $(DEAF_INSTALL_PATH)

remove : uninstall

clean :
	cd src; make clean
	cd Plugins; make clean
