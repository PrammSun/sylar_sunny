#ifndef __SYLAR_LOG_H__
#define __SYLAR__LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<stringstreams>
#include<fstream>
#include<vector>
class Logger;
namespace sylar{
//��־�¼�
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    const char* getFile() const { return m_file; }
    int32_t getLine()const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint32_t getThreadId() const { return m_threadId; }
    uint64_t getTime() const { return m_time; }
    const std::string& getContent() const { return m_content; }

    // ~LogEvent(){}
private:
    const char* m_file = nullptr; //�ļ���
    int32_t m_line = 0; //�к�
    uint32_t m_elapse = 0; //����������ʼ�����ڵĺ�����
    uint32_t m_threadId = 0; //�߳�id
    uint32_t m_fiberId = 0; //Э��id
    uint64_t m_time; //ʱ���
    std::string m_content;


};

// ��־����
class LogLevel{
public:
    enum Level{
        UNKNOW = 0,
        DEBUG=1,
        INFO=2,
        WARN=3,
        ERROR=4,
        FATAL=5
    };

    static const char* ToString(LogLevel::Level);
};

//��־��ʽ��
class LogFamtter {
public:
    typedef std::shared_ptr<LogFamtter> ptr;
    LogFamtter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    void init();
private:
    class FormatItem {
    public:
        typedef shared_ptr<FormatItem> ptr;
        FormatItem(const std::string& fmt = ""){}
        virtual ~FormatItem(){}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
private:
    std::string m_pattern;
    std::vector<formatItem::ptr> m_items;
};

//��־�����
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent:ptr event)=0; //���麯��

    void setFormatter(LogFamtter::ptr val) {
        m_formatter = val;
    }
    LogFamtter::ptr getFormatter()const { return m_formatter; }
private:
    LogLevel::Level m_level;
    LogFamtter::ptr m_formatter;
};

//��־��
class Logger{
public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name="root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void error(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    const std::string getName() const { return m_name; }
private:
    std::string m_name; //��־����
    LogLevel::Level m_level; //��־����
    std::list<LogAppender::ptr> m_appenders //Appender����
};

//���������̨��appender
class StdoutLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    // StdoutLogAppender();
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent:ptr event) override;

private:

};

//����������ļ���Appender
class FileLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    //override��ȷ�ر�ʾһ�������ǶԻ�����һ���麯�������ء�����Ҫ���ǣ�
    //����������麯���������������غ�����ǩ����ƥ�����⡣���ǩ����ƥ�䣬
    //�������ᷢ��������Ϣ��
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent:ptr event) override;
    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;
};
}

#endif