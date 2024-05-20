#pragma once
#include "framework.h"
#include "luatools.h"
#include "crow.h"
#include "crow/middlewares/cors.h"

#include "server_middleware.h"

class UnitsManager;
class Scheduler;

class Server
{
public:
    Server(lua_State* L) {};

    void start(lua_State *L);
    void stop(lua_State *L);

private:
    crow::App<LogMiddleware, crow::CORSHandler, AuthMiddleware, AuthRequiredMiddleware> app;
    std::future<void> serverJob;

    void initWebServer();

    crow::response handle_put(const crow::request &req);
    crow::response handle_get_units(const crow::request& req);
    crow::response handle_get_weapons(const crow::request& req);
    crow::response handle_get_logs(const crow::request& req);
    crow::response handle_get_airbases(const crow::request& req);
    crow::response handle_get_bullseyes(const crow::request& req);
    crow::response handle_get_mission(const crow::request& req);
    crow::response handle_get_command(const crow::request& req);

    crow::response create_general_response(json& data, const std::chrono::milliseconds ms);
    crow::response handle_eptr(std::exception_ptr eptr);
    unsigned long long extract_reference_time(const crow::request& req);
};
