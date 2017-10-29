#ifndef __LOG_H__
#define __LOG_H__

#include <sstream>
#include <string>
#include <stdio.h>

// you can comment this out if not using Xcode (you'll just see a bunch of annoying char codes)
// but take note; changing this one line would result in a recompile of the ENTIRE project!!!
#define USE_COLOR


#define LogD4 L_(logDEBUG4)
#define LogD3 L_(logDEBUG3)
#define LogD2 L_(logDEBUG2)
#define LogD1 L_(logDEBUG1)
#define LogD L_(logDEBUG)
#define LogI L_(logINFO)
#define LogW L_(logWARNING)
#define LogE L_(logERROR)


/** Showing Colors in Xcode: Install plugin from: https://github.com/robbiehanson/XcodeColors */
#define XCODE_COLORS_ESCAPE_MAC "\033["
#define XCODE_COLORS_ESCAPE_IOS "\xC2\xA0["

//#if TARGET_OS_IPHONE
//	#define XCODE_COLORS_ESCAPE  XCODE_COLORS_ESCAPE_IOS
//#else
//	#define XCODE_COLORS_ESCAPE  XCODE_COLORS_ESCAPE_MAC
//#endif

#define XCODE_COLORS_ESCAPE  XCODE_COLORS_ESCAPE_MAC

#define XCODE_COLORS_RESET_FG  XCODE_COLORS_ESCAPE "fg;" // Clear any foreground color
#define XCODE_COLORS_RESET_BG  XCODE_COLORS_ESCAPE "bg;" // Clear any background color
#define XCODE_COLORS_RESET     XCODE_COLORS_ESCAPE ";"   // Clear any foreground or background color

// using colors
// #define LogBlue(frmt, ...) NSLog((XCODE_COLORS_ESCAPE @"fg0,0,255;" frmt XCODE_COLORS_RESET), ##__VA_ARGS__)
// std::cout << XCODE_COLORS_ESCAPE << "fg0,0,255;" << "abcd" << XCODE_COLORS_RESET;

inline std::string NowTime();

enum TLogLevel { logOFF, logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};

template <typename T>
class Log
{
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level = logINFO);
public:
    static TLogLevel& ReportingLevel();
    static std::string ToString(TLogLevel level);
	static std::string ColorLeft(TLogLevel level);

    static TLogLevel FromString(const std::string& level);
protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

template <typename T>
Log<T>::Log()
{
}

template <typename T>
std::ostringstream& Log<T>::Get(TLogLevel level)
{

#ifdef USE_COLOR
	os << XCODE_COLORS_ESCAPE << ColorLeft(level);
#else
	// os << NowTime() << " " << ToString(level) << ": ";
	// os << std::string(level > logDEBUG ? level - logDEBUG : 0, '\t');
#endif
	os << ToString(level) << ": ";
    return os;
}

template <typename T>
Log<T>::~Log()
{
#ifdef USE_COLOR
    os << XCODE_COLORS_RESET;
#endif
	os << std::endl;
    T::Output(os.str());
}

template <typename T>
TLogLevel& Log<T>::ReportingLevel()
{
    static TLogLevel reportingLevel = logDEBUG4;
    return reportingLevel;
}

template <typename T>
std::string Log<T>::ToString(TLogLevel level)
{
	// all normalized to 7 spaces, so they are aligned
	static const char* const buffer[] = { "OFF", "ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "DEBUG1 ", "DEBUG2 ", "DEBUG3 ", "DEBUG4 "};
    return buffer[level];
}


template <typename T>
std::string Log<T>::ColorLeft(TLogLevel level)
{
	static const char* const buffer[] = {
		"fg59,70,73;", // off
		"fg208,42,51;", // error
		"fg174,141,0;", // warning
		"fg70,161,151;", // info
		"fg236,233,213;", // debug
		"fg115,104,196;", // d1
		"fg74,133,210;", // d2
		"fg201,33,131;", // d3
		"fg131,158,0;"}; // d4
    return buffer[level];
}


template <typename T>
TLogLevel Log<T>::FromString(const std::string& level)
{
    if (level == "DEBUG4")
        return logDEBUG4;
    if (level == "DEBUG3")
        return logDEBUG3;
    if (level == "DEBUG2")
        return logDEBUG2;
    if (level == "DEBUG1")
        return logDEBUG1;
    if (level == "DEBUG")
        return logDEBUG;
    if (level == "INFO")
        return logINFO;
    if (level == "WARNING")
        return logWARNING;
    if (level == "ERROR")
        return logERROR;
	if (level == "OFF")
		return logOFF;
    Log<T>().Get(logWARNING) << "Unknown logging level '" << level << "'. Using INFO level as default.";
    return logINFO;
}

class Output2FILE
{
public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
};

inline FILE*& Output2FILE::Stream()
{
    static FILE* pStream = stderr;
    return pStream;
}

inline void Output2FILE::Output(const std::string& msg)
{   
    FILE* pStream = Stream();
    if (!pStream)
        return;
    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllimport)
#   else
#       define FILELOG_DECLSPEC
#   endif // BUILDING_DBSIMPLE_DLL
#else
#   define FILELOG_DECLSPEC
#endif // _WIN32

class FILELOG_DECLSPEC FILELog : public Log<Output2FILE> {};
//typedef Log<Output2FILE> FILELog;

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logDEBUG4
#endif

//#define FILE_LOG(level) \
//    if (level > FILELOG_MAX_LEVEL) ;\
//    else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
//    else FILELog().Get(level)

#define L_(level) \
	if (level > FILELOG_MAX_LEVEL) ;\
	else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
	else FILELog().Get(level)


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, 
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000); 
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}

#endif //WIN32

#endif //__LOG_H__
