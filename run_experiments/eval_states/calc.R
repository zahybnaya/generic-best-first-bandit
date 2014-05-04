#R script for generiting data
# make sure there are no repeating head lines in states.csv

d=read.csv('states_ci_build_1000games.csv', stringsAsFactors=FALSE)

splitByH=split(d,d$H)
for(h in names(splitByH)){
	print(h)
	d<-splitByH[[h]]
	splitByC=split(d,d$CI_Threshold)
	for(ci in names(splitByC)){
		d<-splitByC[[ci]]
		cat("\nCI_Threshold:",ci,"\n")
		splitByIt=split(d,d$Iterations)
		for(it in names(splitByIt)){
			d<-splitByIt[[it]]
			cat("\t#Iterations:",it,"\n")
			cat("\tMMF:",nrow(d[d$GS_Move==d$MMF_Move,])/nrow(d),"\n")
			cat("\tCI:",nrow(d[d$GS_Move==d$CI_Move,])/nrow(d),"\n")
			cat("\tMMB:",nrow(d[d$GS_Move==d$MMB_Move,])/nrow(d),"\n")
			cat("\tCIB:",nrow(d[d$GS_Move==d$CIB_Move,])/nrow(d),"\n")
			cat("\tUCT:",nrow(d[d$GS_Move==d$UCT_Move,])/nrow(d),"\n")
			#cat("\tDifference from UCT:",nrow(d[d$GS_Move!=d$UCT_Move & d$CIB_Move==d$GS_Move ,]),"\n")
			#cat("\tDifference from MMB:",nrow(d[d$GS_Move!=d$MMB_Move & d$CIB_Move==d$GS_Move ,]),"\n")
			cat("\tDifference from UCT:",nrow(d[d$CIB_Move!=d$UCT_Move ,]),"\n")
			cat("\tDifference from MMB:",nrow(d[d$CIB_Move!=d$MMB_Move ,]),"\n")
		}
	}
}

