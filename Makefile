CFLAGS+= -Wextra -Wall -Wno-unused-parameter -g -Wno-multichar -Wno-unused-but-set-variable -O3
# libs to include when linking (with -l<name> flags)
libs:= z
# c source code files
srcs:= apng
output:= apng-hack

ifdef seed
creats != seq 0.3 0.1 2.0
frame = $(seed)/%.png

$(seed).apng: $(creats:%=$(frame))
	cat $^ | ./apng-hack 18 1 12 >$@
	$(RM) $(dir $(frame))

$(frame): $(seed)
	curl -s https://thisanimedoesnotexist.ai/results/psi-$(@:$(frame)=%)/seed$(seed).png -o $@

$(seed):
	mkdir -p $@
endif

include Nice.mk
