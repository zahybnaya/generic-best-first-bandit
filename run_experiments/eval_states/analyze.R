

data=read.csv(file='res.csv',sep=',')

data$ucterror<-abs(data$UCT_Move_Val-data$GS_Move_Val)
data$cdperror<-abs(data$CDP_Move_Val-data$GS_Move_Val)
data$mmerror<-abs(data$MM_Move_Val-data$GS_Move_Val)

print(paste("UCT_ERROR:",mean(data$ucterror)))
print(paste("CDP_ERROR:",mean(data$cdperror)))
print(paste("MM_ERROR:",mean(data$mmerror)))
