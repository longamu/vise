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

import numpy as np;
import tables;
import sys, os;



def exportClusters( clst_fn, clst_fn_new ):
    
    # load data
    print 'Loading clusters';
    
    ci = tables.openFile(clst_fn, 'r');
    
    # Get the cluster node
    for node in ci.walkNodes('/', classname='Array'):
        break;
    
    clst_data= node.read();
    ci.close();
    numClust= clst_data.shape[0];
    nDims= clst_data.shape[1];
    
    print 'Saving clusters';
    
    f= open(clst_fn_new,'wb');
    a= np.array( [numClust, nDims], 'uint32' );
    a.tofile( f );
    
    clst_data.tofile( f );
    
    f.close();
    
    print 'Done';
    
    #print 'Debug: ';
    #print clst_data[0,0:3];
    #print clst_data[2,0:3];



if __name__=='__main__':
    
    if len(sys.argv)==1:
        
        abspath= '/home/relja/Relja/Code/e3misc/';
        abspathNew= '/data4/relja/Data/ox_exp/';
        
        prefix = 'oxc1_5k';
        #prefix = 'paris';
        
        detdesc= 'hesaff_sift';
        k= 1000000;
        seed= 43;
        
        clst_fn = '%sdata/%s/clst_%s_%s_%d_%d.h5'  % ( abspath, prefix, prefix, detdesc, k, seed );
        clst_fn_new = '%sclst_%s_%s_%d_%d.bin'  % ( abspathNew, prefix, detdesc, k, seed );
    
    elif len(sys.argv)==3:
        
        clst_fn= os.path.expanduser(sys.argv[1]);
        clst_fn_new= os.path.expanduser(sys.argv[2]);
    
    exportClusters( clst_fn, clst_fn_new );
    