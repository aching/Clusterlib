#ifndef _INCLUDED_JSONRPC_HTTPD_ADAPTOR_H_
#define _INCLUDED_JSONRPC_HTTPD_ADAPTOR_H_
#include "jsonrpc.h"
#include "httpd.h"
#include <log4cxx/logger.h>

namespace json { namespace rpc {
    class HttpServerAdaptor : 
        public virtual httpd::HttpServerPageHandler {
    public:
        HttpServerAdaptor(httpd::HttpServer *server, JSONRPCManager *manager);
        ~HttpServerAdaptor();
        void pageHandler(const std::string &path, httpd::HttpContext *context);
    private:
        httpd::HttpServer *server;
        JSONRPCManager *manager;
        static log4cxx::LoggerPtr logger;

        class HttpSessionPersistableState : public virtual httpd::HttpSessionState {
        public:
            HttpSessionPersistableState(PersistableState *state);
            ~HttpSessionPersistableState();
            PersistableState *get();
        private:
            PersistableState *state;
        };

        class HttpSessionStatePersistence : public virtual StatePersistence {
        public:
            HttpSessionStatePersistence(httpd::HttpSession *session);
            ~HttpSessionStatePersistence();
            PersistableState *get(const std::string &name);
            void set(const std::string &name, PersistableState *state);
            void erase(const std::string &name);
        private:
            httpd::HttpSession *session;
        };
    };
}}

#endif
