objects = bin/main.o
binout = bin/ffiks
flags = -ggdb -O3 -lcurl
comp = gcc
$(binout) : $(objects)
	$(comp) $(objects) $(flags) -o $(binout)

bin/main.o : src/main.c src/json.h src/http.h
	$(comp) src/main.c $(flags) -c -o bin/main.o

bin/test : src/test.c
	$(comp) src/test.c $(flags) -o bin/test

install : $(binout)
	cp $(binout) ~/.local/bin/
