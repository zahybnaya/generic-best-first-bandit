CFLAGS = -lm -Wall -g 

ggames: common.h domain genericGames.c uct.c type_reachability.c type_reachability.h  minmax.c heuristic.c move.c board.c util.c random.c bfb.c brue.c type.h type.c vts.c sts.c 
	gcc genericGames.c uct.c bfb.c brue.c type.c type_reachability.c vts.c sts.c util.c value_iteration.c domain.o -o ggames $(CFLAGS) 

domain: synth mancala zop c4 sailing domain.c domain.h
	gcc -c domain.c -o tmpdomain.o $(CFLAGS) 
	ld -r tmpdomain.o synth.o mancala.o zop.o c4.o sailing.o -o domain.o 
	rm tmpdomain.o	

synth: synth.c synth.h
	gcc -c synth.c -o synth.o $(CFLAGS)

zop: zop.c zop.h
	gcc -c zop.c -o zop.o ${CFLAGS}
	
c4: c4.c c4.h
	gcc -c c4.c -o c4.o ${CFLAGS}

sailing: sailing.c sailing.h
	gcc -c sailing.c -o sailing.o ${CFLAGS}

mancala: mancala.c mancala.h move.c board.c heuristic.c 
	gcc -c mancala.c -o tmpmancala.o $(CFLAGS) 
	gcc -c move.c -o tmpmove.o $(CFLAGS) 
	gcc -c board.c -o tmpboard.o $(CFLAGS) 
	gcc -c heuristic.c -o tmpheuristic.o $(CFLAGS) 
	gcc -c random.c -o tmprandom.o $(CFLAGS) 
	gcc -c minmax.c -o tmpmm.o $(CFLAGS) 
	ld -r tmpmancala.o tmpboard.o tmpmove.o  tmpheuristic.o tmprandom.o tmpmm.o -o mancala.o  
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
