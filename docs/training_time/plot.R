## Plot VISE training time
##
##' @author Abhishek Dutta <adutta@robots.ox.ac.uk>
##' @version May. 12, 2017

rm( list=ls() )
graphics.off()


library('ggplot2') # for creating plots
library('grid')			# for unit
library('reshape') ## for melt()
library('plyr') ## for ddply

source('f_multiplot.R')

fn <- '/home/tlm/dev/vise/docs/training_time/training_stat.csv'
d <- read.csv(fn)

ds <- subset(d, select= -time_sec)
dt <- subset(d, select= -space_bytes)

names(ds)[names(ds) == 'space_bytes'] <- 'space_mb'
names(dt)[names(dt) == 'time_sec'] <- 'time_min'

ds$space_mb <- ds$space_mb / (1024*1024)
dt$time_min <- dt$time_min / (60)

#d$dataset_name <- factor(d$dataset_name, levels=c("ox5k_100", "ox5k_300", "ox5k_700", "ox5k_1100"), labels=c("100", "300", "700", "1100"))

time_summary <- ddply(dt, .(dataset_name), summarise, date=date[1], time=time[1], dataset_name=dataset_name[1], img_count=img_count[1], state_name="TOTAL", time_min=sum(time_min) )
space_summary <- ddply(ds, .(dataset_name), summarise, date=date[1], time=time[1], dataset_name=dataset_name[1], img_count=img_count[1], state_name="TOTAL", space_mb=sum(space_mb) )

ds <- rbind(ds, space_summary)
dt <- rbind(dt, time_summary)

## Create a plot
p0 <- ggplot( data=dt, aes( x=img_count, y=time_min, color=state_name) )
p0 <- p0 + geom_point() + geom_smooth(method=lm) #+ geom_line( aes(group=state_name) )
p0 <- p0 + scale_x_continuous( name='Number of images in training dataset', breaks=unique(d$img_count) )
p0 <- p0 + scale_y_continuous( name='Time (in min.)' )
#p0 <- p0 + facet_wrap( ~state_name, scales='fixed', labeller=label_both )
#p0 <- p0 + theme_bw() + theme( legend.position='top' )
#p0 <- p0 + theme(plot.margin = unit(c(0.4,0.4,0.4,0.8), "lines")) + theme(axis.title.x = element_text(size = 12, vjust = -0.5)) + theme(axis.title.y = element_text(size = 12, angle = 90, vjust = 2.0)) + theme(axis.text.x = element_text(size = 10)) + theme(axis.text.y = element_text(size = 10)) + theme(legend.key = element_blank(), legend.text = element_text(size = 10)) + theme(panel.margin=unit(0.1, "in")) + theme(panel.grid.minor=element_blank(), panel.grid.major=element_blank())
#p0 <- p0 + geom_hline( aes(yintercept=0.29580399), linetype=3, color='black' ) ## RMSE when the error is +/- 0.2
#p0 <- p0 + scale_x_continuous( name='Training set size (per cent)' ) + scale_y_continuous( name='Root Mean Square Error (RMSE)' )
#cairo_pdf( filename=plot_fn, width=12, height=12, onefile=TRUE )           
#dev.off()

## Disk space used during training 
p1 <- ggplot( data=ds, aes( x=img_count, y=space_mb, color=state_name) )
p1 <- p1 + geom_point() + geom_smooth(method=lm) # + geom_line( aes(group=state_name) )
p1 <- p1 + scale_x_continuous( name='Number of images in training dataset', breaks=unique(d$img_count) )
p1 <- p1 + scale_y_continuous( name='Used disk space (in MB)' )

cairo_pdf( filename='training_time_space_model.pdf', width=6, height=9, onefile=TRUE )  
f_multiplot(p0, p1, cols=1)
dev.off()

mdt <- dlply( dt, .(state_name), lm, formula = time_min~img_count )
cdt = ldply(mdt, coef)
print("Model coefficients for time")
print(cdt)

mds <- dlply( ds, .(state_name), lm, formula = space_mb~img_count )
cds = ldply(mds, coef)
print("Model coefficients for space")
print(cds)
