#ifndef	DEBUG_H
#define DEBUG_H

#include <string>
#include <vector>
#include "cinder/gl/TextureFont.h"
#include "cinder/Vector.h"

using namespace ci;

#define DBG(text, value) Debug::get().print(text, value)
#define DBG_REMOVE(text) Debug::get().remove(text)
#define FONT_SCALE 0.5f

class Debug
{
public:
	static Debug& get()
	{
		static Debug instance;
		return instance;
	}

	~Debug();

	void print(const std::string& text, const std::string& value = std::string());
	void print(const std::string& text, const int value);
	void print(const std::string& text, float value);
	void print(const std::string& text, bool value);
	void print(const std::string& text, const Vec3f value);
	void print(const std::string& text, const Vec2f value);
	void remove(const std::string& text);
	void draw(const Color& color = Color::black());
	void clear();

private:
	Debug();
	Debug(Debug const&);
	void operator=(Debug const&);

	void update(const std::string& text, const std::string& value);
	/*void clear();*/

	std::vector<std::pair<std::string, std::string>> m_textMap;
	gl::TextureFontRef	m_Font;
};

Debug::Debug()
{
	gl::TextureFont::Format format;
	format.textureWidth(512).textureHeight(512);
	Font font = Font("consolas", 36);
	m_Font = gl::TextureFont::create(font, format);
}

Debug::~Debug()
{
	m_textMap.clear();
}

void Debug::print(const std::string& text, const std::string& value)
{
	update(text, value);
}

void Debug::print(const std::string& text, const int value)
{
	std::string val = std::to_string(value);
	update(text, val);
}

void Debug::print(const std::string& text, float value)
{
	std::string val = std::to_string(value);
	update(text, val);
}

void Debug::print(const std::string& text, bool value)
{
	std::string val = (value ? "true" : "false");
	update(text, val);
}

void Debug::print(const std::string& text, const ci::Vec3f value)
{
	std::string val = "x: " + std::to_string(value.x) + " y: " + std::to_string(value.y) + " z: " + std::to_string(value.z);
	update(text, val);
}

void Debug::print(const std::string& text, const ci::Vec2f value)
{
	std::string val = "x: " + std::to_string(value.x) + " y: " + std::to_string(value.y);
	update(text, val);
}

void Debug::remove(const std::string& text)
{
	int idx = -1;
	int tmpidx = 0;

	for(auto& t : m_textMap)
	{
		if(t.first == text)
		{
			idx = tmpidx;
			break;
		}

		tmpidx++;
	}

	if(idx != -1)
	{
		m_textMap.erase(m_textMap.begin() + idx);
	}
}

void Debug::update(const std::string& text, const std::string& value)
{
	bool found = false;

	for(auto& t : m_textMap)
	{
		if(t.first == text)
		{
			t.second = value;
			found = true;
			break;
		}
	}

	if(!found)
	{
		m_textMap.push_back(std::make_pair(text, value));
	}
}

void Debug::draw(const Color& color)
{
	gl::enableAlphaBlending();
	gl::color(color);

	unsigned int x = 10;
	float fontSize = m_Font->getAscent() * FONT_SCALE;;
	unsigned int y = static_cast<int>(fontSize)+10;
	size_t maxChars = 0;

	for(auto& t : m_textMap)
	{
		maxChars = std::max<size_t>(maxChars, t.first.length());
	}

	for(auto& t : m_textMap)
	{
		std::string text = t.first;

		if(!t.second.empty())
		{
			text.append(maxChars - text.length(), ' ');
			text += " : " + t.second;
		}

		m_Font->drawString(text, Vec2f(x, y), gl::TextureFont::DrawOptions().pixelSnap(false).scale(FONT_SCALE));
		y += fontSize + m_Font->getDescent() * FONT_SCALE;
	}

	gl::disableAlphaBlending();
}

void Debug::clear()
{
	m_textMap.clear();
}

#endif