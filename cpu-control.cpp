#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <iomanip>
#include <deque>
#include <climits>  // 添加头文件以使用INT_MIN和INT_MAX
#include <thread>
#include <chrono>
using namespace std;

// 灯光亮度文件路径常量
const std::string RED_PATH = "/sys/class/leds/onecloud:red:alive/brightness";
const std::string BLUE_PATH = "/sys/class/leds/onecloud:blue:alive/brightness";
const std::string GREEN_PATH = "/sys/class/leds/onecloud:green:alive/brightness";

// 渐变步长（值越小过渡越平滑）
const int STEP = 5;
// 渐变延迟时间（单位：毫秒，控制渐变速度）
const int DELAY_MS = 500;

/**
 * @brief 设置指定颜色的亮度
 * @param colorPath 颜色对应的文件路径
 * @param brightness 亮度值（0-255）
 */
void setBrightness(const std::string& colorPath, int brightness) {
    std::ofstream fout(colorPath);
    if (fout.is_open()) {
        fout << brightness;
        fout.close();
    } else {
        std::cerr << "无法打开文件: " << colorPath << std::endl;
    }
}

/**
 * @brief 颜色渐变函数
 * @param start 起始亮度
 * @param end 结束亮度
 * @param colorPath 颜色路径
 */
void fade(int start, int end, const std::string& colorPath) {
    if (start < end) {
        for (int i = start; i <= end; i += STEP) {
            setBrightness(colorPath, i);
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        }
    } else {
        for (int i = start; i >= end; i -= STEP) {
            setBrightness(colorPath, i);
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        }
    }
}

/**
 * @brief 渐变线程函数（用于并行渐变）
 * @param start 起始亮度
 * @param end 结束亮度
 * @param colorPath 颜色路径
 */
void fadeThread(int start, int end, const std::string& colorPath) {
    fade(start, end, colorPath);
}

// 获取当前时间字符串
string getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timeStr(dt);
    // 移除换行符
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }
    return timeStr;
}

// 计算运行时间（秒转换为时分秒格式）
string formatDuration(time_t seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    stringstream ss;
    ss << setfill('0') << setw(2) << hours << "h:"
       << setfill('0') << setw(2) << minutes << "m:"
       << setfill('0') << setw(2) << secs << "s";
    return ss.str();
}

// 写入日志
void writeLog(const string& message) {
    ofstream logFile;
    logFile.open("cpu_control.log", ios::app);
    if (logFile.is_open()) {
        logFile << "[" << getCurrentTime() << "] " << message << endl;
        logFile.close();
    }
}

// 限制日志大小为500行
void limitLogSize() {
    const int MAX_LINES = 500;
    deque<string> lines;
    
    // 读取所有行
    ifstream inFile("cpu_control.log");
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            lines.push_back(line);
            if (lines.size() > MAX_LINES) {
                lines.pop_front();
            }
        }
        inFile.close();
    }
    
    // 如果行数超过限制，重写日志
    if (lines.size() > MAX_LINES) {
        ofstream outFile("cpu_control.log");
        if (outFile.is_open()) {
            for (const auto& line : lines) {
                outFile << line << endl;
            }
            outFile.close();
        }
    }
}

struct TemperatureRecord {
    string time;
    int temp;
    string status;
};

// 根据CPU温度调节灯光
void adjustLighting(int temp) {
    if (temp <= 42) {
        // 低温时显示绿色
        setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 255);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 255);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
    } else if (temp >= 46) {
        // 高温时显示红色
        setBrightness(RED_PATH, 255);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
	 setBrightness(RED_PATH, 255);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
    } else {
        // 适中温度显示黄色
        setBrightness(RED_PATH, 255);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 255);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
	setBrightness(RED_PATH, 255);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 255);
	sleep(2);
	setBrightness(RED_PATH, 0);
        setBrightness(BLUE_PATH, 0);
        setBrightness(GREEN_PATH, 0);
	sleep(1);
    }
}

int main() {
    time_t startTime = time(0);
    string startTimeStr = getCurrentTime();
    cout << "程序开始运行时间: " << startTimeStr << endl << endl;
    
    // 写入启动日志
    writeLog("程序启动");
    
    int i = 1; // 低温提高主频标志
    int j = 0; // 高温降低主频标志
    
    vector<TemperatureRecord> history;
    int freqChanges = 0;
    string lastStatus = "";
    int maxTemp = INT_MIN;  // 使用INT_MIN初始化最高温度
    int minTemp = INT_MAX;  // 使用INT_MAX初始化最低温度

    while (true) {
        system("clear");
        
        time_t currentTime = time(0);
        string timeStr = getCurrentTime();
        time_t runTime = currentTime - startTime;
        
        cout << "[" << timeStr << "] CPU温度监控中..." << endl;
        cout << "=====================================" << endl;
        cout << "程序开始时间: " << startTimeStr << endl;
        cout << "运行时长: " << formatDuration(runTime) << endl;
        cout << "频率调整次数: " << freqChanges << endl;
        cout << "最高温度: " << ((maxTemp == INT_MIN) ? "N/A" : to_string(maxTemp) + "℃") << endl;
        cout << "最低温度: " << ((minTemp == INT_MAX) ? "N/A" : to_string(minTemp) + "℃") << endl;
        cout << "-------------------------------------" << endl;

        // 读取CPU温度
        ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        int temp;
        tempFile >> temp;
        tempFile.close();
        temp /= 1000; // 转换为℃
        
        // 更新最高最低温度
        if (temp > maxTemp) maxTemp = temp;
        if (temp < minTemp) minTemp = temp;

        // 读取当前频率调节策略
        ifstream governorFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        string governor;
        governorFile >> governor;
        governorFile.close();

        string status;
        bool freqChanged = false;
        
        // 低温时提高主频
        if (temp <= 42 && governor == "ondemand" && i == 1) {
            system("cpufreq-set -d 400MHz -u 1540MHz");
            status = "已提高频率上限至1540MHz";
            i = 0;
            j = 0;
            freqChanged = true;
        }
        // 高温时降低主频
        else if (temp >= 46 && governor == "ondemand" && j == 0) {
            system("cpufreq-set -d 400MHz -u 800MHz");
            status = "已降低频率上限至800MHz";
            i = 1;
            j = 1;
            freqChanged = true;
        }
        else {
            status = (i==1 && j==1) ? "降频中" : (i==0 && j==0) ? "升频中" : "初始状态";
        }

        // 检测状态变化并计数
        string currentStatus = (i==1 && j==1) ? "降频" : (i==0 && j==0) ? "升频" : "初始";
        if (lastStatus != "" && lastStatus != currentStatus) {
            freqChanges++;
            writeLog("频率调整: " + lastStatus + " → " + currentStatus);
        }
        lastStatus = currentStatus;

        // 添加到历史记录
        TemperatureRecord record;
        record.time = timeStr;
        record.temp = temp;
        record.status = status;
        history.push_back(record);
        
        // 保持最新5条记录
        if (history.size() > 5) {
            history.erase(history.begin());
        }

        // 输出当前状态
        cout << "当前CPU温度：" << temp << "℃" << endl;
        cout << i << " " << j << " 【当前状态：" 
             << ((i==1 && j==1) ? "降频中" : (i==0 && j==0) ? "升频中" : "初始状态") << "】" << endl;
        cout << endl;
        
        // 输出历史记录
        cout << "最近5次记录:" << endl;
        cout << "时间                     温度   状态" << endl;
        cout << "-----------------------------------" << endl;
        for (const auto& rec : history) {
            cout << rec.time << "  " 
                 << rec.temp << "℃   " 
                 << rec.status << endl;
        }
        
        cout << endl;
        cout << "日志文件: cpu_temp_control.log" << endl;
        cout << "=====================================" << endl;

        // 根据CPU温度调节灯光
        adjustLighting(temp);

        // 每小时写入一次统计信息
        static time_t lastStatsTime = startTime;
        if (currentTime - lastStatsTime >= 3600) {
            writeLog("运行统计 - 时长: " + formatDuration(runTime) + 
                     ", 频率调整: " + to_string(freqChanges) + 
                     ", 最高温度: " + to_string(maxTemp) + "℃" +
                     ", 最低温度: " + to_string(minTemp) + "℃");
            lastStatsTime = currentTime;
            
            // 检查并限制日志大小
            limitLogSize();
        }
    }

    return 0;
}