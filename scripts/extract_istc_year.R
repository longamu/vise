library(stringr)

d <- read.csv('/home/tlm/mydata/15cbt/training_data/istc.csv')

r <- regexpr( '[[:digit:]]{4}', d$imprint )
d$imprint_year <- rep(-1, length(d$imprint))
d$imprint_year[ r != -1 ] <- regmatches(d$imprint, r)
d$imprint_year <- as.numeric(d$imprint_year)
write.csv(d, file='/home/tlm/mydata/15cbt/training_data/istc_with_imprint_year.csv', quote=TRUE, sep=',', row.names=FALSE, col.names=TRUE, na="")

## misc commands
#grep('[[:digit:]]{4}', d$imprint, value=TRUE)
#grep('[[:digit:]]{4}', d$imprint)
#d$imprint[ !grepl('[[:digit:]]{4}', d$imprint) ] # shows all entries with invalid year (total 315)
#length(d$imprint[ grepl('[[:digit:]]{4}', d$imprint) ]) # 30213 entries with valid 4 digit date
