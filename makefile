
BOARD_BASE = 25

main: main.c
	gcc -o main main.c
	./main $(BOARD_BASE)

clean:
	rm main