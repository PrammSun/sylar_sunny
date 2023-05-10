#include"log.h"
#include<map>

namespace sylar{
Logger::Logger(const std::string& name = "root")
	:m_name(name) {

}
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
	if (level >= m_level) {
		for (auto& i : m_appenders) {
			i->log(level, event);
		}
	}
}

void Logger::debug(LogEvent::ptr event) {
	debug(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
	debug(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
	debug(LogLevel::WARN, event);
}
void Logger::fatal(LogEvent::ptr event) {
	debug(LogLevel::FATAL, event);
}
void Logger::error(LogEvent::ptr event) {
	debug(LogLevel::ERROR, event);
}

void Logger::addAppender(LogAppender::ptr appender){
	m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
	for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
		if (*it == appender) {
			m_appenders.erase(it);
			break;
		}
	}
}


void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent : ptr event)
{
	if (level >= m_level) {
		std::count << m_formattter.format(logger, level, event);
	}

}

FileLogAppender::FileLogAppender(const std::string& filename):
	m_filename(filename){

}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent : ptr event)
{
	if (level >= m_level) {
		m_filestream << m_formatter.format(logger, level, event);
	}
}

//
bool FileLogAppender::reopen()
{
	if (m_filestream) {
		m_filestram.close();
	}
	m_filestream.open(m_filename);
	return !!m_filestream;
}

LogFamtter::LogFamtter(const std::string& pattern)
	:m_pattern(pattern) {

}

std::string LogFamtter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
	std::stringstream ss;
	for (auto& i : m_items) {
		i->format(ss,logger, level, event);
	}
	return ss.str();
}
void LogFamtter::init() {
	//str, format, type
	std::vector<std::tuple<std::string,std::string, int>> vec;
	std::string nstr;
	size_t last_pos = 0;
	for (size_t i = 0; i < m_pattern.size(); ++i) {
		
		if (m_pattern[i] != '%') {
			nstr.append(i, m_pattern[i]);
			continue;
		}
		if (i + 1 < m_pattern.size()) {
			if (m_pattern[i + 1] == '%') {
				nstr.append(1, '%');
				continue;
			}
		}
		size_t n = i + 1;
		size_t fmt_begin = 0;
		//0, 1, 2三种状态
		int fmt_status = 0;

		std::string str;
		std::string fmt;
		while (n < m_pattern.size()) {
			if (isspace(m_pattern[n])) {
				//不连续了，退出
				break;
			}
			if (fmt_status == 0) {
				if (m_pattern[n] == '{') {
					//
					str = m_pattern.substr(i + 1, n - i);
					//解析格式
					fmt_status = 1;
					fmt_begin = n;
					++n;
					continue;
				}
			}
			if (fmt_status == 1) {
				if (m_pattern[n] == '}') {
					fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin-1);
					fmt_status = 2;
					++n;
					continue;
				}
			}
		}
		if (fmt_status == 0) {
			if (nstr.empty()) {
				vec.push_back_back(std::make_pair(nstr, "", 0));
			}
			str = m_pattern.substr(i + 1, n - i - 1);
			vec.push_back(std::make_tuple(str, fmt, 1));
			i = n;
		}
		else if (fmt_status == 1) {
			std::count << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i)
				<< std::endl;
			vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));

		}
		else if (fmt_status == 2) {
			if (nstr.empty()) {
				vec.push_back_back(std::make_pair(nstr, "", 0));
			}
			vec.push_back(std::make_tuple(str, fmt, 1));
			i = n;
		}

	}
	if (!nstr.empty()) {
		vec.push_back(std::make_pair(nstr, "", 0));
	}
	//%m -- 消息体
	//%p -- level
	//%r -- 启动后时间
	//%c -- 日志名称
	//%t -- 线程id
	//%n -- 回车换行
	//%d -- 时间
	//%f -- 文件名
	//%l -- 行号
	static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
	(#str, [](const std::string& fmt){return FormatItem::ptr(new C(fmt)); })

	XX(m, MessageFormatItem),
	XX(p, LevelFormatItem),
	XX(r, ElapseFormatItem),
	XX(c, NameFormatItem),
	XX(t, ThreadIdFormatItem),
	XX(n, NewLineFormatItem),
	XX(d, DateTimeFormatItem),
	XX(f, FilenameFormatItem),
	XX(l, LineFormatItem);
#undef xx
	};

	for (auto& i : vec) {
		if (std::get<2>(i) == 0) {
			m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i)));

		}
		else {
			auto it = s_format_items.find(std::get<0>(i));
			if (it == s_format_items.end()) {
				m_items.push_back(FormatItem::ptr(new StringFormarItem("<<error_format %" + std::get<0>(i) + ">>")));
			}
			else {
				m_items.push_back(it->second(std::get<1>(i)));
			}
		}
		std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) << std::endl;
	}
}

//-----------------------formatItem-----------------------------
class MessageFormatItem:public LogFamtter:: FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getContent();
	}
};

class LevelFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		 os << LogLevel::ToString(level);
	}
};
class ElapseFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getElapse();
	}
};
class ThreadIdFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getThreadId();
	}
};
class NameFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << logger->getName();
	}
};

class FiberIdFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getFiberId();
	}
};

class DateTimeFormatItem :public LogFamtter::FormatItem {
public:
	DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%S"):m_format(format){}
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getTime();
	}
private:
	std::string m_format;
};

class FilenameFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getFile();
	}
};

class LineFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->getLine();
	}
};

class NewLineFormatItem :public LogFamtter::FormatItem {
public:
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << std::endl;
	}
};

class StringFormatItem :public LogFamtter::FormatItem {
public:
	StringFormatItem(std::string& str): FormatItem(str),  m_string(str){}
	void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << m_string;
	}
private:
	std::string m_string;
};
//-------------------------------------------------------------------------------

const char* LogLevel::ToString(LogLevel::Level)
{
	switch (Level)
	{
	#define XX(name) \
		case LogLevel::name: \
			return #name; \
			break; \

		XX(DEBUG);
		XX(INFO);
		XX(WARN);
		XX(FATAL);
		XX(ERROR);
	#undef XX;
	default:
		return "UNKNOW";
		break;
	}
	return "UNKNOW";
}

}