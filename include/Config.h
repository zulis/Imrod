#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <typeinfo>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include "cinder/app/AppNative.h"

using namespace ci::app;

class Config
{
public:
	Config(const std::string& fileName);
	~Config();

	void setSection(const std::string& section);
	std::string getString(const std::string& section, const std::string& value);
	int getInt(const std::string& section, const std::string& value);
	float getFloat(const std::string& section, const std::string& value);
	bool getBool(const std::string& section, const std::string& value);
	ci::Vec3f getVec3f(const std::string& section, const std::string& value);
	std::string getString(const std::string& value);
	int getInt(const std::string& value);
	float getFloat(const std::string& value);
	bool getBool(const std::string& value);
	ci::Vec3f getVec3f(const std::string& value);

private:
	template<typename T>
	bool get(const std::string& section, const std::string& value, T& result);

	boost::property_tree::ptree m_pt;
	std::string m_section;
};

template<typename T>
bool Config::get(const std::string& section, const std::string& value, T& result)
{
	try
	{
		result = m_pt.get<T>(section + "." + value);
		return true;
	}
	catch(const std::exception& e)
	{
		return false;
	}
}

Config::Config(const std::string& fileName)
{
	std::string fullPath = fileName;

	if(!boost::filesystem::exists(boost::filesystem::path(fullPath)))
	{
		fullPath = getAssetPath(fileName).string();
	}

	boost::property_tree::ini_parser::read_ini(fullPath, m_pt);
}

Config::~Config()
{
}

void Config::setSection(const std::string& section)
{
	m_section = section;
}

std::string Config::getString(const std::string& section, const std::string& value)
{
	std::string result;

	if(get<std::string>(section, value, result))
	{
		return result;
	}
	else
	{
		return std::string();
	}
}

std::string Config::getString(const std::string& value)
{
	return getString(m_section, value);
}

int Config::getInt(const std::string& section, const std::string& value)
{
	std::string result;

	if(get<std::string>(section, value, result))
	{
		return ::atoi(result.c_str());
	}
	else
	{
		return 0;
	}
}

int Config::getInt(const std::string& value)
{
	return getInt(m_section, value);
}

float Config::getFloat(const std::string& section, const std::string& value)
{
	std::string result;

	if(get<std::string>(section, value, result))
	{
		return ::atof(result.c_str());
	}
	else
	{
		return 0.0f;
	}
}

float Config::getFloat(const std::string& value)
{
	return getFloat(m_section, value);
}

bool Config::getBool(const std::string& section, const std::string& value)
{
	bool result;

	if(get<bool>(section, value, result))
	{
		return result;
	}
	else
	{
		return false;
	}
}

bool Config::getBool(const std::string& value)
{
	return getBool(m_section, value);
}

ci::Vec3f Config::getVec3f(const std::string& section, const std::string& value)
{
	std::string result;

	if(get<std::string>(section, value, result))
	{
		std::vector<std::string> strs;
		boost::split(strs, result, boost::is_any_of("\t "), boost::token_compress_on);
		float x = ::atof(strs[0].c_str());
		float y = ::atof(strs[1].c_str());
		float z = ::atof(strs[2].c_str());

		return Vec3f(x, y, z);
	}
	else
	{
		return ci::Vec3f();
	}
}

ci::Vec3f Config::getVec3f(const std::string& value)
{
	return getVec3f(m_section, value);
}

#endif // CONFIG_H