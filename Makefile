Assignment104: Assignment104.c
	gcc Assignment104.c -lcurses -o Assignment104

debug: Assignment104.c
	gcc -o Assignment104 -lcurses -ggdb3 Assignment104.c

clean: 
	rm -f Assignment104

test1:
	gcc test.c -lcurses -o test
