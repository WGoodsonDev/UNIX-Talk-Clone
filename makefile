server:
	g++ -o server server.cpp getChar.cpp -g -lncurses

clean:
	rm server
	g++ -o server server.cpp getChar.cpp -g -lncurses