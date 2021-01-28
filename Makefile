CFLAGS?= -Wextra -Wall -Wno-unused-parameter -g
# libs to include when linking (with -l<name> flags)
#libs:= X11 lua5.3
# c source code files
srcs:= info
# location for intermediate files (.o and .mk)
junkdir:= .junk

CC=@echo '[33m'$@'	[37mfrom: [32m'$^'[m' ; cc

parse: $(srcs:%=$(junkdir)/%.o)
	$(CC) $(CFLAGS) $(addprefix -l,$(libs)) $^ -o $@

$(junkdir)/%.mk: %.c
	@mkdir -p $(dir $<)
	$(CC) $(CFLAGS) -MF$@ -MG -MM -MP -MT$@ -MT$(<:%.c=$(junkdir)/%.o) $<

$(junkdir)/%.o: %.c
	@mkdir -p $(dir $<)
	$(CC) $(CFLAGS) -c $< -o $@

#PROBLEM: this includes files even if `clean` is being run, causing it to generate dependency files xd
include $(srcs:%=$(junkdir)/%.mk)

#clean:
#	find $(junkdir) -type f -delete
#	$(RM) 12wm

#.PHONY: clean
