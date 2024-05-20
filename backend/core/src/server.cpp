#include "server.h"
#include "logger.h"
#include "defines.h"
#include "unitsManager.h"
#include "weaponsManager.h"
#include "scheduler.h"
#include "luatools.h"
#include <exception>
#include <stdexcept>
#include <chrono>

using namespace std::chrono;

extern UnitsManager *unitsManager;
extern WeaponsManager *weaponsManager;
extern Scheduler *scheduler;

extern json missionData;
extern mutex mutexLock;
extern string sessionHash;
extern string instancePath;

void Server::start(lua_State *L)
{
    log("Starting RESTServer");
    initWebServer();
}

void Server::stop(lua_State *L)
{
    log("Stopping RESTServer");
    app.stop();
    serverJob.wait_for(std::chrono::milliseconds(30'000));
    log("RESTServer stopped");
}

crow::response Server::handle_put(const crow::request &req)
{
    crow::response response = crow::response(crow::OK);

    // TODO: limit what a user can do depending on the role
    auto &ctxAuth = app.get_context<AuthMiddleware>(req);

    auto answer = json::object();
    auto res = json::parse(req.body);
    if (res.is_object())
    {
        lock_guard<mutex> guard(mutexLock);

        for (auto const &e : res.items())
        {
            try
            {
                scheduler->handleRequest(e.key(), e.value(), ctxAuth.username, answer);
            }
            catch (...)
            {
                handle_eptr(std::current_exception());
            }
        }
    }

    response.body = answer.dump();

    return response;
}


/* If present, extract the request reference time. This is used for updates, and it specifies the last time that request has been performed */
unsigned long long Server::extract_reference_time(const crow::request& req) {
    if (req.url_params.get("time") == nullptr)
    {
        return 0;
    }

    try
    {
        return stoull(req.url_params.get("time"));
    }
    catch (...)
    {
        return 0;
    }
}

crow::response Server::handle_get_logs(const crow::request& req)
{
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    try
    {
        auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto data = json::object();
        auto time = extract_reference_time(req);

        auto logs = json::object();
        getLogsJSON(logs, time);
        data["logs"] = logs;

        return create_general_response(data, ms);
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

crow::response Server::handle_get_airbases(const crow::request& req)
{
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    try
    {
        auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto data = json::object();

        auto key = "airbases";
        if (json_has_object_field(missionData, key)) {
            data[key] = missionData[key];
        }

        return create_general_response(data, ms);
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

crow::response Server::handle_get_bullseyes(const crow::request& req)
{
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    try
    {
        auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto data = json::object();

        auto key = "bullseyes";
        if (json_has_object_field(missionData, key)) {
            data[key] = missionData[key];
        }

        return create_general_response(data, ms);
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

crow::response Server::handle_get_mission(const crow::request& req) {
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    try
    {
        auto& ctxAuth = app.get_context<AuthMiddleware>(req);
        auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto data = json::object();

        data["mission"] = missionData["mission"];
        data["mission"]["commandModeOptions"] = scheduler->getCommandModeOptions();

        string commandMode;
        switch (ctxAuth.role)
        {
        case GameMaster:
            commandMode = "Game master";
            break;
        case BlueCommander:
            commandMode = "Blue commander";
            break;
        case RedCommander:
            commandMode = "Red commander";
            break;
        default:
            commandMode = "Observer";
            break;
        }
        data["mission"]["commandModeOptions"]["commandMode"] = commandMode;

        return create_general_response(data, ms);
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

crow::response Server::handle_get_command(const crow::request& req) {
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    try
    {
        auto& ctxAuth = app.get_context<AuthMiddleware>(req);
        auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto data = json::object();

        if (req.url_params.get("commandHash") == nullptr) {
            auto res = crow::response(crow::status::BAD_REQUEST);

            auto error = json::object();
            error["error"] = "Missing commandHash parameter!";
            res.body = error.dump();

            return res;
        }

        data["commandExecuted"] = scheduler->isCommandExecuted(req.url_params.get("commandHash"));

        return create_general_response(data, ms);
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}


crow::response Server::create_general_response(json& data, const std::chrono::milliseconds ms) {
    auto res = crow::response(crow::OK);

    /* Common data */
    data["time"] = json::string_t(to_string(ms.count()));
    data["sessionHash"] = json::string_t(sessionHash);
    data["load"] = scheduler->getLoad();
    data["frameRate"] = scheduler->getFrameRate();

    res.body = data.dump();

    log("General response: " + res.body);

    return res;
}

crow::response Server::handle_get_units(const crow::request &req)
{
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    
    try
    {
        auto response = crow::response(crow::OK);

        auto time = extract_reference_time(req);
        unsigned long long updateTime = ms.count();

        stringstream ss;
        ss.write((char*)&updateTime, sizeof(updateTime));
        unitsManager->getUnitData(ss, time);
        response.body = ss.str();

        log("Unit response: " + response.body);

        return response;
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

crow::response Server::handle_get_weapons(const crow::request& req)
{
    /* Lock for thread safety */
    lock_guard<mutex> guard(mutexLock);

    auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    try
    {
        auto response = crow::response(crow::OK);

        auto time = extract_reference_time(req);
        unsigned long long updateTime = ms.count();

        stringstream ss;
        ss.write((char*)&updateTime, sizeof(updateTime));
        weaponsManager->getWeaponData(ss, time);
        response.body = ss.str();

        log("Weapons response: " + response.body);

        return response;
    }
    catch (...)
    {
        return handle_eptr(std::current_exception());
    }
}

void Server::initWebServer()
{
    string jsonLocation = instancePath + OLYMPUS_JSON_PATH;

    log("Reading configuration from " + jsonLocation);

    std::ifstream ifs(jsonLocation);
    json config = json::parse(ifs);

    if (!config.is_object())
    {
        log("Error reading configuration file. Ensure file exists and is valid JSON!");
        return;
    }

    std::string configKey = "server"; // this is legacy (main), rc uses now "backend"
    if (!json_has_object_field(config, configKey)) {
        configKey = "backend";
    }
    else {
        // TODO log out a warning or convert to new key?!
    }

    if (!json_has_object_field(config, configKey) || !json_has_string_field(config[configKey], "address") || !json_has_number_field(config[configKey], "port"))
    {
        log("Error missing " + configKey + ".address and / or " + configKey + ".port");
        return;
    }

    try
    {
        string address = config[configKey]["address"].template get<string>();
        uint16_t port = config[configKey]["port"].template get<uint16_t>();

        // Setup Auth
        auto &authMiddleware = app.get_middleware<AuthMiddleware>();
        if (json_has_object_field(config, "authentication"))
        {
            if (json_has_string_field(config["authentication"], "gameMasterPassword"))
                authMiddleware.gameMasterPassword = to_string(config["authentication"]["gameMasterPassword"]);
            if (json_has_string_field(config["authentication"], "blueCommanderPassword"))
                authMiddleware.blueCommanderPassword = to_string(config["authentication"]["blueCommanderPassword"]);
            if (json_has_string_field(config["authentication"], "redCommanderPassword"))
                authMiddleware.redCommanderPassword = to_string(config["authentication"]["redCommanderPassword"]);
        }

        // Setup CORS
        auto& cors = app.get_middleware<crow::CORSHandler>();
        cors
            .global()
            .origin("*")
            .allow_credentials()
            .headers(
                "Accept",
                "Origin",
                "Content-Type",
                "Authorization",
                "Refresh")
            .methods(
                crow::HTTPMethod::Get,
                crow::HTTPMethod::Post,
                crow::HTTPMethod::Options,
                crow::HTTPMethod::Head,
                crow::HTTPMethod::Put,
                crow::HTTPMethod::Delete);
        //.max_age(120); won't work, headers are lost, SEE https://github.com/CrowCpp/Crow/issues/721

        CROW_ROUTE(app, "/")
            .CROW_MIDDLEWARES(app, AuthMiddleware)
        ([]()
         { return "{\"running\": true}"; });

        CROW_ROUTE(app, "/olympus")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Put)([this](const crow::request &req) { return handle_put(req); });
        CROW_ROUTE(app, "/olympus/units")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_units(req); });
        CROW_ROUTE(app, "/olympus/weapons")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_weapons(req); });
        CROW_ROUTE(app, "/olympus/logs")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_logs(req); });
        CROW_ROUTE(app, "/olympus/airbases")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_airbases(req); });
        CROW_ROUTE(app, "/olympus/bullseyes")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_bullseyes(req); });
        CROW_ROUTE(app, "/olympus/mission")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_mission(req); });
        CROW_ROUTE(app, "/olympus/commands")
            .CROW_MIDDLEWARES(app, AuthMiddleware, AuthRequiredMiddleware)
            .methods(crow::HTTPMethod::Get)([this](const crow::request& req) { return handle_get_command(req); });

        log("Going to start backend on " + address + ":" + to_string(port));
        serverJob = app.bindaddr(address == "localhost" ? "127.0.0.1" : address).port(port).run_async();
    }
    catch (const exception &e)
    {
        log("Error while creating web server: " + std::string(e.what()));
    }
}

crow::response Server::handle_eptr(std::exception_ptr eptr)
{
    std::string error = "Unknown Exception";
    try
    {
        if (eptr)
        {
            std::rethrow_exception(eptr);
        }
    }
    catch (const std::exception& e)
    {
        error = e.what();
        log(e.what());
    }

    auto response = crow::response(crow::status::INTERNAL_SERVER_ERROR);
    auto data = json::object();
    data["error"] = error;
    data["exception"] = true;
    response.body = data.dump();

    return response;
}
