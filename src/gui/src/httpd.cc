#include "gui.h"
#include "httpd.h"
#include <cstring>
#include <algorithm>

using namespace std;

namespace httpd {
    
bool
CaseInsensitiveLess::operator()(const string &s1, const string &s2) const 
{
    for (string::size_type i = 0; i < s1.size() && i < s2.size(); ++i) {
        char c1 = tolower(s1[i]);
        char c2 = tolower(s2[i]);
        if (c1 < c2) {
            return true;
        } else if (c1 > c2) {
            return false;
            }
    }
    return s1.size() < s2.size();
}

HttpServerException::HttpServerException(const string &message) 
{
    this->message = message;
}

HttpServerException::~HttpServerException() throw() 
{
}

const char *
HttpServerException::what() const throw() 
{
    return message.c_str();
}

HttpServer::HttpServer(uint16_t port, const string &rootDirectory, bool ipv6) 
{
    this->port = port;
    this->rootDirectory = rootDirectory;
    this->ipv6 = ipv6;
    this->sessionLife = 600;
    
    if (this->rootDirectory.size() == 0) {
        this->rootDirectory = "./";
    } 
    else if (*(this->rootDirectory.rbegin()) != '/') {
        this->rootDirectory += '/';
    }
}

HttpServer::~HttpServer() 
{
}

uint16_t HttpServer::getPort() const 
{
    return port;
}

const string &
HttpServer::getRootDirectory() const 
{
    return rootDirectory;
}

bool
HttpServer::isIPv6() const 
{
    return ipv6;
}

void
HttpServer::registerContentType(const string &extension, const string &type) 
{
    if (extension.size() > 0 && extension[0] == '.') {
        // The extension should have no dot
        throw HttpServerException(
            "Extension should not have dot at the beginning.");
    }
    contentTypeMap[extension] = type;
}

void
HttpServer::registerVirtualDirectory(
    const string &path, const string &directory) 
{
    if (path.size() == 0 || path[0] != '/') {
        throw HttpServerException("Path must be an absolute one.");
    }

    string dir = directory;
    
    if (dir.size() == 0) {
        dir = "./";
    } 
    else if (*dir.rbegin() != '/') {
        dir += '/';
    }
    
    if (*path.rbegin() != '/') {
        virtualDirectoryMap[path + "/"] = dir;
    } 
    else {
        virtualDirectoryMap[path] = dir;
    }
}

void
HttpServer::registerIncludeHandler(
    const string &ref, HttpServerIncludeHandler *handler) 
{
    if (ref.find('@') != string::npos) {
        throw HttpServerException("Include references cannot have '@'.");
    }
    
    includeHandlerMap[ref] = handler;
}

void
HttpServer::registerPageHandler(
    const string &path, HttpServerPageHandler *handler) 
{
    if (path.size() == 0 || path[0] != '/') {
        throw HttpServerException("Path must be an absolute one.");
    }
    
    pageHandlerMap[path] = handler;
}

void
HttpServer::registerAccessHandler(HttpServerAccessHandler *handler) 
{
    accessHandlers.push_back(handler);
}

void
HttpServer::setSessionLife(uint32_t sessionLife) 
{
    this->sessionLife = sessionLife;
}

void
HttpServer::cleanExpiredSession() 
{
    time_t expiration = time(NULL) - sessionLife;
    std::map<std::string, HttpSession*>::iterator iter = sessionMap.begin();
    while (iter != sessionMap.end()) {
        if (iter->second->lastAccess < expiration) {
            sessionMap.erase(iter++);
        } else {
            ++iter;
        }
    }
}

void
HttpServer::generateSession(const string &sessionId, HttpContext *context) 
{
    LOG_INFO(GUI_LOG, "New session with SID=%s", sessionId.c_str());
    context->response.headerMap["Set-Cookie"] = 
        "SID=" + sessionId + "; path=/";
    HttpSession *session = new HttpSession();
    session->sessionId = sessionId;
    session->lastAccess = time(NULL);
    sessionMap[session->sessionId] = session;
    context->session = session;
}

void 
HttpServer::getSession(HttpContext *context) 
{
    // Clean the session first
    cleanExpiredSession();
    
    HttpCookieMapType::const_iterator iter = 
        context->request.cookieMap.find("SID");
    if (iter == context->request.cookieMap.end() || iter->second.size() == 0) {
        // There is no session ID
        char possible[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        srand(time(NULL));
        string sessionId;
        for (int i = 0; i < 16; ++i) {
            sessionId += 
                possible[rand() % (sizeof(possible) / sizeof(char) - 1)];
        }
        generateSession(sessionId, context);
    } 
    else {
        map<string, HttpSession*>::const_iterator sessionIter = sessionMap.find(iter->second[0]);
        if (sessionIter == sessionMap.end()) {
            // The session ID cannot be found in the map, may be expired
            generateSession(iter->second[0], context);
        } else {
            // Mark the last access time
            sessionIter->second->lastAccess = time(NULL);
            context->session = sessionIter->second;
        }
    }
}

}
