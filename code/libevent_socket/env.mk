
CC := gcc

#头文件目录
INCLUDE_DIR := -I$(MAKEROOT)/common \

CXXFLAGS := $(INCLUDE_DIR) -Wno-write-strings -std=c++11 -g -fno-strict-aliasing -levent  

#对所有的.o文件以.c文件创建它
%.o : %.cpp
	${CC} ${CXXFLAGS} -c $< -o $(MAKEROOT)/obj/$@

define build_obj
	@echo $(TARGET_DIRS)
	for SubDir in $(TARGET_DIRS); do \
		if ! [ -d $$SubDir ]; then \
	        echo "The $$SubDir is not exist !"; \
	        exit 11; \
	    fi; \
	    echo "Building $$SubDir ..."; \
	    make -C $$SubDir ; \
	    if [ $$? -ne 0 ]; then \
	        echo "Building $$SubDir is failed !"; \
	        exit 12; \
	    fi; \
	done
endef
