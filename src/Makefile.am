bin_PROGRAMS = callbuilder
callbuilder_SOURCES = CDRBuilder.cpp CDRRadius.cpp UniNumber.cpp \
	callbuilder.cpp callbuilder_help.cpp client.cpp monmessage.cpp netmes.cpp parser.cpp \
	radius_avapair.cpp radius_buildreq.cpp radius_cliedtid.cpp radius_config.cpp \
	radius_dict.cpp radius_env.cpp radius_ip_util.cpp radius_lock.cpp radius_log.cpp \
	radius_md5.cpp radius_sendserver.cpp radius_util.cpp systcp.cpp unipar.cpp \
	unipar_pf.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
callbuilder_LDFLAGS = $(all_libraries) 
callbuilder_LDFLAGS += -lpthread 
noinst_HEADERS = CDRBuilder.h CDRRadius.h Errors.h UniNumber.h callbuilder.h \
	client.h dictionary dictionary.cisco lib.h message.h monmessage.h netmes.h \
	parser.h radius_freeradius-client.h radius_includes.h radius_md5.h \
	radius_messages.h radius_options.h radius_pathnames.h systcp.h type.h unipar.h
