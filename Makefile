
all:
	make genboards
	make hybrid
	make getest
	make games
	make gentree
	make bestmoves



ggames: common.h domain.h mancala.h domain.c mancala.c genericGames.c uct.c minmax.c heuristic.c move.c board.c util.c random.c 
	gcc genericGames.c minmax.c uct.c heuristic.c move.c board.c util.c random.c domain.c mancala.c -o ggames -lm -Wall -g

games: common.h domain.h mancala.h domain.c mancala.c games.c uct.c minmax.c heuristic.c move.c board.c util.c random.c 
	gcc games.c minmax.c uct.c heuristic.c move.c board.c util.c random.c domain.c mancala.c -o games -lm -Wall -g

generalTests: generalTests.c
	gcc generalTests.c -o generalTests -Wall -g

testSynth: synth.h synth.c synthTestCase.c
	gcc synth.c mmuct.c domain.c normalNoise.c heuristic.c board.c move.c random.c minmax.c mancala.c synthTestCase.c -o ts -Wall -lm -g

normalNoise: normalNoiseTestCase.c
	gcc normalNoiseTestCase.c heuristic.c board.c move.c mancala.c random.c minmax.c mmuct.c synth.c normalNoise.c domain.c -o normalNoise -lm -Wall -g

synth: mancala.c mancala.h
	echo "TODO"

mancala: mancala.c mancala.h
	echo "TODO"

chess: chess.cpp chess.h normalNoise.c normalNoiseH.h domain.h mmuct.c common.h minmax.c mmuct.c heuristic.c move.c board.c util.c random.c mmuct_test.c uct.c domain.c synth.h synth.c mancala.h mmuct.h
	g++ chess.cpp /media/data/Research/mmuct/gnuchess/gnuchess-6.0.1/src/engine/*.c* -o chess -lm -Wall -g -fpermissive


