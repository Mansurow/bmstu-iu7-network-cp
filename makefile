EXTENSION := out

CFLAGS := -Wall -pedantic -Wvla -Wfloat-conversion -Wfloat-equal -Iinc -g3 -Wextra

OUT := out/server.o

TEST := out/test.o out/threadpool.o

ifeq ($(strip $(wildcard out/*.$(EXTENSION)) $(wildcard out/*.o)),)
	CLEAN := @echo Nothing to be cleaned
else
	CLEAN := rm $(wildcard out/*.$(EXTENSION)) $(wildcard out/*.o)
endif

app : out/app.out

out/app.$(EXTENSION) : $(OUT)
	gcc $^ -o $@

out/test.$(EXTENSION) : $(TEST)
	gcc $^ -o $@ -lpthread

out/%.o : src/%.c inc/*.h
	gcc $(CFLAGS) $< -c -o $@

.PHONY: clean, app

clean :
	$(CLEAN)