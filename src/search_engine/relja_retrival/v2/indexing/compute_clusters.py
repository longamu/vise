#
# ==== Author:
# 
# Relja Arandjelovic (relja@robots.ox.ac.uk)
# Visual Geometry Group,
# Department of Engineering Science
# University of Oxford
#
# ==== Copyright:
#
# The library belongs to Relja Arandjelovic and the University of Oxford.
# No usage or redistribution is allowed without explicit permission.
#

import os, sys;
import ConfigParser;

import pypar as mpi;
from dkmeans_relja.dkmeans import compute_clusters;


def getOptional( f, defaultValue ):
    
    try:
        value= f();
    except ConfigParser.NoOptionError:
        value= defaultValue;
    return value;



if __name__=='__main__':
    
    dsetname= "oxMini20_v2";
    if len(sys.argv)>1: dsetname= sys.argv[1];
    configFn= "../src/ui/web/config/config.cfg";
    if len(sys.argv)>2: configFn= sys.argv[2];
    
    config= ConfigParser.ConfigParser();
    config.read( configFn );
    
    RootSIFT= getOptional( lambda: config.getboolean(dsetname, 'RootSIFT'), True );

    clstFn= os.path.expanduser( config.get(dsetname, 'clstFn') );
    trainFilesPrefix= os.path.expanduser( config.get(dsetname, 'trainFilesPrefix') );
    pntsFn= trainFilesPrefix + "descs.e3bin";
    
    vocSize= getOptional( lambda: config.getint(dsetname, 'vocSize'), 100 );
    clusterNumIteration = getOptional( lambda: config.getint(dsetname, 'clusterNumIteration'), 30 );
    seed= 43;
  
    compute_clusters(clstFn, pntsFn, vocSize,
                     clusterNumIteration, approx=True, seed= seed,
                     featureWrapper= ("hell" if RootSIFT else None) );
    
    mpi.finalize();
