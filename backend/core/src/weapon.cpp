#include "weapon.h"
#include "utils.h"
#include "logger.h"
#include "commands.h"
#include "scheduler.h"
#include "defines.h"

#include <chrono>
using namespace std::chrono;

Weapon::Weapon(json json, unsigned int ID) : ID(ID)
{
	log("Creating weapon with ID: " + to_string(ID));
}

Weapon::~Weapon()
{
}

void Weapon::initialize(json json)
{
	if (json_has_string_field(json, "name"))
		setName(to_string(json["name"]));

	if (json_has_number_field(json, "coalitionID"))
		setCoalition(json["coalitionID"].template get<int32_t>());

	update(json, 0);
}

void Weapon::update(json json, double dt)
{
	if (json_has_object_field(json, "position"))
	{
		setPosition({json["position"]["lat"].template get<double>(),
					 json["position"]["lng"].template get<double>(),
					 json["position"]["alt"].template get<double>()});
	}

	if (json_has_number_field(json, "heading"))
		setHeading(json["heading"].template get<double>());

	if (json_has_number_field(json, "speed"))
		setSpeed(json["speed"].template get<double>());

	if (json_has_boolean_field(json, "isAlive"))
		setAlive(json["isAlive"].template get<bool>());
}

bool Weapon::checkFreshness(unsigned char datumIndex, unsigned long long time)
{
	auto it = updateTimeMap.find(datumIndex);
	if (it == updateTimeMap.end())
		return false;
	else
		return it->second > time;
}

bool Weapon::hasFreshData(unsigned long long time)
{
	for (auto it : updateTimeMap)
		if (it.second > time)
			return true;
	return false;
}

void Weapon::getData(stringstream &ss, unsigned long long time)
{
	const unsigned char endOfData = DataIndex::endOfData;
	ss.write((const char *)&ID, sizeof(ID));
	if (!alive && time == 0)
	{
		unsigned char datumIndex = DataIndex::category;
		appendString(ss, datumIndex, category);
		datumIndex = DataIndex::alive;
		appendNumeric(ss, datumIndex, alive);
	}
	else
	{
		for (unsigned char datumIndex = DataIndex::startOfData + 1; datumIndex < DataIndex::lastIndex; datumIndex++)
		{
			if (checkFreshness(datumIndex, time))
			{
				switch (datumIndex)
				{
				case DataIndex::category:
					appendString(ss, datumIndex, category);
					break;
				case DataIndex::alive:
					appendNumeric(ss, datumIndex, alive);
					break;
				case DataIndex::coalition:
					appendNumeric(ss, datumIndex, coalition);
					break;
				case DataIndex::name:
					appendString(ss, datumIndex, name);
					break;
				case DataIndex::position:
					appendNumeric(ss, datumIndex, position);
					break;
				case DataIndex::speed:
					appendNumeric(ss, datumIndex, speed);
					break;
				case DataIndex::heading:
					appendNumeric(ss, datumIndex, heading);
					break;
				}
			}
		}
	}
	ss.write((const char *)&endOfData, sizeof(endOfData));
}

void Weapon::triggerUpdate(unsigned char datumIndex)
{
	updateTimeMap[datumIndex] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

/* Missile */
Missile::Missile(json json, unsigned int ID) : Weapon(json, ID)
{
	log("New Missile created with ID: " + to_string(ID));
	setCategory("Missile");
};

/* Bomb */
Bomb::Bomb(json json, unsigned int ID) : Weapon(json, ID)
{
	log("New Bomb created with ID: " + to_string(ID));
	setCategory("Bomb");
};