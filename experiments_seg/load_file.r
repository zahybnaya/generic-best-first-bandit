
library('ggplot2')
data=read.csv('seg_mancala_data.csv')

# Display for seg=1 and against MC 
v=data[data$seg==1 & data$a2 == 1,]
pdf('mc_1')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(aes(linetype = factor(v$a1))) + geom_point(size=2) + ggtitle("segmentation=1") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="Backup:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top") +geom_hline(yintercept=0.5))
dev.off()


# Display for seg=5 and against MC 
v=data[data$seg==5 & data$a2 == 1,]
pdf('mc_5')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(aes(linetype = factor(v$a1))) + geom_point(size=2) + ggtitle("segmentation=5") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="Backup:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top")+geom_hline(yintercept=0.5))
dev.off()


# Display for seg=10 and against MC 
v=data[data$seg==10 & data$a2 == 1,]
pdf('mc_10')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(aes(linetype = factor(v$a1))) + geom_point(size=2) + ggtitle("segmentation=10") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="Backup:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top")+geom_hline(yintercept=0.5))
dev.off()


# Display for seg=20 and against MC 
v=data[data$seg==20 & data$a2 == 1,]
pdf('mc_20')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(aes(linetype = factor(v$a1))) + geom_point(size=2) + ggtitle("segmentation=20") + ylab(" % win rate") + xlab("#iterations") + scale_linetype_discrete(name="Backup:") + theme_bw()+ theme(legend.position="top") + theme(legend.position="top")+geom_hline(yintercept=0.5))
dev.off()


