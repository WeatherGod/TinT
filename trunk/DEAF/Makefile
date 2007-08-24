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
	-rm -f "DEAF$(VERSION).tar"
	-rm -f "DEAF$(VERSION).tar.gz"
	-rm -f "DEAF$(VERSION).tar.bz2"
	cd src; make clean
	cd Plugins; make clean

tar : DEAF$(VERSION).tar

tar.gz : DEAF$(VERSION).tar.gz

tar.bz2 : DEAF$(VERSION).tar.bz2

DEAF$(VERSION).tar.gz : DEAF$(VERSION).tar
	gzip -f "DEAF$(VERSION).tar"

DEAF$(VERSION).tar.bz2 : DEAF$(VERSION).tar
	bzip2 -f "DEAF$(VERSION).tar"

DEAF$(VERSION).tar : Plugins/Plugins.tar deaf.config include.mk Makefile README \
           include/DeafBean.h include/DeafConf.h include/DeafInfo_t.h \
           include/DeafPlugin.h include/deafUtil.h include/PluginConf.h \
           src/DeafConf.C src/DeafInfo_t.C src/deafUtil.C src/PluginConf.C \
           src/deaf.C src/Make_DeafTemplate.pl src/Makefile
	mkdir -p "DEAF$(VERSION)/include/"
	mkdir -p "DEAF$(VERSION)/src/"
	cp -f deaf.config include.mk Makefile README "DEAF$(VERSION)/"
	cp -f include/DeafBean.h include/DeafConf.h include/DeafInfo_t.h \
              include/DeafPlugin.h include/deafUtil.h include/PluginConf.h \
              "DEAF$(VERSION)/include/"
	cp -f src/DeafConf.C src/DeafInfo_t.C src/deafUtil.C src/PluginConf.C \
              src/deaf.C src/Make_DeafTemplate.pl src/Makefile \
              "DEAF$(VERSION)/src/"
	tar -xf Plugins/Plugins.tar -C "DEAF$(VERSION)/"
	-rm -f "DEAF$(VERSION).tar"
	tar -cf "DEAF$(VERSION).tar" "DEAF$(VERSION)/"
	rm -rf "DEAF$(VERSION)/"

Plugins/Plugins.tar :
	cd Plugins; make Plugins.tar

