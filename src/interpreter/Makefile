all: tinyc

tinyc: ast.o tinyc_parser.o tinyc_lexer.o main.o
	g++ -g -o $@ $^

ast.o: ast.cpp ast.h
	g++ -g -c -o $@ $<
	
main.o: main.cpp ast.h
	g++ -g -c -o $@ $<

tinyc_lexer.o: tinyc_lexer.cpp
	g++ -g -c -o $@ $<

tinyc_lexer.cpp: tinyc.l ast.h
	flex -o $@ $<

tinyc_parser.cpp: tinyc.y ast.h
	bison --defines=tokens.h -o $@ $<

tinyc_parse.o: tinyc_parser.cpp
	g++ -g -c -o $@ $<

clean:
	rm -f *.o
	rm -f tinyc_lexer.cpp
	rm -f tinyc_parser.cpp
	rm -f tokens.h
	rm -f tinyc
