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

import os;

import webserver;



APIport= 35300;


server, config= webserver.get( [{'dsetname': 'ballads', 'APIport': APIport, 'enableUpload': True}], serverroot= 'ballads', webserverPort= 8080);


def Server():
    
    return server;
    