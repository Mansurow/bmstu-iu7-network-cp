EXTENSION := out

CFLAGS := -Wall -pedantic -Wvla -Wfloat-conversion -Wfloat-equal -Iinc -g3 -Wextra

OUT := out/server.o out/http.o out/thread_pool.o

TEST := out/test.o out/thread_pool.o

ifeq ($(strip $(wildcard out/*.$(EXTENSION)) $(wildcard out/*.o)),)
	CLEAN := @echo Nothing to be cleaned
else
	CLEAN := rm $(wildcard out/*.$(EXTENSION)) $(wildcard out/*.o)
endif

app : out/app.out

test : out/test.out

out/app.$(EXTENSION) : $(OUT)
	gcc $^ -o $@

out/test.$(EXTENSION) : $(TEST)
	gcc $^ -o $@ -lpthread

out/%.o : src/%.c inc/*.h
	gcc $(CFLAGS) $< -c -o $@

.PHONY: clean, app

clean :
	$(CLEAN)