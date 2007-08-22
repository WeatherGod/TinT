CC = gcc
CFLAGS = -O2
OBJECTS = DeafConf.o DeafInfo_t.o PluginConf.o deafUtil.o

include ../include.mk

#-----------------------------------------------------------------------------

all : libDEAF.so deaf

deaf : libDEAF.so deaf.C
	$(CC) $(CFLAGS) $(LDFLAGS) -rdynamic deaf.C -o deaf \
              -I ../include $(BOOST_INC) $(XMLPP_INC) $(MYSQLPP_INC) \
              -L ./ $(BOOST_LIB) $(MYSQLPP_LIB) \
              -l DEAF -l stdc++

##################################################################################
############### For Install ######################################################

$(DEAF_LIB_PATH)/libDEAF.so : libDEAF.so
	cp libDEAF.so $(DEAF_LIB_PATH)/libDEAF.so

$(DEAF_BIN_PATH)/deaf : deaf
	cp deaf $(DEAF_BIN_PATH)/deaf

$(DEAF_BIN_PATH)/Make_DeafTemplate.pl : Make_DeafTemplate.pl
	cp Make_DeafTemplate.pl $(DEAF_BIN_PATH)/Make_DeafTemplate.pl

$(DEAF_INC_PATH)/DeafBean.h : ../include/DeafBean.h
	cp ../include/DeafBean.h $(DEAF_INC_PATH)/DeafBean.h

$(DEAF_INC_PATH)/DeafPlugin.h : ../include/DeafPlugin.h
	cp ../include/DeafPlugin.h $(DEAF_INC_PATH)/DeafPlugin.h

##########################################################################################


libDEAF.so : $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) -o libDEAF.so \
              $(BOOST_LIB) $(XMLPP_LIB) $(MYSQLPP_LIB) \
              -l stdc++ -l dl

DeafConf.o : DeafConf.C
	$(CC) $(CFLAGS) -fPIC -c DeafConf.C -o DeafConf.o -I ../include $(MYSQLPP_INC) $(XMLPP_INC)

DeafInfo_t.o : DeafInfo_t.C
	$(CC) $(CFLAGS) -fPIC -c DeafInfo_t.C -o DeafInfo_t.o -I ../include $(MYSQLPP_INC)

PluginConf.o : PluginConf.C
	$(CC) $(CFLAGS) -fPIC -c PluginConf.C -o PluginConf.o -I ../include $(XMLPP_INC)

deafUtil.o : deafUtil.C
	$(CC) $(CFLAGS) -fPIC -c deafUtil.C -o deafUtil.o -I ../include $(MYSQLPP_INC) $(BOOST_INC)
##############################################################################################


install : $(DEAF_LIB_PATH)/libDEAF.so $(DEAF_BIN_PATH)/deaf $(DEAF_BIN_PATH)/Make_DeafTemplate.pl \
          $(DEAF_INC_PATH)/DeafBean.h $(DEAF_INC_PATH)/DeafPlugin.h

uninstall : remove

remove :
	- rm -f $(DEAF_LIB_PATH)/libDEAF.so
	- rm -f $(DEAF_BIN_PATH)/deaf
	- rm -f $(DEAF_BIN_PATH)/Make_DeafTemplate.pl
	- rm -f $(DEAF_INC_PATH)/DeafBean.h
	- rm -f $(DEAF_INC_PATH)/DeafPlugin.h

clean :
	- rm -f $(OBJECTS) libDEAF.so deaf
