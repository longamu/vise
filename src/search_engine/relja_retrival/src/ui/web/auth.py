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
import urllib;


_username_key_= 'username';


class Authenticator:
    
    
    class Authenticator_:
        
        @staticmethod
        def needsLogin():
            def decorate(f):
                if not hasattr(f, '_cp_config'):
                    f._cp_config = dict();
                if 'auth.needsLogin' not in f._cp_config:
                    f._cp_config['auth.needsLogin']= True;
                return f;
            return decorate;
        
        @staticmethod
        def doesntNeedLogin():
            def decorate(f):
                if not hasattr(f, '_cp_config'):
                    f._cp_config = dict();
                if 'auth.doesntNeedLogin' not in f._cp_config:
                    f._cp_config['auth.doesntNeedLogin']= True;
                return f;
            return decorate;
     
    
    
    def __init__(self, pageTemplate, userpass= None, allNeedLogin= True, debugNoLogin= False):
        
        self.pT= pageTemplate;
        self.debugNoLogin= debugNoLogin;
        
        if userpass!=None:
            
            self.userpass= userpass;
            self.allNeedLogin= allNeedLogin;
            
            cherrypy.tools.auth= cherrypy.Tool('before_handler', self.enforceLoggedIn);
        
        else:
            cherrypy.tools.auth= cherrypy.Tool('before_handler', self.allPublic);
    
    
    
    def allPublic(self, *args, **kwargs):
        pass;
    
    
    
    def enforceLoggedIn(self, *args, **kwargs):
        
        needsLogin= cherrypy.request.config.get('auth.needsLogin', None);
        doesntNeedLogin= cherrypy.request.config.get('auth.doesntNeedLogin', None);
        
        needsLogin= \
             not(self.debugNoLogin) and \
             ( (needsLogin!=None) or \
               (self.allNeedLogin and doesntNeedLogin==None) );
        
        
        if needsLogin:
            
            username= cherrypy.session.get( _username_key_, default= None );
                        
            requestedParams= urllib.quote(cherrypy.request.request_line.split()[1]);
            
            if username:
                cherrypy.request.login= username;
            else:
                raise cherrypy.HTTPRedirect("login?fromPage=%s" % requestedParams);
            
    @cherrypy.expose
    @Authenticator_.doesntNeedLogin()
    def login(self, username=None, password=None, fromPage=""):
        
        if username==None or password==None:
            return self.getLogin(fromPage= fromPage);
        else:
            if (username in self.userpass) and (self.userpass[username]==password):
                # logged in
                cherrypy.request.login= username;
                
                cherrypy.session.regenerate();
                cherrypy.session[ _username_key_ ]= username;
                
                raise cherrypy.HTTPRedirect(fromPage);
            else:
                return self.getLogin(username= username, message= "Invalid username or password", fromPage= fromPage);
    
    @cherrypy.expose
    @Authenticator_.doesntNeedLogin()
    def getLogin(self, username= "", message="Please log in", fromPage=""):
        
        body= """
            <center>
            
            <h3>%s</h3><br>
            
            <form method="POST" action="login">
            <input type="hidden" value="%s" name="fromPage">
            
            <table width="200" cellpadding="5">
            <tr><td>Username</td><td>
                <input type="text" name="username" value="%s">
            </td></tr>
            <tr><td>Password</td><td>
                <input type="password" name="password">
            </td></tr>
                        
            </table>
            <br>
            <input type="submit" value="Log in">
            
            </form>
            
            </center>
        """ % (message, fromPage, username);
        
        return self.pT.get(title= "Log in", body= body);
    
    @cherrypy.expose
    @Authenticator_.doesntNeedLogin()
    def logout(self):
        
        username= cherrypy.session.get( _username_key_, default= None );
        if username:
            cherrypy.session[ _username_key_ ]= None;
            cherrypy.lib.sessions.expire();
            cherrypy.request.login= None;
        
        return self.pT.get(title= "Log in", body= "Logged out");
    
    @staticmethod
    def needsLogin():
        def decorate(f):
            if not hasattr(f, '_cp_config'):
                f._cp_config = dict();
            if 'auth.needsLogin' not in f._cp_config:
                f._cp_config['auth.needsLogin']= True;
            return f;
        return decorate;
    
    @staticmethod
    def doesntNeedLogin():
        def decorate(f):
            if not hasattr(f, '_cp_config'):
                f._cp_config = dict();
            if 'auth.doesntNeedLogin' not in f._cp_config:
                f._cp_config['auth.doesntNeedLogin']= True;
            return f;
        return decorate;



class Test:
    
    
    def __init__(self, auth_obj):
        
        self.auth_obj= auth_obj;
        
        self.login= auth_obj.login;
        self.logout= auth_obj.logout;
        
    
    @cherrypy.expose
    @Authenticator.needsLogin()
    def index( self, temp='12"3"21' ):
        
        username= cherrypy.request.login;
        
        return """
            Logged in as %s<br>
            temp= %s<br>
            <a href="logout">Log out</a>
        """ % (username, temp);
        
    


if __name__=='__main__':
    
    auth_obj= Authenticator( { 'relja' : 'bla' }, allNeedLogin= True );
    
    cherrypy.config.update({
          'server.socket_port' : 8083,\
          'server.socket_host' : "0.0.0.0",\
          'server.keepalive' : False
          });
    
    
    conf= {
        '/': {
            'tools.sessions.on': True,
            'tools.auth.on': True
        }
    };
    
    cherrypy.quickstart( Test(auth_obj), '/', config= conf );
