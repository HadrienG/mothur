###################################################
#
# Makefile for mothur
#
###################################################

#
# Macros
#
# 64BIT_VERSION - set to no if you are using a 32bit arch.
# OPTIMIZE - yes will increase speed of executable.
# USEREADLINE - link with readline libraries.  Must have readline installed. Windows set to no.
# USEBOOST - link with boost libraries. Must install boost. Allows the make.contigs command to read .gz files.
# BOOST_LIBRARY_DIR - location of boost libraries
# BOOST_INCLUDE_DIR - location of boost include files
# MOTHUR_FILES - default location for mothur to look for input files at runtime. Most often used for reference files.

PREFIX := ${CURDIR} 

64BIT_VERSION ?= yes
OPTIMIZE ?= yes
USEREADLINE ?= yes
USEBOOST ?= yes
BOOST_LIBRARY_DIR="\"Enter_your_boost_library_path_here\""
BOOST_INCLUDE_DIR="\"Enter_your_boost_include_path_here\""
MOTHUR_FILES="\"Enter_your_default_path_here\""
RELEASE_DATE = "\"3/20/2017\""
VERSION = "\"1.39.5\""

ifeq  ($(strip $(64BIT_VERSION)),yes)
    CXXFLAGS += -DBIT_VERSION
endif

# Fastest
ifeq  ($(strip $(OPTIMIZE)),yes)
    CXXFLAGS += -O3
endif

CXXFLAGS += -DRELEASE_DATE=${RELEASE_DATE} -DVERSION=${VERSION} -std=c++0x
LDFLAGS += -std=c++0x

ifeq  ($(strip $(MOTHUR_FILES)),"\"Enter_your_default_path_here\"")
else
    CXXFLAGS += -DMOTHUR_FILES=${MOTHUR_FILES}
endif


# if you do not want to use the readline library, set this to no.
# make sure you have the library installed


ifeq  ($(strip $(USEREADLINE)),yes)
    CXXFLAGS += -DUSE_READLINE
    LIBS += -lreadline
endif


#User specified boost library
ifeq  ($(strip $(USEBOOST)),yes)

    LDFLAGS += -L ${BOOST_LIBRARY_DIR}

    LIBS += -lboost_iostreams -lz
    CXXFLAGS += -DUSE_BOOST -I ${BOOST_INCLUDE_DIR}
endif

#
# INCLUDE directories for mothur
#
#
    VPATH=source/calculators:source/chimera:source/classifier:source/clearcut:source/commands:source/communitytype:source/datastructures:source/metastats:source/randomforest:source/read:source/svm
    skipUchime := source/uchime_src/
    subdirs :=  $(sort $(dir $(filter-out  $(skipUchime), $(wildcard source/*/))))
    subDirIncludes = $(patsubst %, -I %, $(subdirs))
    subDirLinking =  $(patsubst %, -L%, $(subdirs))
    CXXFLAGS += -I. $(subDirIncludes)
    LDFLAGS += $(subDirLinking)


#
# Get the list of all .cpp files, rename to .o files
#
    OBJECTS=$(patsubst %.cpp,%.o,$(wildcard $(addsuffix *.cpp,$(subdirs))))
    OBJECTS+=$(patsubst %.c,%.o,$(wildcard $(addsuffix *.c,$(subdirs))))
    OBJECTS+=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
    OBJECTS+=$(patsubst %.c,%.o,$(wildcard *.c))

mothur : $(OBJECTS) uchime
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $(OBJECTS) $(LIBS)

uchime:
	cd source/uchime_src && ./mk && mv uchime ../../ && cd ..

install : mothur uchime
	if [ "${CURDIR}" = "$(PREFIX)" ]; then \
		echo 'done'; \
	else \
		mkdir -p $(PREFIX); \
		for file in mothur uchime; do \
			cp -f $$file $(PREFIX) ; \
		done \
	fi


%.o : %.c %.h
	$(COMPILE.c) $(OUTPUT_OPTION) $<
%.o : %.cpp %.h
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
%.o : %.cpp %.hpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<



clean :
	@rm -f $(OBJECTS)
	@rm -f uchime
