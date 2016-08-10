RM = rm -f
MAKE = make
COPY = cp
CC = g++
export CC

AR = ar
PS = cpp
LIBEXT=a
#CFLAGS = -rdynamic -Wall -m64  -pipe -fopenmp -fpermissive  -std=c++1y
CFLAGS = -rdynamic -Wall -m64  -pipe -fopenmp -std=c++1y
export CFLAGS

RELEASE_FLAGS = -O2 -Wno-deprecated	-Wno-unused-result -DELPP_THREAD_SAFE
DEBUG_FLAGS = -pg -g -gdwarf-2 -g3 -O0 -DLOG_CB -Wno-deprecated -Wno-unused-result  -DELPP_THREAD_SAFE

RELEASE	= asynobjectbool
DEBUG	= $(RELEASE)_d

TARGET = $(RELEASE) $(DEBUG)
DEPFLAGS = -MMD
LIBFLAGS =  rcs

#SOURCES = $(wildcard ./*.$(PS))
#SOURCES += $(wildcard ./common/*.$(PS))

ALL_INCLUDE = . ./include ./common ./common/http ./common/netlib ./common/nlohmannjson  ./common/connpool ./common/mysql
ALL_LIB_PATH = ./lib

ALL_LIBSS = tcmalloc
#ALL_LIBSD = pthread m dl rt z ssl crypto boost_thread boost_system
ALL_LIBSD =  m dl rt z ssl crypto ev

PATHS = . ./include  ./common ./common/http  ./common/netlib ./common/connpool ./common/message ./common/base64 ./common/urlencode ./aerospike ./process ./common/nlohmannjson ./common/mysql

SOURCES = $(foreach dir, $(PATHS), $(wildcard $(dir)/*.$(PS)))

OBJECTS_R = $(patsubst %.$(PS),%.o,$(SOURCES))

OBJECTS_D = $(patsubst %.$(PS),%d.o,$(SOURCES))

OBJS = $(OBJECTS_R) $(OBJECTS_D)

LIB_PATH = $(addprefix -L, $(ALL_LIB_PATH))

LIB_LINKS = $(addprefix -l, $(ALL_LIBSS)) 
LIB_LINKD = $(addprefix -l, $(ALL_LIBSD)) 
LIB_LINK = -Wl,-Bstatic $(LIB_LINKS) -Wl,-Bdynamic ${LIB_LINKD}

DEPS := $(patsubst %.o, %.d, $(OBJS))

MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))

MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.$(PS),$(MISSING_DEPS)))

#thirdparty software dirs-specified
#SUBDIRS = ./common/protobuf

INSTALL = ../../bin

.PHONY : all install deps objs clean veryclean debug release rebuild subdirs

all :   debug $(RELEASE)

debug : $(DEBUG)
subdirs: $(SUBDIRS)
#	@for dir in $(SUBDIRS);\
#	do\
#		$(MAKE) -C $${dir};\
#	done
install : debug
	@echo '----- copy new version -----'

release : debug $(RELEASE)
	@echo '----- copy new version -----'

$(DEBUG) : $(OBJECTS_D) $(addprefix -l,$(LIBS_D))
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) -o $@  $^  $(LIB_PATH) $(LIB_LINK)
$(RELEASE) : $(OBJECTS_R)  $(addprefix -l,$(LIBS_R))
	$(CC) $(RELEASE_FLAGS) $(CFLAGS) -o $@  $^  $(LIB_PATH) $(LIB_LINK)
#	@now=`date -d 'now' "+%Y%m%d%H%M%S"`;tar jcvf  $(DEBUG)$${now}.tar.bz2  $(DEBUG)
deps : $(DEPS)

objs : $(OBJS)

clean:
	@echo '----- cleaning obj file -----'
	@$(RM) $(OBJS)
	@echo '----- cleaning dependency file -----'
	@$(RM) $(DEPS)
	@echo '----- cleaning binary file -----'
	@$(RM) -f $(RELEASE)
	@$(RM) -f $(DEBUG)
	@$(RM) -f *.log  *.bz2 core core.* *.out

veryclean : clean cleansub
		@echo '----- cleaning execute file -----'
		@$(RM) $(TARGET)

rebuild: veryclean all

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) : subdirs 
	@$(RM) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)

$(OBJECTS_D) : %d.o : %.$(PS)
	$(CC) $(DEPFLAGS) $(DEBUG_FLAGS) $(CFLAGS) $(addprefix -I,$(ALL_INCLUDE)) -c $< -o $@

$(OBJECTS_R) : %.o : %.$(PS)
	$(CC) $(DEPFLAGS) $(RELEASE_FLAGS) $(CFLAGS) $(addprefix -I,$(ALL_INCLUDE)) -c $< -o $@
