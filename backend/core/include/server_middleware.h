#pragma once

#include "framework.h"
#include "luatools.h"
#include "crow.h"

enum AuthRole
{
    Guest = 0,
    BlueCommander = 1,
    RedCommander = 2,
    GameMaster = 3,
};

struct AuthMiddleware : crow::ILocalMiddleware
{
    struct context
    {
        bool hasAuth = false;
        bool isAuth = false;
        string username = "";
        string password = "";
        AuthRole role = AuthRole::Guest;
    };

    std::string gameMasterPassword = "";
    std::string blueCommanderPassword = "";
    std::string redCommanderPassword = "";

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx) {}
};

struct AuthRequiredMiddleware : crow::ILocalMiddleware
{
    struct context
    {
    };

    template <typename AllContext>
    void before_handle(crow::request& req, crow::response& res, context& ctx, AllContext& all_ctx)
    {
        auto& ctxAuth = all_ctx.template get<AuthMiddleware>();
        if (!ctxAuth.isAuth)
        {
            res.code = crow::status::UNAUTHORIZED;
            res.end();
        }
    }
    void after_handle(crow::request& req, crow::response& res, context& ctx) {}
};

// might should migrate to crow own logger / debugger
struct LogMiddleware
{
    struct context
    {
        bool enabled = true; // TODO default=false & make configureable
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx) {}
};
