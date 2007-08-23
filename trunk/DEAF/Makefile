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
	-rm -f DEAF.tar
	-rm -f DEAF.tar.gz
	-rm -f DEAF.tar.bz2
	cd src; make clean
	cd Plugins; make clean

tar : DEAF.tar

tar.gz : DEAF.tar.gz

tar.bz2 : DEAF.tar.bz2

DEAF.tar.gz : DEAF.tar
	gzip -f DEAF.tar

DEAF.tar.bz2 : DEAF.tar
	bzip2 -f DEAF.tar

DEAF.tar : Plugins/Plugins.tar deaf.config include.mk Makefile README \
           include/DeafBean.h include/DeafConf.h include/DeafInfo_t.h \
           include/DeafPlugin.h include/deafUtil.h include/PluginConf.h \
           src/DeafConf.C src/DeafInfo_t.C src/deafUtil.C src/PluginConf.C \
           src/deaf.C src/Make_DeafTemplate.pl src/Makefile
	-rm -f DEAF.tar
	tar -cf DEAF.tar --no-recursion \
            ../DEAF/deaf.config ../DEAF/include.mk ../DEAF/Makefile ../DEAF/README \
            ../DEAF/include/DeafBean.h ../DEAF/include/DeafConf.h ../DEAF/include/DeafInfo_t.h \
            ../DEAF/include/DeafPlugin.h ../DEAF/include/deafUtil.h ../DEAF/include/PluginConf.h \
            ../DEAF/src/DeafConf.C ../DEAF/src/DeafInfo_t.C ../DEAF/src/deafUtil.C ../DEAF/src/PluginConf.C \
            ../DEAF/src/deaf.C ../DEAF/src/Make_DeafTemplate.pl ../DEAF/src/Makefile
	tar -Af DEAF.tar Plugins/Plugins.tar

Plugins/Plugins.tar :
	cd Plugins; make Plugins.tar

