
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
	-rm -r $(DEAF_INSTALL_PATH)/Plugins

