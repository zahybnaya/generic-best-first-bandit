library('grid')
library('ggplot2')
data=read.csv('seg_mancala_data.csv',sep=",")

# Display for seg=1 and against MC 
v=data[data$seg==1 & data$a2 == "MC",]
vmcdp=data[data$seg==1 & data$a1 == "MC" & data$a2 == "DP",]
if(nrow(vmcdp)>0){
	vmcdp$a1<-"DP"
	vmcdp$a2<-"MC"
	tmp=vmcdp$max_win
	vmcdp$max_win <- vmcdp$min_win
	vmcdp$min_win <- tmp
	v<-rbind(v,vmcdp)
}
print("seg1 length:")
print(nrow(v))
pdf('mc_1.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype=factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=1") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2))
      + scale_shape_manual(values=c(6,5,1))
      )
dev.off()

# Display for seg=4 and against MC 
v=data[data$seg==4 & data$a2 == "MC",]
vmcdp=data[data$seg==4 & data$a1 == "MC" & data$a2 == "DP",]
if(nrow(vmcdp)>0){
	vmcdp$a1<-"DP"
	vmcdp$a2<-"MC"
	tmp=vmcdp$max_win
	vmcdp$max_win <- vmcdp$min_win
	vmcdp$min_win <- tmp
	v<-rbind(v,vmcdp)
}
print("seg4 length")
print(nrow(v))
pdf('mc_4.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=4") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()

# Display for seg=8 and against MC 
v=data[data$seg==8 & data$a2 == "MC",]
vmcdp=data[data$seg==8 & data$a1 == "MC" & data$a2 == "DP",]
if(nrow(vmcdp)>0){
	vmcdp$a1<-"DP"
	vmcdp$a2<-"MC"
	tmp=vmcdp$max_win
	vmcdp$max_win <- vmcdp$min_win
	vmcdp$min_win <- tmp
	v<-rbind(v,vmcdp)
}
print("seg8 length")
print(nrow(v))
pdf('mc_8.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=8") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=8 and against MC 
v=data[data$seg==16 & data$a2 == "MC",]
vmcdp=data[data$seg==16 & data$a1 == "MC" & data$a2 == "DP",]
if(nrow(vmcdp)>0){
	vmcdp$a1<-"DP"
	vmcdp$a2<-"MC"
	tmp=vmcdp$max_win
	vmcdp$max_win <- vmcdp$min_win
	vmcdp$min_win <- tmp
	v<-rbind(v,vmcdp)
}
print("seg16 length")
print(nrow(v))
pdf('mc_16.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=16") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()

# Display for seg=8 and against MC 
v=data[data$seg==24 & data$a2 == "MC",]
vmcdp=data[data$seg==24 & data$a1 == "MC" & data$a2 == "DP",]
if(nrow(vmcdp)>0){
	vmcdp$a1<-"DP"
	vmcdp$a2<-"MC"
	tmp=vmcdp$max_win
	vmcdp$max_win <- vmcdp$min_win
	vmcdp$min_win <- tmp
	v<-rbind(v,vmcdp)
}
print("seg24 length")
print(nrow(v))
pdf('mc_24.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=24") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()




# Display for seg=5 and against MC 
v=data[data$seg==5 & data$a2 == "MC",]
print("seg5 length:")
print(nrow(v))
pdf('mc_5.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=5") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=10 and against MC 
v=data[data$seg==10 & data$a2 == "MC",]
print("seg10 length:")
print(nrow(v))
pdf('mc_10.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=1,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=10") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


# Display for seg=20 and against MC 
v=data[data$seg==20 & data$a2 == "MC",]
print(nrow(v))
pdf('mc_20.pdf')
print(ggplot(v,aes(x=v$iterations, y=v$max_win, group=factor(v$a1))) + geom_line(size=2,aes(linetype = factor(v$a1))) + geom_point(size=4, shape=factor(v$a1)) +  ylab(" % win rate") + ylim(0,0.9)+ xlab("#iterations (thousands)") + scale_linetype_discrete(name="Seg=20") + theme_bw(base_size = 32, base_family = "Helvetica")+ theme(legend.position = c(0.15,0.85), legend.key.size= unit(1.9,"lines")) + geom_hline(yintercept=0.5, linecolor=5, linewidth=1) + scale_x_continuous(labels= function(x) format(x/1000,digits = 2)))
dev.off()


