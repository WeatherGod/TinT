CC = gcc
CFLAGS = -O3

include ../../include.mk

#####################################################################################
#  for example: -I /home/Zaphod/include
MYINC = -I /usr/include/BUtils

#  for example: -L /home/Zaphod/lib -l ZaphodLib
MYLIB = -L /usr/lib -l BMetar -l BUtils
######################################################################################


MYBEAN_DIR = ./
PLUGINNAME = WMO_Metar
BEANNAME = MetarBean
DEAFBUILD_DIR = $(PWD)

#-----------------------------------------------------------------------

plugin : lib$(PLUGINNAME).so


lib$(PLUGINNAME).so : $(BEANNAME).o $(PLUGINNAME).o
	$(CC) $(LDFLAGS) -shared $(PLUGINNAME).o $(BEANNAME).o  -o lib$(PLUGINNAME).so \
              $(MYLIB) $(MYSQLPP_LIB) -l stdc++

$(BEANNAME).o : $(BEANNAME).C
	$(CC) $(CFLAGS) -fPIC -c $(BEANNAME).C -o $(DEAFBUILD_DIR)/$(BEANNAME).o -I ../../include $(MYINC)

$(PLUGINNAME).o : $(PLUGINNAME).C
	$(CC) $(CFLAGS) -fPIC -c $(PLUGINNAME).C -o $(PLUGINNAME).o -D _DOLINKING_ \
              -I ./ -I ../../include $(MYINC) $(MYSQLPP_INC)

clean :
	- rm -f lib$(PLUGINNAME).so
	- rm -f $(BEANNAME).o
	- rm -f $(PLUGINNAME).o

remove : uninstall

uninstall :
	- rm -f $(DEAF_INC_PATH)/$(BEANNAME).h
	- rm -f $(DEAF_INC_PATH)/$(PLUGINNAME).h

install : lib$(PLUGINNAME).so $(DEAF_INC_PATH)/$(PLUGINNAME).h $(DEAF_INC_PATH)/$(BEANNAME).h

$(DEAF_INC_PATH)/$(BEANNAME).h : $(BEANNAME).h
	cp -f $(BEANNAME).h $(DEAF_INC_PATH)/$(BEANNAME).h

$(DEAF_INC_PATH)/$(PLUGINNAME).h : $(PLUGINNAME).h
	cp -f $(PLUGINNAME).h $(DEAF_INC_PATH)/$(PLUGINNAME).h

