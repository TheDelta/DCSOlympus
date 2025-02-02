#pragma once
#include "framework.h"

#pragma pack(push, 1)
struct Coords {
    double lat = 0;
    double lng = 0;
    double alt = 0;
};

struct Offset {
    double x = 0;
    double y = 0;
    double z = 0;
};
#pragma pack(pop)

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const DllExport std::string CurrentDateTime();
std::wstring DllExport to_wstring(const std::string& str);
std::string DllExport to_string(json& value);
std::string DllExport to_string(const std::wstring& wstr);
std::string DllExport random_string(size_t length);

bool DllExport operator== (const Coords& a, const Coords& b);
bool DllExport operator!= (const Coords& a, const Coords& b);
bool DllExport operator== (const Coords& a, const double& b);
bool DllExport operator!= (const Coords& a, const double& b);

bool DllExport operator== (const Offset& a, const Offset& b);
bool DllExport operator!= (const Offset& a, const Offset& b);
bool DllExport operator== (const Offset& a, const double& b);
bool DllExport operator!= (const Offset& a, const double& b);

double DllExport knotsToMs(const double knots);
double DllExport msToKnots(const double ms);
double DllExport ftToM(const double ft);
double DllExport mToFt(const double m);

bool DllExport json_has_boolean_field(json obj, std::string field);
bool DllExport json_has_string_field(json obj, std::string field);
bool DllExport json_has_number_field(json obj, std::string field);
bool DllExport json_has_object_field(json obj, std::string field);
bool DllExport json_has_array_field(json obj, std::string field);
