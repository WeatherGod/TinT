CC = gcc
CFLAGS = -O2

include ../../include.mk

##################################################
MYINC = -I /usr/include/BUtils
MYLIB = -L /usr/lib -l BMOS -l BUtils
##################################################


MYBEAN_DIR = ../WMO_MOS
PLUGINNAME = KleitMOS
BEANNAME = MOSBean
DEAFBUILD_DIR = $(PWD)

#-----------------------------------------------------------------------

plugin : lib$(PLUGINNAME).so

lib$(PLUGINNAME).so : $(BEANNAME).o $(PLUGINNAME).o
	$(CC) $(LDFLAGS) -shared $(PLUGINNAME).o $(BEANNAME).o  -o lib$(PLUGINNAME).so \
              $(MYLIB) $(MYSQLPP_LIB) -l stdc++


$(BEANNAME).o : $(MYBEAN_DIR)/$(BEANNAME).o
	- cp -u $(MYBEAN_DIR)/$(BEANNAME).o $(DEAFBUILD_DIR)/$(BEANNAME).o

$(MYBEAN_DIR)/$(BEANNAME).o :
	cd $(MYBEAN_DIR); \
	make DEAFBUILD_DIR='$(DEAFBUILD_DIR)' $(BEANNAME).o

$(PLUGINNAME).o : $(PLUGINNAME).h $(PLUGINNAME).C
	$(CC) $(CFLAGS) -fPIC -c $(PLUGINNAME).C -o $(PLUGINNAME).o -D _DOLINKING_ \
              -I ./ -I ../../include -I $(MYBEAN_DIR) $(MYINC) $(MYSQLPP_INC)

clean :
	- rm -f lib$(PLUGINNAME).so
	- rm -f $(BEANNAME).o
	- rm -f $(PLUGINNAME).o


remove : uninstall

uninstall :
	- rm -f $(DEAF_INC_PATH)/$(PLUGINNAME).h


install : $(DEAF_INC_PATH)/$(PLUGINNAME).h

$(DEAF_INC_PATH)/$(PLUGINNAME).h : $(PLUGINNAME).h
	cp -f $(PLUGINNAME).h $(DEAF_INC_PATH)/$(PLUGINNAME).h

