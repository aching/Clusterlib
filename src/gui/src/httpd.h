#ifndef _CL_HTTPD_H_
#define _CL_HTTPD_H_

namespace httpd {

const std::string HTTP_HEADER_ACCEPT = "Accept";
const std::string HTTP_HEADER_CONTENT_LENGTH = "Content-Length";
const std::string HTTP_HEADER_CONTENT_ENCODING = "Content-Encoding";
const std::string HTTP_HEADER_CONTENT_TYPE = "Content-Type";
const std::string HTTP_DEFAULT_CONTENT_TYPE = "text/plain";
const std::string HTTP_METHOD_GET = "GET";
const std::string HTTP_METHOD_POST = "POST";
const std::string HTTP_CRLF = "\r\n";
const uint64_t HTTP_MAX_REQUEST_SIZE = 1024*1024*8;

struct CaseInsensitiveLess 
{
    bool operator()(const std::string &s1, const std::string &s2) const;
};

typedef std::map<std::string, std::string, CaseInsensitiveLess> 
    HttpHeaderMapType;
typedef std::map<std::string, std::vector<std::string> > HttpQueryMapType;
typedef std::map<std::string, std::vector<std::string> > HttpCookieMapType;

class HttpSessionState 
{
  public:
    HttpSessionState() {}
    virtual ~HttpSessionState() {}
};

struct HttpSession 
{
    typedef std::map<std::string, HttpSessionState *> HttpSessionStateMap;
    HttpSessionStateMap state;
    time_t lastAccess;
    std::string sessionId;
};

struct HttpRequest 
{
    std::string method;
    std::string url;
    std::string protocolVersion;
    HttpHeaderMapType headerMap;
    HttpCookieMapType cookieMap;
    HttpQueryMapType queryMap;
    std::string body;
};

enum HttpResponseCode 
{
    HTTP_OK = 200,
    HTTP_PARTIAL_CONTENT = 206,
    HTTP_MOVED_PERMANENTLY = 301,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_SERVICE_UNAVAILABLE = 503,
};

struct HttpResponse 
{
    HttpResponseCode responseCode;
    HttpHeaderMapType headerMap;
    std::string body;
};

struct HttpContext 
{
    std::string client;
    HttpRequest request;
    HttpResponse response;
    HttpSession *session;
};

class HttpServerIncludeHandler 
{
  public:
    virtual ~HttpServerIncludeHandler() {}

    virtual void includeHandler(
        const std::string &reference, HttpContext *context) = 0;
};

class HttpServerPageHandler 
{
  public:
    virtual ~HttpServerPageHandler() {}

    virtual void pageHandler(
        const std::string &path, HttpContext *context) = 0;
};

class HttpServerAccessHandler 
{
  public:
    enum AccessPermission 
    {
        ALLOW,
        DENY,
        NEXT
    };

    virtual ~HttpServerAccessHandler() {}

    virtual AccessPermission canAccess(const HttpContext &context) = 0;
};

class HttpServerException 
    : public virtual std::exception 
{
  public:
    /**
     * Creates an instance of HttpServerException with error message.
     * @param message the error message.
     */
    explicit HttpServerException(const std::string &message);
    /**
     * Gets the error message of this exception.
     * @return the error message.
     */
    virtual const char *what() const throw();
    /**
     * Destroys the instance of HttpServerException.
     */
    virtual ~HttpServerException() throw();
  private:
    /**
     * Represents the error message.
     */
    std::string message;
};

class HttpServer 
{
  public:
    virtual ~HttpServer();
    
    HttpServer(
        uint16_t port, const std::string &rootDirectory, bool ipv6 = false);
    
    virtual uint16_t getPort() const;
    
    virtual const std::string &getRootDirectory() const;
    
    virtual bool isIPv6() const;
    
    virtual void start() = 0;
    
    virtual void stop() = 0;
    
    virtual void registerContentType(
        const std::string &extension, const std::string &type);
    
    virtual void registerVirtualDirectory(
        const std::string &path, const std::string &directory);
    
    virtual void registerIncludeHandler(
        const std::string &ref, HttpServerIncludeHandler *handler);
    
    virtual void registerPageHandler(
        const std::string &path, HttpServerPageHandler *handler);
    
    virtual void registerAccessHandler(HttpServerAccessHandler *handler);
    
    virtual void setSessionLife(uint32_t sessionLife);

  protected:
    void cleanExpiredSession();
    void generateSession(const std::string &sessionId, HttpContext *context);
    void getSession(HttpContext *context);
    
    bool ipv6;
    uint16_t port;
    uint32_t sessionLife;
    std::list<HttpServerAccessHandler*> accessHandlers;
    std::string rootDirectory;
    std::map<std::string, std::string, CaseInsensitiveLess> contentTypeMap;
    std::map<std::string, std::string> virtualDirectoryMap;
    std::map<std::string, HttpServerIncludeHandler*> includeHandlerMap;
    std::map<std::string, HttpServerPageHandler*> pageHandlerMap;
    std::map<std::string, HttpSession*> sessionMap;
};

}

#endif
