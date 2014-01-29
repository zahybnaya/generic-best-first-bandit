
library('ggplot2')
data=read.csv('SR.csv')
v=data[data$c==2.5 & data$type_system==2 & data$c==2.5 & data$policy == 3 & data$heuristics == 3 & data$min_alg == 'UCT+SR' & data$pits == 6 & data$back_propagation == 1,]


pdf('mancala6stsAVG.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$sts_size))) + geom_line(aes(linetype = factor(v$sts_size))) + geom_point(size=2) + ggtitle("Mancala TSMCTS with TSST") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="STTS Threshold:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top"))
dev.off()


v=data[data$c==2.5 & data$policy == 2 & data$heuristics == 3 & data$min_alg == 'UCT' & data$pits == 6 & data$back_propagation == 2,]
pdf('mancala6stsMM.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$sts_size))) + geom_line(aes(linetype = factor(v$sts_size))) + geom_point(size=2) + ggtitle("Mancala TSMCTS with TSST") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="STTS Threshold:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top"))
dev.off()

#
#pdf('mancala6sts1000.pdf')
#ggplot(v,aes(x=v[v$sts_size=='1000',]$iterations, y=v[v$sts_size=='1000',]$max_win)) + geom_line(aes(size = 4)) + geom_point(size=4) + ggtitle("Size based TS(1000)") + ylab(" % win rate") + xlab("#iterations")
#dev.off()
#
#v=v[(v$sts_size=='1000' & v$iterations=='10000') | (v$sts_size=='100' & v$iterations=='1000') | (v$sts_size=='500' & v$iterations=='5000')] 
#ggplot(v,aes(x=v$iterations, y=v$max_win)) + geom_line(aes(size = 4)) + geom_point(size=4) + ggtitle("Size based TS(1000)") + ylab(" % win rate") + xlab("#iterations")
#
#
#
#
#pdf('mancala6sts100.pdf')
#ggplot(v,aes(x=v[v$sts_size=='100',]$iterations, y=v[v$sts_size=='100',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(100)") + ylab(" % win rate") + xlab("#iterations")
#dev.off()
#
#
#pdf('mancala6sts500.pdf');
#ggplot(v,aes(x=v[v$sts_size=='500',]$iterations, y=v[v$sts_size=='500',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(500)") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
#pdf('mancala6sts700.pdf');
#ggplot(v,aes(x=v[v$sts_size=='700',]$iterations, y=v[v$sts_size=='700',]$max_win)) + geom_line() + geom_point(size=4) + ggtitle("Size based TS(700)") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
# Minmax comparision
v=data[data$c==2.5 & (data$policy == 2 | is.na(data$policy)) & data$min_alg == 'MINMAX' & data$pits == 6 & data$back_propagation == 2,]
v[is.na(v)]<-'UCT'
v1=v[v$heuristics==1,]

v1i=v1[v1$iterations==1000,]
pdf('mancala6depthh11K.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==10000,]
pdf('mancala6depthh110K.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==100000,]
pdf('mancala6depthh1100K.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=2, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==5000,]
pdf('mancala6depthh15K.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==15000,]
pdf('mancala6depthh115K.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v2=v[v$heuristics==2,]

v2i=v2[v2$iterations==1000,]
pdf('mancala6depthh21K.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==10000,]
pdf('mancala6depthh210K.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==100000,]
pdf('mancala6depthh2100K.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=2, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==5000,]
pdf('mancala6depthh25K.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==15000,]
pdf('mancala6depthh215K.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v3=v[v$heuristics == 3,]

v3i=v3[v3$iterations==1000,]
pdf('mancala6depthh31K.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ theme(legend.position="top") +scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==10000,]
pdf('mancala6depthh310K.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==100000,]
pdf('mancala6depthh3100K.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==5000,]
pdf('mancala6depthh35K.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==15000,]
pdf('mancala6depthh315K.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v5=v[v$heuristics == 5,]

v5i=v5[v5$iterations==1000,]
pdf('mancala6depthh51K.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==10000,]
pdf('mancala6depthh510K.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==100000,]
pdf('mancala6depthh5100K.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==5000,]
pdf('mancala6depthh55K.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==15000,]
pdf('mancala6depthh515K.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v6=v[v$heuristics == 6,]

v6i=v6[v6$iterations==1000,]
pdf('mancala6depthh61K.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==10000,]
pdf('mancala6depthh610K.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==100000,]
pdf('mancala6depthh6100K.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==5000,]
pdf('mancala6depthh65K.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==15000,]
pdf('mancala6depthh615K.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


##### operator avg ###############

v=data[data$c==2.5 & (data$policy == 2 | is.na(data$policy)) & data$min_alg == 'MINMAX' & data$pits == 6 & data$back_propagation == 1,]
v[is.na(v)]<-'UCT'
v1=v[v$heuristics==1,]

v1i=v1[v1$iterations==1000,]
pdf('mancala6depthh11KAVG.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==10000,]
pdf('mancala6depthh110KAVG.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==100000,]
pdf('mancala6depthh1100KAVG.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=2, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==5000,]
pdf('mancala6depthh15KAVG.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v1i=v1[v1$iterations==15000,]
pdf('mancala6depthh115KAVG.pdf')
print(ggplot(v1i,aes(x=v1i$mm_level, y=v1i$max_win, size=3, group=factor(v1i$sts_size))) + geom_line(aes(color = factor(v1i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=1, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v2=v[v$heuristics==2,]

v2i=v2[v2$iterations==1000,]
pdf('mancala6depthh21KAVG.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==10000,]
pdf('mancala6depthh210KAVG.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==100000,]
pdf('mancala6depthh2100KAVG.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=2, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==5000,]
pdf('mancala6depthh25KAVG.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v2i=v2[v2$iterations==15000,]
pdf('mancala6depthh215KAVG.pdf')
print(ggplot(v2i,aes(x=v2i$mm_level, y=v2i$max_win, size=3, group=factor(v2i$sts_size))) + geom_line(aes(color = factor(v2i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=2, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v3=v[v$heuristics == 3,]

v3i=v3[v3$iterations==1000,]
pdf('mancala6depthh31KAVG.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ theme(legend.position="top") +scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==10000,]
pdf('mancala6depthh310KAVG.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==100000,]
pdf('mancala6depthh3100KAVG.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==5000,]
pdf('mancala6depthh35KAVG.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v3i=v3[v3$iterations==15000,]
pdf('mancala6depthh315KAVG.pdf')
print(ggplot(v3i,aes(x=v3i$mm_level, y=v3i$max_win, size=3, group=factor(v3i$sts_size))) + geom_line(aes(color = factor(v3i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=3, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v5=v[v$heuristics == 5,]

v5i=v5[v5$iterations==1000,]
pdf('mancala6depthh51KAVG.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==10000,]
pdf('mancala6depthh510KAVG.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==100000,]
pdf('mancala6depthh5100KAVG.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==5000,]
pdf('mancala6depthh55KAVG.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v5i=v5[v5$iterations==15000,]
pdf('mancala6depthh515KAVG.pdf')
print(ggplot(v5i,aes(x=v5i$mm_level, y=v5i$max_win, size=3, group=factor(v5i$sts_size))) + geom_line(aes(color = factor(v5i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=5, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()


v6=v[v$heuristics == 6,]

v6i=v6[v6$iterations==1000,]
pdf('mancala6depthh61KAVG.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 1K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==10000,]
pdf('mancala6depthh610KAVG.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 10K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==100000,]
pdf('mancala6depthh6100KAVG.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 100K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==5000,]
pdf('mancala6depthh65KAVG.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 5K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()

v6i=v6[v6$iterations==15000,]
pdf('mancala6depthh615KAVG.pdf')
print(ggplot(v6i,aes(x=v6i$mm_level, y=v6i$max_win, size=3, group=factor(v6i$sts_size))) + geom_line(aes(color = factor(v6i$sts_size))) + geom_point(size=2) + ggtitle("Win rate against MM with h=6, 15K iterations") + ylab(" % win rate") + xlab("depth")  + theme_bw()+ theme(legend.position="top")+ scale_color_discrete(name="Threshold"))
dev.off()



###################################

##nomm=data[data$c==2.5 & data$policy == '2' & data$min_alg == 'MINMAX' & data$max_alg == 'BFB' & data$pits == 6,]
#mm=data[data$c==2.5  & data$min_alg == 'MINMAX' & data$max_alg == 'UCT' & data$pits == 6,]
#v=rbind(mm,nomm)
#v[is.na(v)] <- 'UCT'
#
#
#pdf('mancala6MMCompare_h1.pdf');
#v1=v[v$heuristics=='1',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=1") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
#pdf('mancala6MMCompare_h2.pdf');
#v1=v[v$heuristics=='2',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=2") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
#pdf('mancala6MMCompare_h3.pdf');
#v1=v[v$heuristics=='3',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=3") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
##pdf('mancala7MMCompare_h4.pdf');
##v1=v[v$heuristics=='4',]
##ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=4") + ylab(" % win rate") + xlab("#iterations");
##dev.off();
##
#
#
#pdf('mancala6MMCompare_h5.pdf');
#v1=v[v$heuristics=='5',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=5") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
#
#pdf('mancala6MMCompare_h6.pdf');
#v1=v[v$heuristics=='6',]
#ggplot(v1,aes(x=v1$iterations, y=v1$max_win, group=as.factor(v1$sts_size))) + geom_line(aes(colour=as.factor(v1$sts_size), size=4)) + geom_point(size=4) + ggtitle("Win rate for minimax h=6") + ylab(" % win rate") + xlab("#iterations");
#dev.off();
#
#
#
#
#




