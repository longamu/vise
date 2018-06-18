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

import os.path;

try:
    __file__= __file__;
except:
    __file__= sys.executable;
    
# scriptroot= os.path.dirname( os.path.abspath(__file__) );
# for cx_freeze:
scriptroot= os.path.dirname( os.path.abspath(__file__) );
pos_= scriptroot.find('/library.zip');
if pos_>0:
    scriptroot= scriptroot[:pos_];

def getScriptroot():
    return scriptroot;
