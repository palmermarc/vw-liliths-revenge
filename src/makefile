CC      = gcc
PROF    = -ggdb
C_FLAGS = -g -O -Wall -Dlinux $(PROF) 
L_FLAGS = -O -lcrypt -lm $(PROF)
OBJDIR  = ./obj
EXENAME = vw

O_FILES := $(patsubst %.c,$(OBJDIR)/%.o,$(wildcard *.c))

$(EXENAME): $(OBJDIR) $(O_FILES)
	@echo ---
	@echo Linking: $@
	@rm -f $(EXENAME)
	@$(CC) -o $(EXENAME) $(O_FILES) $(L_FLAGS)

$(OBJDIR):
	@mkdir $(OBJDIR) 

$(OBJDIR)/%.o: %.c merc.h
	@echo $<
	@$(CC) -c $(C_FLAGS) $< -o $@ 

clean:
	@rm -rf $(OBJDIR)
	@rm -f $(EXENAME)

