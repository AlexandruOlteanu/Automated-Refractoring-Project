build: automated.o

automated.o: automated.cpp
	g++ automated.cpp -o automated_run -Wall -Wextra

run: 
	./automated_run

install: 
	./install.sh

uninstall:
	./uninstall.sh

.PHONY: clean

clean:
	rm -rf *.o automated_run