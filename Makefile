ded_flags = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef \
-Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations		\
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain   \
-Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion            \
-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2           \
-Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers       \
-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo    \
-Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel             \
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

.phony: all clean intall uninstall

B_PREFIX = bin/
O_PREFIX = objects/
S_PREFIX = sources/
T_PREFIX = text_files/
SOURCES = main commoner stack
OBJECTS := $(patsubst %,$(O_PREFIX)%.o,$(SOURCES))
HEADER_LIST =$(S_PREFIX)*.h

all: main

run: main
	@./$(B_PREFIX)main

main: $(OBJECTS)
	@mkdir -p $(B_PREFIX)
	@gcc $(ded_flags) $^ -o $(B_PREFIX)$@

$(O_PREFIX)%.o: $(S_PREFIX)%.cpp $(HEADER_LIST)
	@mkdir -p $(O_PREFIX)
	@gcc $(ded_flags) $< -c -o $@

clean:
	rm -rf $(O_PREFIX)*.o $(B_PREFIX)*.exe $(T_PREFIX)logs.txt
