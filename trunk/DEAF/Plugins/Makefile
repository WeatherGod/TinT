
include ../include.mk

plugins : 
	@for PluginDir in * ; do \
            if [ -e $$PluginDir/Makefile ]; then \
                cd $$PluginDir; make plugin; cd ..; \
            fi; \
        done


install : $(DEAF_INSTALL_PATH)/Plugins plugins
	@for PluginDir in *; do \
            if [ -e $$PluginDir/Makefile ]; then \
                cd $$PluginDir; make install; cd ..; \
            fi; \
        done
	

$(DEAF_INSTALL_PATH)/Plugins :
	mkdir -p $(DEAF_INSTALL_PATH)/Plugins


clean :
	-rm -f Plugins.tar
	-rm -f Plugins.tar.gz
	-rm -f Plugins.tar.bz2
	@for PluginDir in *; do \
            if [ -e $$PluginDir/Makefile ]; then \
                cd $$PluginDir; make clean; cd ..; \
            fi; \
        done

remove : uninstall

uninstall :
	@for PluginDir in *; do \
            if [ -e $$PluginDir/Makefile ]; then \
                cd $$PluginDir; make uninstall; cd ..; \
            fi; \
        done
	-rm -fr $(DEAF_INSTALL_PATH)/Plugins

tar : Plugins.tar

tar.gz : Plugins.tar.gz

tar.bz2 : Plugins.tar.bz2

Plugins.tar.gz : Plugins.tar
	gzip -f Plugins.tar

Plugins.tar.bz2 : Plugins.tar
	bzip2 -f Plugins.tar

Plugins.tar :
	mkdir Plugins/
	@for PluginDir in *; do \
            if [ -e $$PluginDir/Makefile ]; then \
                cd $$PluginDir; make $$PluginDir.tar; cd ..; \
                tar -xf "$$PluginDir/$$PluginDir.tar" -C Plugins/; \
            fi; \
        done
#------- Note: This copy statement MUST come after the loop, or else the 
#-------       loop will think that this temporary directory is another plugin
	cp -f Makefile Plugins/
	-rm -rf Plugins.tar
	tar -cf Plugins.tar Plugins/
	rm -rf Plugins/

