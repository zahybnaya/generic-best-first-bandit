
library('grid')
library('ggplot2')
data=read.csv('seg_mancala_data.csv',sep="\t")

# Display for seg=1 and against MC 
v=data[data$seg==1 & data$a2 == 1,]
pdf('mc_1.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=2,aes(linetype = factor(v$a1))) + geom_point(size=2) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=1") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=5 and against MC 
v=data[data$seg==5 & data$a2 == 1,]
pdf('mc_5.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=2,aes(linetype = factor(v$a1))) + geom_point(size=2) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=5") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=10 and against MC 
v=data[data$seg==10 & data$a2 == 1,]
pdf('mc_10.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=2,aes(linetype = factor(v$a1))) + geom_point(size=2) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=10") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=20 and against MC 
v=data[data$seg==20 & data$a2 == 1,]
pdf('mc_20.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=2,aes(linetype = factor(v$a1))) + geom_point(size=2) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=20") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


