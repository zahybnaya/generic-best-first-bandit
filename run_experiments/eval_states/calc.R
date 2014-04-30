#R script for generiting data
# make sure there are no repeating head lines in states.csv

d=read.csv('states.csv')

splitByH=split(d,d$H)
for(h in names(splitByH)){
	print(h)
	d<-splitByH[[h]]
	splitByC=split(d,d$CI_Threshold)
	for(ci in names(splitByC)){
		d<-splitByC[[ci]]
		cat("\nCI_Threshold:",ci,"\n")
		splitByIt=split(d,d$iterations)
		for(it in names(splitByIt)){
			d<-splitByIt[[it]]
			cat("\t#Iterations:",it,"\n")
			cat("\tMMF:",nrow(d[d$GS_Move==d$MMF_Move,])/nrow(d),"\n")
			cat("\tCI:",nrow(d[d$GS_Move==d$CI_Move,])/nrow(d),"\n")
			cat("\tUCT:",nrow(d[d$GS_Move==d$UCT_Move,])/nrow(d),"\n")
			cat("\tDifference from UCT:",nrow(d[d$GS_Move!=d$UCT_Move & d$CI_Move==d$GS_Move ,]),"\n")
			cat("\tDifference from MMF:",nrow(d[d$GS_Move!=d$MMF_Move & d$CI_Move==d$GS_Move ,]),"\n")
		}
	}
}
