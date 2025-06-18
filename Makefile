
#Configure
NB_CFG_PRINT_INTERNALS := 1
NB_CFG_PRINT_INFO      := 0

#Import functions
include ../../sys-nbframework/lib-nbframework-src/MakefileFuncs.mk

#Init workspace
$(eval $(call nbCall,nbInitWorkspace))

#Import projects
include MakefileProject.mk

