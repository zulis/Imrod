#ifndef	DEBUG_H
#define DEBUG_H

#include <string>
#include <vector>
#include "cinder/gl/TextureFont.h"
#include "cinder/Vector.h"

#define DBG(text, value) Debug::get().print(text, value)
#define DBG_REMOVE(text) Debug::get().remove(text)

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
		void print(const std::string& text, const ci::Vec3f value);
		void print(const std::string& text, const ci::Vec2f value);
		void remove(const std::string& text);
		void draw(const ci::Color& color = ci::Color::black());
		void clear();

	private:
		Debug();
		Debug(Debug const&);
		void operator=(Debug const&);

		void update(const std::string& text, const std::string& value);
		/*void clear();*/

		std::vector<std::pair<std::string, std::string>> m_textMap;
		ci::gl::TextureFontRef	m_Font;

};

#endif