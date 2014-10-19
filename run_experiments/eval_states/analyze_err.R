

data=read.csv(file='../../results/evalstates/states.csv',sep=',',stringsAsFactors=FALSE)
data$ucterror<-abs(data$UCT_value-data$GS_Move_Val)
data$cdperror<-abs(data$CDP_value-data$GS_Move_Val)
data$mmerror<-abs(data$MM_value-data$GS_Move_Val)

splitByIt=split(data,data$Iterations)
		for(it in names(splitByIt)){
			cat("\t#Iterations:",it,"\n")
			d<-splitByIt[[it]]
			print(paste("UCT_ERROR:",mean(d$ucterror)))
			print(paste("CDP_ERROR:",mean(d$cdperror)))
			print(paste("MM_ERROR:",mean(d$mmerror)))
		}

