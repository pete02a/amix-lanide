# AMIX DLM module build. KIT must be the matching generated k46 kit.
ifndef KIT
$(error Set KIT to the absolute path of the generated AMIX k46 DLM kit)
endif
MODULE = lanide
OBJS = lanide.o hw.o
include $(KIT)/mk/module.mk
