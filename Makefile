CFLAGS = -lm -Wall -g -fPIC

ggames: common.h domain genericGames.c uct.c minmax.c heuristic.c move.c board.c util.c random.c 
	gcc genericGames.c minmax.c uct.c util.c domain.o -o ggames $(CFLAGS) -shared

domain: synth mancala domain.c domain.h
	gcc -c domain.c -o tmpdomain.o $(CFLAGS) -fPIC
	ld tmpdomain.o synth.o mancala.o -o domain.o -lm -shared -fPIC
	rm tmpdomain.o	

synth: synth.c synth.h
	gcc -c synth.c -o synth.o $(CFLAGS)

mancala: mancala.c mancala.h move.c board.c
	gcc -c mancala.c -o tmpmancala.o $(CFLAGS) -fPIC
	gcc -c move.c -o tmpmove.o $(CFLAGS) -fPIC
	gcc -c board.c -o tmpboard.o $(CFLAGS) -fPIC
	gcc -c heuristic.c -o tmpheuristic.o $(CFLAGS) -fPIC
	gcc -c random.c -o tmprandom.o $(CFLAGS) -fPIC
	gcc -c minmax.c -o tmpmm.o $(CFLAGS) -fPIC
	ld tmpboard.o tmpmove.o  tmpheuristic.o tmprandom.o tmpmm.o -o mancala.o -lm -shared -fPIC
	rm tmpmm.o tmpmove.o tmpboard.o tmprandom.o tmpmancala.o tmpheuristic.o

chess: chess.cpp chess.h normalNoise.c normalNoiseH.h domain.h mmuct.c common.h minmax.c mmuct.c heuristic.c move.c board.c util.c random.c mmuct_test.c uct.c domain.c synth.h synth.c mancala.h mmuct.h
	g++ chess.cpp /media/data/Research/mmuct/gnuchess/gnuchess-6.0.1/src/engine/*.c* -o chess $(CFLAGS)


generalTests: generalTests.c
	gcc generalTests.c -o generalTests -Wall -g

testSynth: synth.h synth.c synthTestCase.c
	gcc synth.c mmuct.c domain.c normalNoise.c heuristic.c board.c move.c random.c minmax.c mancala.c synthTestCase.c -o ts -Wall -lm -g

normalNoise: normalNoiseTestCase.c
	gcc normalNoiseTestCase.c heuristic.c board.c move.c mancala.c random.c minmax.c mmuct.c synth.c normalNoise.c domain.c -o normalNoise $(CFLAGS)

clean:
	rm *.o
