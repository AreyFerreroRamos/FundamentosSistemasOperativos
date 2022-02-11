

tron0 : tron0.c winsuport.o winsuport.h
	gcc -Wall tron0.c winsuport.o -o tron0 -lcurses

tron1 : tron1.c winsuport.o winsuport.h
	gcc -Wall tron1.c winsuport.o -o tron1 -lcurses -lpthread

tron2 : tron2.c winsuport.o winsuport.h
	gcc -Wall tron2.c winsuport.o -o tron2 -lcurses -lpthread

tron3 : tron3.c winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -Wall tron3.c winsuport2.o memoria.o -o tron3 -lcurses -lpthread

oponent3 : oponent3.c winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -Wall oponent3.c winsuport2.o memoria.o -o oponent3 -lcurses

tron4 : tron4.c winsuport2.o winsuport.h memoria.o memoria.h semafor.o semafor.h
	gcc -Wall tron4.c winsuport2.o memoria.o semafor.o -o tron4 -lcurses -lpthread

oponent4 : oponent4.c winsuport2.o winsuport2.h memoria.o memoria.h semafor.o semafor.h
	gcc -Wall oponent4.c winsuport2.o memoria.o semafor.o -o oponent4 -lcurses

tron5 : tron5.c winsuport2.o winsuport.h memoria.o memoria.h semafor.o semafor.h missatge.o missatge.h
	gcc -Wall tron5.c winsuport2.o memoria.o semafor.o missatge.o -o tron5 -lcurses -lpthread

oponent5 : oponent5.c winsuport2.o winsuport2.h memoria.o memoria.h semafor.o semafor.h missatge.o missatge.h
	gcc -Wall oponent5.c winsuport2.o memoria.o semafor.o missatge.o -o oponent5 -lcurses -lpthread

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

winsuport2.o : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

memoria.o : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o

semafor.o : semafor.c semafor.h
	gcc -c -Wall semafor.c -o semafor.o

missatge.o : missatge.c missatge.h
	gcc -c -Wall missatge.c -o missatge.o 

clean : 
	rm winsuport.o winsuport2.o memoria.o tron0 tron1 tron2 tron3 oponent3 tron4 oponent4 tron5 oponent5

