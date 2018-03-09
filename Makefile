test: send1.o recv1.o send2.o recv2.o send3.o recv3.o
	g++ send1.o -o sender1
	g++ recv1.o -o receiver1
	g++ send2.o -o sender2
	g++ recv2.o -o receiver2
	g++ send3.o -o sender3
	g++ recv3.o -o receiver3
send1.o:
	g++ send1.cpp -c
recv1.o:
	g++ recv1.cpp -c
send2.o:
	g++ send2.cpp -c
recv2.o:
	g++ recv2.cpp -c
send3.o:
	g++ send3.cpp -c
recv3.o:
	g++ recv3.cpp -c
clean:
	rm -rf send1.o recv1.o sender1 receiver1 send2.o recv2.o sender2 receiver2 send3.o recv3.o sender3 receiver3
