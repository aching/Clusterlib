#include "mhd_httpd.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>

using namespace std;
using namespace log4cxx;

namespace httpd {namespace microhttpd {
    LoggerPtr MicroHttpServer::logger(Logger::getLogger("httpd.microhttpd.MicroHttpServer"));

    MicroHttpServer::MicroHttpServer(uint16_t port, const std::string &rootDirectory, bool ipv6) :
        HttpServer(port, rootDirectory, ipv6), httpDaemon(NULL) {
    }


    MicroHttpServer::~MicroHttpServer() {
        if (httpDaemon) {
            stop();
        }
    }

    void MicroHttpServer::stop() {
        MHD_stop_daemon(httpDaemon);
        httpDaemon = NULL;
    }

    void MicroHttpServer::start() {
        unsigned int option = MHD_USE_SELECT_INTERNALLY;

        if (ipv6) {
            option |= MHD_USE_IPv6;
        }

        httpDaemon = MHD_start_daemon(option, port, acceptPolicyCallback, this, accessHandlerCallback, this,
                                      MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, this, MHD_OPTION_END);
        if (!httpDaemon) {
            char buf[33];
            snprintf(buf, sizeof(buf), "%d", port);
            throw HttpServerException(string("Starting Micro Httpd Server on port ") + buf + " with root directory " + rootDirectory + " failed.");
        }
    }

    int MicroHttpServer::accessHandlerCallback(void *cls, MHD_Connection *connection, const char *url, const char *method,
                                               const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
        MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);

        return _this->accessHandler(connection, url, method, version, upload_data, upload_data_size, con_cls);
    }

    int MicroHttpServer::acceptPolicyCallback(void *cls, const struct sockaddr *addr, socklen_t addrlen) {
        MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);

        return _this->acceptPolicy(addr, addrlen);
    }

    int MicroHttpServer::requestCompletedCallback(void *cls, MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe) {
        MicroHttpServer *_this = reinterpret_cast<MicroHttpServer *>(cls);

        return _this->requestCompleted(connection, con_cls, toe);
    }

    int MicroHttpServer::keyValueCallback(void *cls, MHD_ValueKind kind, const char *key, const char *value) {
        vector<pair<string, string> > *keyValues = reinterpret_cast<vector<pair<string, string> > *>(cls);
        keyValues->push_back(make_pair(string(key), string(value)));

        return MHD_YES;
    }

    int MicroHttpServer::contentReaderCallback(void *cls, uint64_t pos, char *buf, int max) {
        HttpFileContext *fc = reinterpret_cast<HttpFileContext *>(cls);

        return fc->_this->contentReader(fc, pos, buf, max);
    }

    void MicroHttpServer::contentReaderFreeCallback(void *cls) {
        HttpFileContext *fc = reinterpret_cast<HttpFileContext *>(cls);

        if (fc->file.is_open()) {
            fc->file.close();
        }

        delete fc;
    }

    int MicroHttpServer::accessHandler(MHD_Connection *connection, const char *url, const char *method,
                                               const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
        HttpContext *&context = *reinterpret_cast<HttpContext **>(con_cls);

        if (!context) {
            const MHD_ConnectionInfo *info = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
            context = new HttpContext();
            context->client = getSockAddrPresentation(info->client_addr, ipv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in));
            context->request.method = method;
            context->request.url = url;
            context->request.protocolVersion = version;
            if (context->request.method != MHD_HTTP_METHOD_GET && context->request.method != MHD_HTTP_METHOD_POST) {
                // Only GET and POST methods are supported.
                return MHD_NO;
            }

            if (context->request.url.find("/..") != string::npos) {
                // "/.." are not allowed in the URL to avoid security issues
                return MHD_NO;
            }

            // Parse the headers
            vector<pair<string, string> > keyValues;
            MHD_get_connection_values(connection, MHD_HEADER_KIND, keyValueCallback, &keyValues);
            
            for (vector<pair<string, string> >::const_iterator iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
                context->request.headerMap[iter->first] = iter->second;
            }

            // Parse the cookies
            keyValues.clear();
            MHD_get_connection_values(connection, MHD_COOKIE_KIND, keyValueCallback, &keyValues);
            
            for (vector<pair<string, string> >::const_iterator iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
                context->request.cookieMap[iter->first].push_back(iter->second);
            }

            // Parse the queries
            keyValues.clear();
            MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, keyValueCallback, &keyValues);
            
            for (vector<pair<string, string> >::const_iterator iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
                context->request.queryMap[iter->first].push_back(iter->second);
            }
        } else if (*upload_data_size) {
            if (context->request.method != MHD_HTTP_METHOD_POST) {
                // Only post method allows for data
                return MHD_NO;
            }
            
            if ((context->request.body.size() + *upload_data_size) > HTTP_MAX_REQUEST_SIZE) {
                // When the request is too large
                return MHD_NO;
            }

            // Consume the data and add it back to the request body
            context->request.body += string(upload_data, *upload_data_size);
            *upload_data_size = 0;
        } else {
            // Otherwise, dispatch the request
            return dispatchRequest(connection, context);
        }

        return MHD_YES;
    }

    int MicroHttpServer::dispatchRequest(MHD_Connection *connection, HttpContext *context) {
        int ret = MHD_YES;

        // Get/generate the session
        getSession(context);

        LOG4CXX_INFO(logger, "Dispatching the request on " << context->request.url << ", session ID=" << context->session->sessionId);
        LOG4CXX_DEBUG(logger, "Body: " << context->request.body);

        if (pageHandlerMap.find(context->request.url) != pageHandlerMap.end()) { 
            LOG4CXX_INFO(logger, "Page handler found for " + context->request.url);
            // Found a page handler, has the top priority
            try {
                pageHandlerMap[context->request.url]->pageHandler(context->request.url, context);
            } catch (const HttpServerException &ex) {
                context->response.body = string("Internal error occurred.") + HTTP_CRLF;
                context->response.body += string("Cause: ") + ex.what() + HTTP_CRLF;
                context->response.headerMap.clear();
                context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = "text/plain";
                context->response.responseCode = HTTP_INTERNAL_SERVER_ERROR;
            }

            LOG4CXX_DEBUG(logger, "Response Code: " << context->response.responseCode);

            // Send the response back
            MHD_Response *response = MHD_create_response_from_data(context->response.body.size(), (void *)context->response.body.c_str(), MHD_NO, MHD_NO);
            
            if (response == NULL) {
                return MHD_NO;
            }

            for (HttpHeaderMapType::const_iterator iter = context->response.headerMap.begin(); iter != context->response.headerMap.end(); ++iter) {
                MHD_add_response_header(response, iter->first.c_str(), iter->second.c_str());
            }

            ret = MHD_queue_response(connection, context->response.responseCode, response);
            MHD_destroy_response(response);
        } else {
            LOG4CXX_INFO(logger, "Processing file for " << context->request.url);
            // File processing
            pair<string, string> match = make_pair("/", rootDirectory);
            for (map<string, string>::reverse_iterator iter = virtualDirectoryMap.rbegin(); iter != virtualDirectoryMap.rend(); ++iter) {
                const string &path = iter->first;
                if (path.size() < context->request.url.size() && context->request.url.compare(0, path.size(), path) == 0) {
                    match = *iter;
                    break;
                }
            }

            string realFile = match.second + context->request.url.substr(match.first.size());
            LOG4CXX_DEBUG(logger, "Matched virtual directory: " << match.first << ", physical directory: " << match.second);

            if (isDirectory(realFile)) {
                // If the file cannot be open, try access as a directory
                // Default file would be index.html
                if (*realFile.rbegin() == '/') {
                    realFile += "index.html";
                } else {
                    realFile += "/index.html";
                }
            }

            HttpFileContext *fc = new HttpFileContext();
            fc->_this = this;
            fc->context = context;
            fc->file.open(realFile.c_str(), ifstream::in | ifstream::binary);

            LOG4CXX_DEBUG(logger, "The physical file name: " << realFile);

            if (fc->file.fail()) {
                // File not found
                delete fc;

                // Send the 404 response back
                context->response.body = string("Location not found.") + HTTP_CRLF;
                context->response.headerMap.clear();
                context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = "text/plain";
                context->response.responseCode = HTTP_NOT_FOUND;

                MHD_Response *response = MHD_create_response_from_data(context->response.body.size(), (void *)context->response.body.c_str(), MHD_NO, MHD_NO);
                
                if (response == NULL) {
                    return MHD_NO;
                }
                
                for (HttpHeaderMapType::const_iterator iter = context->response.headerMap.begin(); iter != context->response.headerMap.end(); ++iter) {
                    MHD_add_response_header(response, iter->first.c_str(), iter->second.c_str());
                }
                
                ret = MHD_queue_response(connection, context->response.responseCode, response);
                MHD_destroy_response(response);
            } else {
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

                LOG4CXX_DEBUG(logger, "Content type: " << contentType << " extension:" << extension);

                fc->includeHandlerProcess = contentType.compare(0, 5, "text/") == 0;

                context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = contentType;
                context->response.responseCode = HTTP_OK;

                // Send the response back
                off_t realFileSize = getFileSize(realFile);

                MHD_Response *response = MHD_create_response_from_callback(fc->includeHandlerProcess ? MHD_RESPONSE_UNKNOWN_TOTAL_SIZE : realFileSize, 4*1024, contentReaderCallback, fc, contentReaderFreeCallback);                 
                if (response == NULL) {
                    LOG4CXX_DEBUG(logger, "Null response back with size = "
                                  << realFileSize << ", name = " << realFile);
                    return MHD_NO;
                }
                
                LOG4CXX_DEBUG(logger, "Send the response back with size "
                              << getFileSize(realFile));

                for (HttpHeaderMapType::const_iterator iter = context->response.headerMap.begin(); iter != context->response.headerMap.end(); ++iter) {
                    MHD_add_response_header(response, iter->first.c_str(), iter->second.c_str());
                }
                
                ret = MHD_queue_response(connection, context->response.responseCode, response);
                MHD_destroy_response(response);
            }
        }

        return ret;
    }

    off_t MicroHttpServer::getFileSize(const string &file) {
        struct stat64 fileinfo;
        if (stat64(file.c_str(), &fileinfo) < 0) return 0;
        return fileinfo.st_size;
    }

    bool MicroHttpServer::isDirectory(const string &file) {
        struct stat64 fileinfo;
        if (stat64(file.c_str(), &fileinfo) < 0) return false;
        return S_ISDIR(fileinfo.st_mode);
    }

    int MicroHttpServer::contentReader(HttpFileContext *fc, uint64_t pos, char *buf, int max) {
        LOG4CXX_DEBUG(logger, "Starting contentReader with offset = " << pos 
                      << ", max = " << max);        
        if (fc->buffer.size() == 0) {
            // Only when the buffer is empty, we read from the file
            char rdbuf[4096];
            while (fc->file.good() && fc->buffer.size() < (string::size_type)max) {
                fc->file.read(rdbuf, 4096);
                fc->buffer += string(rdbuf, fc->file.gcount());
            }

            // After reading, we still have nothing, it is end of chunk.
            if (fc->buffer.size() == 0) {
                LOG4CXX_DEBUG(logger, "End of chunk.");
                // Either error occurred or nothing to read
                return -1;
            }

            if (fc->includeHandlerProcess) {
                // process the include handler
                ostringstream oss;
                string::size_type pos = 0;
                string::size_type lastpos = 0;
                while (lastpos >= fc->buffer.size() || (pos = fc->buffer.find("<@", lastpos)) != string::npos) {
                    string::size_type next = fc->buffer.find("@>", pos + 2);
                    // The ending "@>" is beyond the buffer
                    if (next == string::npos) {
                        uint32_t count = 0;
                        char ch = 0;
                        // If the next "@>" is missing, read at most 128 bytes to find it
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

                    LOG4CXX_INFO(logger, "Processing directive '" << directive << "'");

                    map<string, HttpServerIncludeHandler*>::const_iterator iter = includeHandlerMap.find(directive);

                    if (iter == includeHandlerMap.end()) {
                        LOG4CXX_WARN(logger, "Directive '" << directive << "' not supported.");
                        oss << "<!--MISSING INCLUDE HANDLER '" << directive << "'-->";
                    } else {
                        // Execute the include handler
                        fc->context->response.body = "";
                        try {
                            iter->second->includeHandler(directive, fc->context);
                            LOG4CXX_INFO(logger, "Directive '" << directive << "' processed.");
                        } catch (const HttpServerException &ex) {
                            LOG4CXX_WARN(logger, "Directive '" << directive << "' processing error (" << ex.what() << ").");
                            fc->context->response.body = "<!--ERROR INCLUDE HANDLER '" + directive + "': " + ex.what() + "-->";
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
        int size = ((int)fc->buffer.size() > max) ? max : fc->buffer.size();
        LOG4CXX_DEBUG(logger, "Sending chunk with offset = " << pos 
                      << ", size = " << size << ", max = " << max);        

        memcpy(buf, fc->buffer.c_str(), size);
        fc->buffer = fc->buffer.substr(size);

        return size;
    }


    string MicroHttpServer::getSockAddrPresentation(const struct sockaddr *addr, socklen_t addrlen) {
        // The maximum size of DNS is at present 255 (RFC 2181)
        char buf[256];
        getnameinfo(addr, addrlen, buf, sizeof(buf), NULL, 0, 0);

        return buf;
    }

    int MicroHttpServer::acceptPolicy(const struct sockaddr *addr, socklen_t addrlen) {
        HttpContext context;

        context.client = getSockAddrPresentation(addr, addrlen);

        LOG4CXX_INFO(logger, "Client " + context.client + " connecting.");

        for (list<HttpServerAccessHandler*>::iterator iter = accessHandlers.begin(); iter != accessHandlers.end(); ++iter) {
            switch ((*iter)->canAccess(context)) {
            case HttpServerAccessHandler::ALLOW:
                LOG4CXX_INFO(logger, "Connection from " + context.client + " accepeted.");
                return MHD_YES;
            case HttpServerAccessHandler::DENY:
                LOG4CXX_INFO(logger, "Connection from " + context.client + " refused.");
                return MHD_NO;
            case HttpServerAccessHandler::NEXT:
                // The next handler judge the result
                break;
            }
        }
        
        // By default, it is allowed
        LOG4CXX_INFO(logger, "Connection from " + context.client + " accepeted.");
        return MHD_YES;
    }

    int MicroHttpServer::requestCompleted(MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe) {
        HttpContext *&context = *reinterpret_cast<HttpContext **>(con_cls);
        LOG4CXX_INFO(logger, "Connection completed.");
        
        if (context) {
            // When the HttpContext exists, free the space
            delete context;
            context = NULL;
        }
        
        return MHD_YES;
    }
}}
