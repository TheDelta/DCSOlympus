#include "server_middleware.h"

#include "logger.h"
#include "base64.hpp"

using namespace base64;

void AuthMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx)
{
    auto it = req.headers.find("Authorization");
    if (it == req.headers.end())
        return;

    string authorization = it->second;
    string s = "Basic ";
    string::size_type i = authorization.find(s);
    if (i == std::string::npos)
        return;

    authorization.erase(i, s.length());

    string decoded = from_base64(authorization);
    i = decoded.find(":");
    if (i == std::string::npos)
        return;

    ctx.hasAuth = true;
    if (i <= decoded.length())
        ctx.username = decoded.substr(0, i);

    if (i + 1 < decoded.length())
        ctx.password = decoded.substr(i + 1);

    if (ctx.password.length() > 0)
    {
        // TODO should one password allow to grant all roles?
        if (gameMasterPassword.compare(ctx.password) == 0)
        {
            ctx.role = AuthRole::GameMaster;
        }
        else if (blueCommanderPassword.compare(ctx.password) == 0)
        {
            ctx.role = AuthRole::BlueCommander;
        }
        else if (redCommanderPassword.compare(ctx.password) == 0)
        {
            ctx.role = AuthRole::RedCommander;
        }

        ctx.isAuth = ctx.role != AuthRole::Guest;
    }
}

void LogMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    if (!ctx.enabled) {
        return;
    }

    log(std::format("Request [{}] {}", crow::method_name(req.method), req.url));
}