#ifndef __BOARD_H__
#define	__BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include "stream.h"
#include "session.h"

namespace fb { namespace game {

class board
{
public:
#pragma region item class
	class article
	{
	private:
		uint16_t			_id;
		std::string			_title;
		std::string			_content;
		std::string			_owner;
		uint8_t				_month, _day;
		uint8_t				_color;

	public:
		article(uint16_t id, const std::string& title, const std::string& content, const std::string& owner, uint8_t month, uint8_t day, uint8_t color = 0x00);
		~article();

	public:
		uint16_t			id() const;
		const std::string&	title() const;
		const std::string&	content() const;
		const std::string&	owner() const;
		uint8_t				month() const;
		uint8_t				day() const;
		uint8_t				color() const;
	};
#pragma endregion

#pragma region section class
	class section
	{
	private:
		std::string			 _title;
		std::vector<article*> _articles;

	public:
		section(const std::string& title);
		~section();

	public:
		const std::string&				title() const;
		const std::vector<article*>&	articles() const;
		article*						add(uint16_t id, const std::string& title, const std::string& content, const session& session, uint8_t color = 0x00);
		article*						add(uint16_t id, const std::string& title, const std::string& content, const std::string& owner, uint8_t color = 0x00);
	};
#pragma endregion

private:
	std::vector<section*>	_sections;

public:
	board();
	~board();

public:
	const std::vector<section*>&	sections() const;
	section*						at(uint32_t index);

	section*						add(const std::string& name);
	void							remove(uint32_t index);

public:
	section*						operator [] (uint32_t index);

public:
	ostream							make_sections_stream() const;
	ostream							make_articles_stream(uint16_t section_id, uint16_t offset) const;
};

} }

#endif // !__BOARD_H__
