#include "gui.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>

/* For older versions of libmicrohttpd */
#ifndef MHD_SIZE_UNKNOWN 
#define MHD_SIZE_UNKNOWN  ((uint64_t) -1LL)
#endif

using namespace std;

namespace httpd { namespace microhttpd {

MicroHttpServer::MicroHttpServer(uint16_t port, 
                                 const string &rootDirectory, 
                                 bool ipv6) 
    : HttpServer(port, rootDirectory, ipv6), httpDaemon(NULL) 
{
}

MicroHttpServer::~MicroHttpServer() 
{
    if (httpDaemon) {
        stop();
    }
}

void
MicroHttpServer::stop() 
{
    MHD_stop_daemon(httpDaemon);
    httpDaemon = NULL;
}

void
MicroHttpServer::start() 
{
    unsigned int option = MHD_USE_SELECT_INTERNALLY;
    
    if (ipv6) {
        option |= MHD_USE_IPv6;
    }
    
    httpDaemon = MHD_start_daemon(option, 
                                  port, 
                                  acceptPolicyCallback, 
                                  this, 
                                  accessHandlerCallback, 
                                  this,
                                  MHD_OPTION_NOTIFY_COMPLETED, 
                                  requestCompletedCallback, 
                                  this, 
                                  MHD_OPTION_END);
    if (!httpDaemon) {
        char buf[33];
        snprintf(buf, sizeof(buf), "%d", port);
        throw HttpServerException(
            string("Starting Micro Httpd Server on port ") + buf + 
            " with root directory " + rootDirectory + " failed.");
    }
}

int 
MicroHttpServer::accessHandlerCallback(void *cls, 
                                       MHD_Connection *connection, 
                                       const char *url, 
                                       const char *method,
                                       const char *version, 
                                       const char *upload_data, 
                                       size_t *upload_data_size, 
                                       void **con_cls) 
{
    MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);
    
    return _this->accessHandler(connection, 
                                url, 
                                method, 
                                version, 
                                upload_data, 
                                upload_data_size, 
                                con_cls);
}

int
MicroHttpServer::acceptPolicyCallback(
    void *cls, const struct sockaddr *addr, socklen_t addrlen) 
{
    MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);
    
    return _this->acceptPolicy(addr, addrlen);
}

int
MicroHttpServer::requestCompletedCallback(void *cls, 
                                          MHD_Connection *connection, 
                                          void **con_cls, 
                                          MHD_RequestTerminationCode toe) 
{
    MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);
    
    return _this->requestCompleted(connection, con_cls, toe);
}

int
MicroHttpServer::keyValueCallback(
    void *cls, MHD_ValueKind kind, const char *key, const char *value) 
{
    vector<pair<string, string> > *keyValues = 
        reinterpret_cast<vector<pair<string, string> > *>(cls);
    keyValues->push_back(make_pair(string(key), string(value)));
    
    return MHD_YES;
}

int
MicroHttpServer::contentReaderCallback(
    void *cls, uint64_t pos, char *buf, int max) 
{
    HttpFileContext *fc = reinterpret_cast<HttpFileContext *>(cls);
    
    return fc->_this->contentReader(fc, pos, buf, max);
}

void
MicroHttpServer::contentReaderFreeCallback(void *cls) 
{
    HttpFileContext *fc = reinterpret_cast<HttpFileContext *>(cls);
    
    if (fc->file.is_open()) {
        fc->file.close();
    }

    delete fc;
}

int
MicroHttpServer::accessHandler(MHD_Connection *connection, 
                               const char *url, 
                               const char *method,
                               const char *version, 
                               const char *upload_data, 
                               size_t *upload_data_size, 
                               void **con_cls) 
{
    HttpContext *&context = *reinterpret_cast<HttpContext **>(con_cls);
    
    if (!context) {
        const MHD_ConnectionInfo *info = 
            MHD_get_connection_info(connection, 
                                    MHD_CONNECTION_INFO_CLIENT_ADDRESS);
        context = new HttpContext();
        context->client = getSockAddrPresentation(
            (struct sockaddr *) info->client_addr, 
            ipv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in));
        context->request.method = method;
        context->request.url = url;
        context->request.protocolVersion = version;
        if (context->request.method != MHD_HTTP_METHOD_GET && 
            context->request.method != MHD_HTTP_METHOD_POST) {
            /* Only GET and POST methods are supported. */
            return MHD_NO;
        }

        if (context->request.url.find("/..") != string::npos) {
            /* "/.." are not allowed in the URL to avoid security issues */
            return MHD_NO;
        }

        /* Parse the headers */
        vector<pair<string, string> > keyValues;
        MHD_get_connection_values(
            connection, MHD_HEADER_KIND, keyValueCallback, &keyValues);
        
        vector<pair<string, string> >::const_iterator iter;
        for (iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
            context->request.headerMap[iter->first] = iter->second;
        }

        /* Parse the cookies */
        keyValues.clear();
        MHD_get_connection_values(
            connection, MHD_COOKIE_KIND, keyValueCallback, &keyValues);
            
        for (iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
            context->request.cookieMap[iter->first].push_back(iter->second);
        }

        /* Parse the queries */
        keyValues.clear();
        MHD_get_connection_values(
            connection, MHD_GET_ARGUMENT_KIND, keyValueCallback, &keyValues);
        
        for (iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
            context->request.queryMap[iter->first].push_back(iter->second);
        }
    }
    else if (*upload_data_size) {
        if (context->request.method != MHD_HTTP_METHOD_POST) {
            /* Only post method allows for data */
            return MHD_NO;
        }
            
        if ((context->request.body.size() + *upload_data_size) > 
            HTTP_MAX_REQUEST_SIZE) {
            /* When the request is too large */
            return MHD_NO;
        }

        /* Consume the data and add it back to the request body */
        context->request.body += string(upload_data, *upload_data_size);
        *upload_data_size = 0;
    } 
    else {
        /* Otherwise, dispatch the request */
        return dispatchRequest(connection, context);
    }

    return MHD_YES;
}

int
MicroHttpServer::dispatchRequest(
    MHD_Connection *connection, HttpContext *context) 
{
    int ret = MHD_YES;
    
    /* Get/generate the session */
    getSession(context);
    
    LOG_INFO(GUI_LOG, 
             "Dispatching the request on %s with session ID = %s",
             context->request.url.c_str(),
             context->session->sessionId.c_str());
    LOG_DEBUG(GUI_LOG,
              "Body: %s", 
              context->request.body.c_str());
    
    if (pageHandlerMap.find(context->request.url) != pageHandlerMap.end()) { 
        LOG_INFO(GUI_LOG, 
                 "Page handler found for %s", 
                 context->request.url.c_str());
        /* Found a page handler, has the top priority */
        try {
            pageHandlerMap[context->request.url]->pageHandler(
                context->request.url, context);
        }
        catch (const HttpServerException &ex) {
            context->response.body = 
                string("Internal error occurred.") + HTTP_CRLF;
            context->response.body += 
                string("Cause: ") + ex.what() + HTTP_CRLF;
            context->response.headerMap.clear();
            context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = 
                "text/plain";
            context->response.responseCode = HTTP_INTERNAL_SERVER_ERROR;
        }
        
        LOG_DEBUG(GUI_LOG, 
                  "Response Code: %" PRId32,
                  static_cast<int32_t>(context->response.responseCode));
        
        /* Send the response back */
        MHD_Response *response = MHD_create_response_from_data(
            context->response.body.size(), 
            (void *)context->response.body.c_str(), 
            MHD_NO, 
            MHD_NO);
            
        if (response == NULL) {
            return MHD_NO;
        }
        
        for (HttpHeaderMapType::const_iterator iter = 
                 context->response.headerMap.begin(); 
             iter != context->response.headerMap.end(); 
             ++iter) {
            MHD_add_response_header(
                response, iter->first.c_str(), iter->second.c_str());
        }
        
        ret = MHD_queue_response(
            connection, context->response.responseCode, response);
        MHD_destroy_response(response);
    }
    else {
        LOG_INFO(GUI_LOG,
                 "Processing file for %s", 
                 context->request.url.c_str());
        /* File processing */
        pair<string, string> match = make_pair("/", rootDirectory);
        for (map<string, string>::reverse_iterator iter = 
                 virtualDirectoryMap.rbegin(); 
             iter != virtualDirectoryMap.rend(); 
             ++iter) {
            const string &path = iter->first;
            if (path.size() < context->request.url.size() && 
                context->request.url.compare(0, path.size(), path) == 0) {
                match = *iter;
                break;
            }
        }
        
        string realFile = 
            match.second + context->request.url.substr(match.first.size());
        LOG_DEBUG(GUI_LOG,
                  "Matched virtual directory: %s, physical directory: %s",
                  match.first.c_str(),
                  match.second.c_str());
        
        if (isDirectory(realFile)) {
            /*
             * If the file cannot be open, try access as a directory
             * Default file would be index.html
             */
            if (*realFile.rbegin() == '/') {
                realFile += "index.html";
            } 
            else {
                realFile += "/index.html";
            }
        }
        
        HttpFileContext *fc = new HttpFileContext();
        fc->_this = this;
        fc->context = context;
        fc->file.open(realFile.c_str(), ifstream::in | ifstream::binary);
        
        LOG_DEBUG(GUI_LOG, "The physical file name: %s", realFile.c_str());
        
        if (fc->file.fail()) {
            /* File not found */
            delete fc;
            
            /* Send the 404 response back */
            context->response.body = string("Location not found.") + HTTP_CRLF;
            context->response.headerMap.clear();
            context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = "text/plain";
            context->response.responseCode = HTTP_NOT_FOUND;
            
            MHD_Response *response = MHD_create_response_from_data(context->response.body.size(), (void *)context->response.body.c_str(), MHD_NO, MHD_NO);
            
            if (response == NULL) {
                return MHD_NO;
            }
            
            for (HttpHeaderMapType::const_iterator iter = 
                     context->response.headerMap.begin(); 
                 iter != context->response.headerMap.end();
                 ++iter) {
                MHD_add_response_header(
                    response, iter->first.c_str(), iter->second.c_str());
            }
            
            ret = MHD_queue_response(
                connection, context->response.responseCode, response);
            MHD_destroy_response(response);
        }
        else {
            string::size_type pos = realFile.find_last_of("./");
            string extension("");
            
            if (pos != string::npos && realFile[pos] == '.') {
                // There is extension
                extension = realFile.substr(pos + 1);
            }
            
            string contentType = HTTP_DEFAULT_CONTENT_TYPE;
            if (contentTypeMap.find(extension) != contentTypeMap.end()) {
                contentType = contentTypeMap[extension];
            }
            
            LOG_DEBUG(GUI_LOG, "Content type: %s, extension: %s",
                      contentType.c_str(),
                      extension.c_str());

            fc->includeHandlerProcess = 
                contentType.compare(0, 5, "text/") == 0;
            
            context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = 
                contentType;
            context->response.responseCode = HTTP_OK;
            
            // Send the response back
            off_t realFileSize = getFileSize(realFile);

            MHD_Response *response = MHD_create_response_from_callback(
                fc->includeHandlerProcess ? MHD_SIZE_UNKNOWN : realFileSize, 
                4*1024, 
                contentReaderCallback, 
                fc, 
                contentReaderFreeCallback);                 
            if (response == NULL) {
                LOG_DEBUG(GUI_LOG,
                          "Null response back with size = %" PRId32 
                          ", name = %s",
                          static_cast<int32_t>(realFileSize),
                          realFile.c_str());
                return MHD_NO;
            }
            
            LOG_DEBUG(GUI_LOG, 
                      "Send the response back with size %" PRId32,
                      static_cast<int32_t>(getFileSize(realFile)));
            
            for (HttpHeaderMapType::const_iterator iter = 
                     context->response.headerMap.begin(); 
                 iter != context->response.headerMap.end(); 
                 ++iter) {
                MHD_add_response_header(
                    response, iter->first.c_str(), iter->second.c_str());
            }
            
            ret = MHD_queue_response(
                connection, context->response.responseCode, response);
            MHD_destroy_response(response);
        }
    }
    
    return ret;
}

off_t
MicroHttpServer::getFileSize(const string &file) 
{
    struct stat fileinfo;
    if (stat(file.c_str(), &fileinfo) < 0) {
        return 0;
    }
    return fileinfo.st_size;
}

bool
MicroHttpServer::isDirectory(const string &file) 
{
    struct stat fileinfo;
    if (stat(file.c_str(), &fileinfo) < 0) {
        return false;
    }
    return S_ISDIR(fileinfo.st_mode);
}

int
MicroHttpServer::contentReader(
    HttpFileContext *fc, uint64_t pos, char *buf, int max) 
{
    LOG_DEBUG(GUI_LOG, 
              "Starting contentReader with offset = %" PRIu64 
              ", max = %" PRId32,
              pos,
              static_cast<int32_t>(max));
    if (fc->buffer.size() == 0) {
        // Only when the buffer is empty, we read from the file
        char rdbuf[4096];
        while (fc->file.good() && fc->buffer.size() < (string::size_type)max) {
            fc->file.read(rdbuf, 4096);
            fc->buffer += string(rdbuf, fc->file.gcount());
        }
        
        // After reading, we still have nothing, it is end of chunk.
        if (fc->buffer.size() == 0) {
            LOG_DEBUG(GUI_LOG, "End of chunk.");
            // Either error occurred or nothing to read
            return -1;
        }
        
        if (fc->includeHandlerProcess) {
            // process the include handler
            ostringstream oss;
            string::size_type pos = 0;
            string::size_type lastpos = 0;
            while (lastpos >= fc->buffer.size() || 
                   (pos = fc->buffer.find("<@", lastpos)) != string::npos) {
                string::size_type next = fc->buffer.find("@>", pos + 2);
                // The ending "@>" is beyond the buffer
                if (next == string::npos) {
                    uint32_t count = 0;
                    char ch = 0;
                    // If the next "@>" is missing, read at most 128
                    // bytes to find it
                    while (fc->file.good() && count < 128) {
                        char lastch = ch;
                        ch = (char) fc->file.get();
                        ++count;
                        fc->buffer += ch;
                        if (ch == '>' && lastch == '@') {
                            next = fc->buffer.size() - 2;
                            break;
                        }
                    }
                }
                
                if (next == string::npos) {
                    // Cannot find the ending "@>"
                    break;
                }
                
                string directive(fc->buffer, pos + 2, next - pos - 2);
                oss << fc->buffer.substr(lastpos, pos - lastpos);
                
                LOG_INFO(GUI_LOG, 
                         "Processing directive '%s'", 
                         directive.c_str());
                
                map<string, HttpServerIncludeHandler*>::const_iterator iter = 
                    includeHandlerMap.find(directive);
                
                if (iter == includeHandlerMap.end()) {
                    LOG_WARN(GUI_LOG, 
                             "Directive '%s' not supported.",
                             directive.c_str());
                    oss << "<!--MISSING INCLUDE HANDLER '" 
                        << directive << "'-->";
                } 
                else {
                    // Execute the include handler
                    fc->context->response.body = "";
                    try {
                        iter->second->includeHandler(directive, fc->context);
                        LOG_INFO(GUI_LOG,
                                 "Directive '%s' processed.",
                                 directive.c_str());
                    } catch (const HttpServerException &ex) {
                        LOG_WARN(GUI_LOG, 
                                 "Directive '%s' processing error (%s)",
                                 directive.c_str(),
                                 ex.what());
                        fc->context->response.body = 
                            "<!--ERROR INCLUDE HANDLER '" + directive + "': " +
                            ex.what() + "-->";
                    }
                    
                    oss << fc->context->response.body;
                }
                
                lastpos = next + 2;
            }
            
            if (lastpos < fc->buffer.size()) {
                oss << fc->buffer.substr(lastpos);
            }
            
            fc->buffer = oss.str();
        }
    }
    
    // Send the buffer.
    int64_t size = ((int)fc->buffer.size() > max) ? max : fc->buffer.size();
    LOG_DEBUG(GUI_LOG,
              "Sending chunk with offset = %" PRIu64 ", size = %" PRId64 
              ", max = %" PRId32,
              pos,
              size,
              static_cast<int32_t>(max));        
    
    memcpy(buf, fc->buffer.c_str(), size);
    fc->buffer = fc->buffer.substr(size);
    
    return size;
}

string
MicroHttpServer::getSockAddrPresentation(
    const struct sockaddr *addr, socklen_t addrlen) 
{
    // The maximum size of DNS is at present 255 (RFC 2181)
    char buf[256];
    getnameinfo(addr, addrlen, buf, sizeof(buf), NULL, 0, 0);
    
    return buf;
}

int
MicroHttpServer::acceptPolicy(const struct sockaddr *addr, socklen_t addrlen) 
{
    HttpContext context;
    
    context.client = getSockAddrPresentation(addr, addrlen);
    
    LOG_INFO(GUI_LOG, "Client %s connecting", context.client.c_str());
    
    for (list<HttpServerAccessHandler*>::iterator iter = 
             accessHandlers.begin(); 
         iter != accessHandlers.end(); 
         ++iter) {
        switch ((*iter)->canAccess(context)) {
            case HttpServerAccessHandler::ALLOW:
                LOG_INFO(GUI_LOG,
                         "Connection from %s accepted.", 
                         context.client.c_str());
                return MHD_YES;
            case HttpServerAccessHandler::DENY:
                LOG_INFO(GUI_LOG,
                         "Connection from %s refused.", 
                         context.client.c_str());
                return MHD_NO;
            case HttpServerAccessHandler::NEXT:
                // The next handler judge the result
                break;
        }
    }
    
    // By default, it is allowed
    LOG_INFO(GUI_LOG, "Connection from %s accepted", context.client.c_str());
    return MHD_YES;
}

int
MicroHttpServer::requestCompleted(
    MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe)
{
    HttpContext *&context = *reinterpret_cast<HttpContext **>(con_cls);
    LOG_INFO(GUI_LOG, "Connection completed.");
    
    if (context) {
        // When the HttpContext exists, free the space
        delete context;
        context = NULL;
    }
    
    return MHD_YES;
}

}}
