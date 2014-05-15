
library('ggplot2')
data=read.csv('./mancala_backop.csv')
v=data[data$c==2.5 & (data$backop == 'DP' | data$backop == 'CDP') & data$heuristics == 3 & data$min_alg == 'UCT',]


pdf('mancala_h3_againstMC.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$backop))) + geom_line(aes(linetype = factor(v$backop))) + geom_point(size=2) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations") + scale_linetype_discrete(name="Win rate for Backup operator:") + theme_bw(base_size = 33, base_family = "Helvetica")+ theme(legend.position = c(0.5,0.9)) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1))
dev.off()

#v=data[data$c==2.5 & (data$backop == 'MC' | data$backop == 'CDP') & data$heuristics == 3 & data$min_alg == 'UCT',]
pdf('mancala_h3_againstDP.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$backop))) + geom_line(aes(linetype = factor(v$backop))) + geom_point(size=2) + ylim(0,0.9)+ xlab("#iterations") + scale_linetype_discrete(name="Win rate for Backup operator:") + theme_bw(base_size = 33, base_family = "Helvetica")+ theme(legend.position = c(0.4,0.9))  +theme(axis.text.y=element_blank(), axis.title.y=element_blank(),axis.ticks=element_blank())+ geom_hline(yintercept=0.5, linecolor=5, linewidth=1))
dev.off()

