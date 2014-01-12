
library('ggplot2')
data=read.csv('mancala.csv')
v=data[data$c==2.5 & data$policy == 2 & data$heuristics == 3 & data$min_alg == 'UCT' & data$pits == 6,]

pdf('mancala6sts1000.pdf')
ggplot(v,aes(x=v[v$sts_size=='1000',]$iterations, y=v[v$sts_size=='1000',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(1000)") + ylab(" % win rate") + xlab("#iterations")
dev.off()


pdf('mancala6sts100.pdf')
ggplot(v,aes(x=v[v$sts_size=='100',]$iterations, y=v[v$sts_size=='100',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(100)") + ylab(" % win rate") + xlab("#iterations")
dev.off()


pdf('mancala6sts500.pdf');
ggplot(v,aes(x=v[v$sts_size=='500',]$iterations, y=v[v$sts_size=='500',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(500)") + ylab(" % win rate") + xlab("#iterations");
dev.off();


pdf('mancala6sts700.pdf');
ggplot(v,aes(x=v[v$sts_size=='700',]$iterations, y=v[v$sts_size=='700',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(700)") + ylab(" % win rate") + xlab("#iterations");
dev.off();

# Minmax comparision
nomm=data[data$c==2.5 & data$policy == '2' & data$min_alg == 'MINMAX' & data$max_alg == 'BFB' & data$pits == 6,]
mm=data[data$c==2.5  & data$min_alg == 'MINMAX' & data$max_alg == 'UCT' & data$pits == 6,]
v=rbind(mm,nomm)
v[is.na(v)] <- 'UCT'


pdf('mancala6MMCompare_h1.pdf');
v1=v[v$heuristics=='1',]
ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=1") + ylab(" % win rate") + xlab("#iterations");
dev.off();


pdf('mancala6MMCompare_h2.pdf');
v1=v[v$heuristics=='2',]
ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=2") + ylab(" % win rate") + xlab("#iterations");
dev.off();


pdf('mancala6MMCompare_h3.pdf');
v1=v[v$heuristics=='3',]
ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=3") + ylab(" % win rate") + xlab("#iterations");
dev.off();


#pdf('mancala7MMCompare_h4.pdf');
#v1=v[v$heuristics=='4',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=4") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#


pdf('mancala6MMCompare_h5.pdf');
v1=v[v$heuristics=='5',]
ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=5") + ylab(" % win rate") + xlab("#iterations");
dev.off();



pdf('mancala6MMCompare_h6.pdf');
v1=v[v$heuristics=='6',]
ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=6") + ylab(" % win rate") + xlab("#iterations");
dev.off();









