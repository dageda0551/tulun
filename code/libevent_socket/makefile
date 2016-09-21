
export MAKEROOT := $(shell pwd)

TARGET_DIRS := common meta

include ./env.mk

all :
	@$(call build_obj)

clean :
	-rm -f ./obj/*.o ./out ./meta/*.o 
