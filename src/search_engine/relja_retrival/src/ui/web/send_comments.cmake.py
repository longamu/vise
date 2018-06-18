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

import cherrypy;

import smtplib;
from email.mime.text import MIMEText;

import sqlite3;
import time;


class sendComments:
    
    def __init__(self, pageTemplate):
        self.pT= pageTemplate;
    
    
    @cherrypy.expose
    def index(self, toemail, subject, emailbody, tickedList, queryRec):
        
        emailbody+= "\n\nSelected image names:\n\n" + tickedList;
        
        fromemail= 'relja.automatic@gmail.com';
        
        msg = MIMEText( emailbody );
        msg['Subject']= subject;
        msg['From']= fromemail;
        msg['To']= toemail;
        
        server= smtplib.SMTP('smtp.gmail.com',587);
        server.ehlo();
        server.starttls();
        server.ehlo();
        server.login(fromemail,'@RR_AUTO_EMAIL_PASS_PY@');

        server.sendmail( fromemail, [toemail], msg.as_string() );
        
        server.close();
        
        body="""
        Email sent!<br>
        <a href="page0">Return to home page</a><br><br>
        
        To: %s <br>
        Subject: %s <br>
        Body: <br><br>
        <pre>
        %s
        </pre>
        """ % (toemail, subject, emailbody);
        
        # record activity
        
        c= sqlite3.connect('output/anno.db');
        annoID= c.execute('select max(annoID) from anno').fetchone()[0];
        if annoID!=None:
            annoID= annoID+1;
        else:
            annoID= 0;
        c.execute('insert into anno(annoID,queryRec,date,toemail,body) values(?,?,?,?,?)', [annoID, queryRec, time.asctime(), toemail, emailbody] );
        c.commit();
        c.close();
        
        return self.pT.get(title= "Email sent", body= body);
    
