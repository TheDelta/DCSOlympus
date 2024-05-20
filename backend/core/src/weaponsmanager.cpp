#include "framework.h"
#include "weaponsManager.h"
#include "logger.h"
#include "weapon.h"
#include "scheduler.h"

#include "base64.hpp"
using namespace base64;

WeaponsManager::WeaponsManager(lua_State *L)
{
	LogInfo(L, "Weapons Manager constructor called successfully");
}

WeaponsManager::~WeaponsManager()
{
}

Weapon *WeaponsManager::getWeapon(unsigned int ID)
{
	if (weapons.find(ID) == weapons.end())
	{
		return nullptr;
	}
	else
	{
		return weapons[ID];
	}
}

void WeaponsManager::update(json &data, double dt)
{

	for (json::iterator it = data.begin(); it != data.end(); ++it)
	{
		unsigned int ID = std::stoi(it.key());
		if (weapons.count(ID) == 0)
		{
			json value = it.value();
			if (json_has_string_field(value, "category"))
			{
				string category = value["category"].template get<string>();
				if (category.compare("Missile") == 0)
					weapons[ID] = dynamic_cast<Weapon *>(new Missile(it.value(), ID));
				else if (category.compare("Bomb") == 0)
					weapons[ID] = dynamic_cast<Weapon *>(new Bomb(it.value(), ID));

				/* Initialize the weapon if creation was successfull */
				if (weapons.count(ID) != 0)
				{
					weapons[ID]->update(it.value(), dt);
					weapons[ID]->initialize(it.value());
				}
			}
		}
		else
		{
			/* Update the weapon if present*/
			if (weapons.count(ID) != 0)
				weapons[ID]->update(it.value(), dt);
		}
	}
}

void WeaponsManager::getWeaponData(stringstream &ss, unsigned long long time)
{
	for (auto const &p : weapons)
		p.second->getData(ss, time);
}
