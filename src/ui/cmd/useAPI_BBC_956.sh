#!/bin/bash
#while nc $@; do echo "";
while true; do

	for ID in `seq 1 1 956`
	do
		
		upload="<processImage><imageFn>/data/Yujie/Databases/BBC_frames_956/1000_screenshgrabs/${ID}.jpg</imageFn><compDataFn>/data/Yujie/Databases/BBC_frames_956/tmp/${ID}.compdata</compDataFn></processImage>"
		end=' $END$'
		cmd="<externalQuery><wordFn>/data/Yujie/Databases/BBC_frames_956/tmp/${ID}.compdata</wordFn><startFrom>0</startFrom><numberToReturn>5</numberToReturn>
		<xl>0.00</xl><xu>1024.00</xu><yl>0.00</yl><yu>576.00</yu></externalQuery>"
		
		echo "${upload}$end" | nc $@
		echo "${cmd}$end" | nc $@
		
		
		
	done

break;

done
