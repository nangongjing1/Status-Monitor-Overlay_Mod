/*
 * 模式专用配置设置
 * 
 * 基于实际设置结构，每种模式仅显示适用的设置：
 * 
 * Mini 模式：刷新率、颜色（背景、焦点背景、分隔线、分类、文字）、
 *           开关、字号、条目、时间格式
 * 
 * Micro 模式：刷新率、颜色（背景、分隔线、分类、文字）、开关、
 *            字号、条目、文字对齐、垂直位置（仅上/下）、时间格式
 * 
 * 完整模式：刷新率、开关（实际频率、差值等）、
 *           水平位置（仅左/右）- 无颜色、字体或条目
 * 
 * FPS 计数器：刷新率、颜色（仅背景、文字）、字号、
 *             水平/垂直位置
 * 
 * FPS 图表：刷新率、颜色（8种图表专用颜色）、开关（仅显示信息）、
 *           水平/垂直位置 - 无字体
 * 
 * 游戏分辨率：刷新率、颜色（仅背景、分类、文字）、
 *             水平/垂直位置 - 无开关、字体或条目
 */

#pragma once
#include <tesla.hpp>
#include "../Utils.hpp"

#include <unordered_set>

// 导航用的外部变量
extern std::string jumpItemName;
extern std::string jumpItemValue;
extern std::atomic<bool> jumpItemExactMatch;
static tsl::elm::ListItem* lastSelectedListItem;

// 前向声明
class ConfiguratorOverlay;
class RefreshRateConfig;
class FontSizeConfig;
class FontSizeSelector;
class ColorConfig;
class ColorSelector;
class AlphaSelector;
class ShowConfig;
class TogglesConfig;
class DTCFormatConfig;
class ModeComboConfig;

// =============================================================================
// 共享辅助函数
// =============================================================================

// 将模式名称映射到其 INI section 字符串。
inline std::string modeToSection(const std::string& mode) {
    if (mode == "Mini")             return "mini";
    if (mode == "Micro")            return "micro";
    if (mode == "Full")             return "full";
    if (mode == "FPS Counter")      return "fps-counter";
    if (mode == "FPS Graph")        return "fps-graph";
    if (mode == "Game Resolutions") return "game_resolutions";
    return "";
}

// 读取布尔型 INI 值，当键不存在时返回默认值。
inline bool readBool(const std::string& section, const std::string& key, bool defaultVal = true) {
    std::string value = ult::parseValueFromIniSection(configIniPath, section, key);
    if (value.empty()) return defaultVal;
    convertToUpper(value);
    return value != "FALSE";
}

// 读取反转存储的布尔值（键为 "show_stacked_*" 时，
// 文件中的 true 表示 NOT stacked）。返回逻辑的 "stacked" 状态。
inline bool readInvertedBool(const std::string& section, const std::string& key, bool defaultVal = true) {
    std::string value = ult::parseValueFromIniSection(configIniPath, section, key);
    if (value.empty()) return defaultVal;
    convertToUpper(value);
    return value == "FALSE"; // 反转：文件中 "false" => stacked true
}

// 颜色辅助函数
inline std::string extractColorWithoutAlpha(const std::string& rgba) {
    if (rgba.length() >= 5 && rgba[0] == '#') return rgba.substr(0, 4);
    return rgba;
}

inline std::string extractAlphaFromColor(const std::string& rgba) {
    if (rgba.length() == 5 && rgba[0] == '#') return std::string(1, rgba[4]);
    return "9";
}

inline std::string setAlphaInColor(const std::string& rgba, char alpha) {
    if (rgba.length() >= 4 && rgba[0] == '#') return rgba.substr(0, 4) + alpha;
    return rgba;
}

// 共享颜色名称查找。ColorSelector 和 ColorConfig 均使用。
// 返回 #RGB 字符串对应的颜色名称，未知则返回 hex 本身。
inline std::string getColorName(const std::string& hexColor) {
    std::string rgb = (hexColor.length() == 5 && hexColor[0] == '#')
                      ? hexColor.substr(0, 4) : hexColor;

    static const std::map<std::string, std::string> colorNames = {
        {"#000","黑色"},      {"#222","炭黑"},    {"#444","深灰"},
        {"#666","灰色"},       {"#999","浅灰"},   {"#CCC","银色"},
        {"#EEE","米白"},  {"#FFF","白色"},

        {"#200","深红"},   {"#700","栗色"},       {"#B22","绯红"},
        {"#F00","红色"},        {"#F66","浅红"},    {"#FAA","鲑红"},

        {"#520","深橙"},{"#A40","焦橙"}, {"#F80","橙色"},
        {"#FB6","浅橙"},{"#FC8","蜜桃"},

        {"#220","暗黄"},{"#CA0","金色"},         {"#FF0","黄色"},
        {"#FF6","浅黄"},{"#FFC","奶油"},

        {"#020","深绿"}, {"#063","森林绿"}, {"#080","绿色"},
        {"#0C0","酸橙绿"}, {"#0F0","亮绿"}, {"#8F8","浅绿"},
        {"#CFA","薄荷"},

        {"#022","深青"},  {"#066","蓝绿"},         {"#0AA","水绿"},
        {"#0FF","青色"},       {"#8FF","浅青"},

        {"#002","午夜蓝"},{"#003","深蓝"},  {"#04A","海军蓝"},
        {"#06F","皇家蓝"}, {"#00F","蓝色"},         {"#2DF","浅蓝"},
        {"#8CF","天蓝"},   {"#ACE","粉蓝"},

        {"#202","深紫"},{"#404","茄紫"},     {"#608","靛蓝"},
        {"#808","紫色"},     {"#A0F","紫罗兰"},       {"#C8F","薰衣草"},
        {"#D9F","浅薰衣草"},

        {"#606","深品红"},{"#F0F","品红"},     {"#F4A","艳粉"},
        {"#F8A","粉红"},       {"#FCE","浅粉"},

        {"#321","深棕"}, {"#642","棕色"},        {"#A75","浅棕"},
        {"#DB8","棕褐"},
    };

    auto it = colorNames.find(rgb);
    if (it != colorNames.end()) {
        if (rgb == "#000" && hexColor.length() == 5 && hexColor[4] == '0')
            return "透明";
        return it->second;
    }
    return rgb;
}

// 将 4 位 alpha 半字节字符转换为百分比字符串。
inline std::string alphaToPercent(char alpha) {
    static const char* const table[16] = {
        "0%","7%","13%","20%","27%","33%","40%","47%",
        "53%","60%","67%","73%","80%","87%","93%","100%"
    };
    const int idx = (alpha >= '0' && alpha <= '9') ? (alpha - '0')
                  : (alpha >= 'A' && alpha <= 'F') ? (alpha - 'A' + 10)
                  : (alpha >= 'a' && alpha <= 'f') ? (alpha - 'a' + 10)
                  : 9;
    return table[idx];
}

inline std::string getAlphaPercentage(const std::string& color) {
    if (color.length() == 5 && color[0] == '#') return alphaToPercent(color[4]);
    return "60%";
}

// 紧凑的模式标志集合。每个类构造时从模式字符串初始化一次。
struct ModeFlags {
    bool isMini, isMicro, isFull, isGameRes, isFPSCounter, isFPSGraph;
    explicit ModeFlags(const std::string& mode)
        : isMini      (mode == "Mini")
        , isMicro     (mode == "Micro")
        , isFull      (mode == "Full")
        , isGameRes   (mode == "Game Resolutions")
        , isFPSCounter(mode == "FPS Counter")
        , isFPSGraph  (mode == "FPS Graph") {}
};

// 一次性清除所有三个跳转状态全局变量。
inline void clearJump() {
    jumpItemName = ""; jumpItemValue = ""; jumpItemExactMatch = false;
}

// 对新项应用勾选标记，同时清除先前选项的标记。
inline void selectItem(tsl::elm::ListItem*& lastSelected,
                       tsl::elm::ListItem* newItem,
                       const std::string& checkmark,
                       const std::string& clearValue = "") {
    if (lastSelected && lastSelected != newItem)
        lastSelected->setValue(clearValue);
    newItem->setValue(checkmark);
    lastSelected = newItem;
}

// 构建标准 OverlayFrame，附加内容并返回。
inline tsl::elm::Element* makeFrame(const std::string& subtitle, tsl::elm::Element* content) {
    auto* f = new tsl::elm::OverlayFrame("状态监控", subtitle);
    f->setContent(content);
    return f;
}

// 将 #RGB 或 #RGBA 字符串转换为完全不透明的 tsl::Color 用于颜色色块。
// 每个十六进制半字节直接映射到 4 位 RGBA4444 通道值。
inline tsl::Color hexToSwatchColor(const std::string& hex) {
    if (hex.size() < 4 || hex[0] != '#') return tsl::Color(0xF, 0xF, 0xF, 0xF);
    auto nibble = [](char c) -> u8 {
        if (c >= '0' && c <= '9') return static_cast<u8>(c - '0');
        if (c >= 'A' && c <= 'F') return static_cast<u8>(c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return static_cast<u8>(c - 'a' + 10);
        return 0u;
    };
    return tsl::Color(nibble(hex[1]), nibble(hex[2]), nibble(hex[3]), 0xF);
}

// 用于列表项颜色色块的 Unicode 实心方块。
static const std::string COLOR_SWATCH = "■";

// drawStringWithColoredSections 的特殊字符向量 — 将 ■ 路由到色块颜色。
static const std::vector<std::string> COLOR_SWATCH_SPECIAL = { COLOR_SWATCH };

// 模板：isMini=true 使用 MiniListItem 高度，isMini=false 使用完整 ListItem 高度。
// 通过在父类渲染之前从 m_value 中剥离色块以显示其真实颜色
// （无双重合成/抗锯齿边缘），然后通过 drawStringWithColoredSections
// 使用存储的色块颜色绘制一次。勾选标记和焦点高亮由父类正常绘制。
template<bool isMini>
class ColorSwatchListItemT : public std::conditional_t<isMini, tsl::elm::MiniListItem, tsl::elm::ListItem> {
    using Base = std::conditional_t<isMini, tsl::elm::MiniListItem, tsl::elm::ListItem>;
public:
    explicit ColorSwatchListItemT(const std::string& text)
        : Base(text), m_swatchColor(tsl::Color(0xF, 0xF, 0xF, 0xF)) {}

    void setSwatchColor(tsl::Color color) { m_swatchColor = color; }

    virtual void draw(tsl::gfx::Renderer* renderer) override {
        const std::string full = this->m_value;

        // x = getX() + getWidth() - textWidth(full,20) - 19
        const s32 fullValueWidth = renderer->getTextDimensions(full, false, 20).first;
        const s32 swatchX = this->getX() + this->getWidth() - fullValueWidth - 19;

        // 剥离色块，使父类不会渲染它。
        std::string withoutSwatch = full;
        const auto pos = withoutSwatch.find(COLOR_SWATCH);
        if (pos != std::string::npos) {
            withoutSwatch.erase(pos, COLOR_SWATCH.size());
            while (!withoutSwatch.empty() && withoutSwatch.front() == ' ')
                withoutSwatch.erase(withoutSwatch.begin());
        }
        this->m_value    = withoutSwatch;
        this->m_maxWidth = 0;
        Base::draw(renderer);
        this->m_value    = full;
        this->m_maxWidth = 0;

        // 在正确位置以 m_swatchColor 绘制色块一次。
        static constexpr s32 fontSize   = 20;
        static constexpr u16 itemHeight = isMini ? tsl::style::MiniListItemDefaultHeight
                                                 : tsl::style::ListItemDefaultHeight;
        static constexpr s32 yOffset    = (tsl::style::ListItemDefaultHeight - itemHeight) / 2 + 1;
        const s32 swatchY = this->getY() + 45 - yOffset - 1;
        renderer->drawStringWithColoredSections(
            COLOR_SWATCH, false, COLOR_SWATCH_SPECIAL,
            swatchX, swatchY, fontSize, tsl::Color(0, 0, 0, 0), m_swatchColor);
    }

private:
    tsl::Color m_swatchColor;
};

// ColorConfig 导航行（完整高度）；ColorSelector 调色板行（mini 高度）。
using ColorSwatchListItem     = ColorSwatchListItemT<false>;
using MiniColorSwatchListItem = ColorSwatchListItemT<true>;

// ColorSelector 使用的共享颜色调色板。
static const std::vector<std::pair<std::string, std::string>> g_colorPalette = {
    {"黑色","#000"},        {"炭黑","#222"},     {"深灰","#444"},
    {"灰色","#666"},         {"浅灰","#999"},   {"银色","#CCC"},
    {"米白","#EEE"},    {"白色","#FFF"},

    {"深红","#200"},     {"栗色","#700"},       {"绯红","#B22"},
    {"红色","#F00"},          {"浅红","#F66"},    {"鲑红","#FAA"},

    {"深橙","#520"},  {"焦橙","#A40"}, {"橙色","#F80"},
    {"浅橙","#FB6"}, {"蜜桃","#FC8"},

    {"暗黄","#220"},  {"金色","#CA0"},         {"黄色","#FF0"},
    {"浅黄","#FF6"}, {"奶油","#FFC"},

    {"深绿","#020"},   {"森林绿","#063"}, {"绿色","#080"},
    {"酸橙绿","#0C0"},   {"亮绿","#0F0"}, {"浅绿","#8F8"},
    {"薄荷","#CFA"},

    {"深青","#022"},    {"蓝绿","#066"},         {"水绿","#0AA"},
    {"青色","#0FF"},         {"浅青","#8FF"},

    {"午夜蓝","#002"},{"深蓝","#003"},    {"海军蓝","#04A"},
    {"皇家蓝","#06F"},   {"蓝色","#00F"},         {"浅蓝","#2DF"},
    {"天蓝","#8CF"},     {"粉蓝","#ACE"},

    {"深紫","#202"},  {"茄紫","#404"},     {"靛蓝","#608"},
    {"紫色","#808"},       {"紫罗兰","#A0F"},       {"薰衣草","#C8F"},
    {"浅薰衣草","#D9F"},

    {"深品红","#606"}, {"品红","#F0F"},      {"艳粉","#F4A"},
    {"粉红","#F8A"},         {"浅粉","#FCE"},

    {"深棕","#321"},   {"棕色","#642"},        {"浅棕","#A75"},
    {"棕褐","#DB8"},
};

// =============================================================================
// 模式快捷键辅助函数
// =============================================================================

static constexpr const char* const g_defaultModeCombos[] = {
    "ZL+ZR+DDOWN",  "ZL+ZR+DRIGHT", "ZL+ZR+DUP",    "ZL+ZR+DLEFT",
    "L+R+DDOWN",    "L+R+DRIGHT",   "L+R+DUP",       "L+R+DLEFT",
    "L+DDOWN",      "R+DDOWN",
    "ZL+ZR+PLUS",   "L+R+PLUS",     "ZL+ZR+MINUS",   "L+R+MINUS",
    "ZL+MINUS",     "ZR+MINUS",     "ZL+PLUS",        "ZR+PLUS",    "MINUS+PLUS",
    "LS+RS",        "L+DDOWN+RS",   "L+R+LS",         "L+R+RS",
    "ZL+ZR+LS",     "ZL+ZR+RS",     "ZL+ZR+L",        "ZL+ZR+R",    "ZL+ZR+LS+RS"
};

static constexpr size_t kModeComboSlotCount = 5;

inline int modeComboIndexFor(const std::string& modeName) {
    if (modeName == "Full")             return 0;
    if (modeName == "Mini")             return 1;
    if (modeName == "Micro")            return 2;
    if (modeName == "FPS Graph")        return 3;
    if (modeName == "FPS Counter")      return 4;
    if (modeName == "Game Resolutions") return 5;
    return -1;
}

inline std::string readModeCombo(int modeIdx) {
    if (modeIdx < 0 || modeIdx >= static_cast<int>(kModeComboSlotCount)) return "";
    if (filename.empty() || !ult::isFile(ult::OVERLAYS_INI_FILEPATH)) return "";
    const std::string mc = ult::parseValueFromIniSection(
        ult::OVERLAYS_INI_FILEPATH, filename, "mode_combos");
    if (mc.empty()) return "";
    auto slots = ult::splitIniList(mc);
    if (modeIdx >= static_cast<int>(slots.size())) return "";
    return slots[modeIdx];
}

inline void removeModeComboFromOthers(const std::string& keyCombo) {
    if (keyCombo.empty()) return;
    const u64 targetKeys = tsl::hlp::comboStringToKeys(keyCombo);
    if (targetKeys == 0) return;

    if (ult::isFile(ult::OVERLAYS_INI_FILEPATH)) {
        auto data = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
        bool dirty = false;
        for (auto& [name, section] : data) {
            auto kcIt = section.find("key_combo");
            if (kcIt != section.end() && !kcIt->second.empty() &&
                tsl::hlp::comboStringToKeys(kcIt->second) == targetKeys) {
                kcIt->second = "";
                dirty = true;
            }
            auto mcIt = section.find("mode_combos");
            if (mcIt != section.end() && !mcIt->second.empty()) {
                auto slots = ult::splitIniList(mcIt->second);
                bool changed = false;
                for (auto& c : slots) {
                    if (!c.empty() && tsl::hlp::comboStringToKeys(c) == targetKeys) {
                        c = "";
                        changed = true;
                    }
                }
                if (changed) {
                    mcIt->second = "(" + ult::joinIniList(slots) + ")";
                    dirty = true;
                }
            }
        }
        if (dirty) ult::saveIniFileData(ult::OVERLAYS_INI_FILEPATH, data);
    }

    if (ult::isFile(ult::PACKAGES_INI_FILEPATH)) {
        auto pkgData = ult::getParsedDataFromIniFile(ult::PACKAGES_INI_FILEPATH);
        bool pkgDirty = false;
        for (auto& [name, section] : pkgData) {
            auto kcIt = section.find("key_combo");
            if (kcIt != section.end() && !kcIt->second.empty() &&
                tsl::hlp::comboStringToKeys(kcIt->second) == targetKeys) {
                kcIt->second = "";
                pkgDirty = true;
            }
        }
        if (pkgDirty) ult::saveIniFileData(ult::PACKAGES_INI_FILEPATH, pkgData);
    }
}

inline void writeModeCombo(int modeIdx, const std::string& combo) {
    if (modeIdx < 0 || modeIdx >= static_cast<int>(kModeComboSlotCount)) return;
    if (filename.empty()) return;
    auto iniData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
    auto& section = iniData[filename];
    auto slots = ult::splitIniList(section["mode_combos"]);
    if (slots.size() < kModeComboSlotCount) slots.resize(kModeComboSlotCount);
    slots[modeIdx] = combo;
    section["mode_combos"] = "(" + ult::joinIniList(slots) + ")";
    ult::saveIniFileData(ult::OVERLAYS_INI_FILEPATH, iniData);
    tsl::hlp::loadEntryKeyCombos();
}

// =============================================================================
// 透明度选择器
// =============================================================================
class AlphaSelector : public tsl::Gui {
private:
    std::string modeName;
    std::string colorKey;
    std::string title;

public:
    AlphaSelector(const std::string& mode, const std::string& key, const std::string& displayTitle)
        : modeName(mode), colorKey(key), title(displayTitle) {}

    ~AlphaSelector() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader(title));

        const std::string section = modeToSection(modeName);
        std::string currentColor = ult::parseValueFromIniSection(configIniPath, section, colorKey);
        if (currentColor.empty()) currentColor = "#000A";
        const std::string currentAlpha = extractAlphaFromColor(currentColor);

        static const std::pair<std::string, char> alphaOptions[16] = {
            {"0%",'0'},   {"7%",'1'},   {"13%",'2'},  {"20%",'3'},
            {"27%",'4'},  {"33%",'5'},  {"40%",'6'},  {"47%",'7'},
            {"53%",'8'},  {"60%",'9'},  {"67%",'A'},  {"73%",'B'},
            {"80%",'C'},  {"87%",'D'},  {"93%",'E'},  {"100%",'F'}
        };

        for (const auto& option : alphaOptions) {
            auto* alphaItem = new tsl::elm::MiniListItem(option.first);
            if (!currentAlpha.empty() && currentAlpha[0] == option.second)
                selectItem(lastSelectedListItem, alphaItem, ult::CHECKMARK_SYMBOL);
            alphaItem->setClickListener([this, alphaItem, option, section](uint64_t keys) {
                if (keys & KEY_A) {
                    std::string color = ult::parseValueFromIniSection(configIniPath, section, colorKey);
                    if (color.empty()) color = "#000A";
                    ult::setIniFileValue(configIniPath, section, colorKey, setAlphaInColor(color, option.second));
                    selectItem(lastSelectedListItem, alphaItem, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(alphaItem);
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
        return makeFrame("透明度", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = title;
            jumpItemValue = "";
            jumpItemExactMatch = false;
            tsl::swapTo<ColorConfig>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 时间格式配置
// =============================================================================
struct DTCFormatEntry {
    std::string label;
    std::string fmt;
};
struct DTCFormatCategory {
    std::string header;
    std::vector<DTCFormatEntry> entries;
};

static const std::vector<DTCFormatCategory> dtcFormatCategories = {
    {"时间", {
        {"24小时制",    "%H:%M"},
        {"24小时制(秒)", "%H:%M:%S"},
        {"12小时制",    "%l:%M %p"},
        {"12小时制(秒)", "%l:%M:%S %p"}
    }},
    {"日期", {
        {"US 日期(横杠)",   "%m-%d-%Y"},
        {"US 日期(斜杠)",  "%m/%d/%Y"},
        {"EU 日期(横杠)",   "%d-%m-%Y"},
        {"EU 日期(斜杠)",  "%d/%m/%Y"},
        {"ISO 日期",        "%Y-%m-%d"},
        {"ISO 日期(斜杠)", "%Y/%m/%d"},
        {"短日期 US",   "%m/%d/%y"},
        {"短日期 EU",   "%d/%m/%y"},
        {"紧凑日期",    "%Y%m%d"}
    }},
    {"星期/月", {
        {"短星期+日期",  "%a, %b %d"},
        {"完整星期+日期","%A, %B %d"},
        {"星期(缩写)",    "%a"},
        {"星期(完整)",    "%A"},
        {"月份(缩写)",    "%b"},
        {"月份(完整)",    "%B"},
        {"月份/年",     "%b %Y"},
        {"年中第几天",   "%j"},
    }},
};

static const std::vector<std::pair<std::string, std::string>>& getDTCFormatsFlat() {
    static const std::vector<std::pair<std::string, std::string>> flat = []() {
        std::vector<std::pair<std::string, std::string>> out;
        for (const auto& cat : dtcFormatCategories)
            for (const auto& e : cat.entries)
                out.push_back({e.label, e.fmt});
        return out;
    }();
    return flat;
}

class DTCFormatConfig : public tsl::Gui {
private:
    std::string modeName;
    int slot;

public:
    DTCFormatConfig(const std::string& mode, int slotIndex)
        : modeName(mode), slot(slotIndex) {}
    ~DTCFormatConfig() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        const std::string section = modeToSection(modeName);
        const std::string iniKey  = (slot == 2) ? "dtc_format_2" : "dtc_format_1";
        const std::string title   = (slot == 2) ? "时间格式 2" : "时间格式 1";
        //const bool isMiniMode     = (modeName == "Mini");

        std::string currentValue = ult::parseValueFromIniSection(configIniPath, section, iniKey);
        if (currentValue.empty()) {
            if (slot == 1) currentValue = std::string("%a, %b %d");
            else           currentValue = std::string("%l:%M:%S %p");
        }

        if (slot == 2) {
            list->addItem(new tsl::elm::CategoryHeader("无"));
            auto* noneItem = new tsl::elm::MiniListItem(ult::OPTION_SYMBOL);
            if (currentValue == ult::OPTION_SYMBOL)
                selectItem(lastSelectedListItem, noneItem, ult::CHECKMARK_SYMBOL);
            noneItem->setClickListener([this, noneItem, section](uint64_t keys) {
                if (keys & KEY_A) {
                    ult::setIniFileValue(configIniPath, section, "dtc_format_2", ult::OPTION_SYMBOL);
                    selectItem(lastSelectedListItem, noneItem, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(noneItem);
        }

        for (const auto& cat : dtcFormatCategories) {
            list->addItem(new tsl::elm::CategoryHeader(cat.header));
            for (const auto& entry : cat.entries) {
                auto* formatItem = new tsl::elm::MiniListItem(entry.label);
                if (entry.fmt == currentValue)
                    selectItem(lastSelectedListItem, formatItem, ult::CHECKMARK_SYMBOL);
                const std::string capturedFmt = entry.fmt;
                const std::string capturedKey = iniKey;
                formatItem->setClickListener([this, formatItem, capturedFmt, capturedKey, section](uint64_t keys) {
                    if (keys & KEY_A) {
                        ult::setIniFileValue(configIniPath, section, capturedKey, capturedFmt);
                        selectItem(lastSelectedListItem, formatItem, ult::CHECKMARK_SYMBOL);
                        return true;
                    }
                    return false;
                });
                list->addItem(formatItem);
            }
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
        return makeFrame(title, list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = (slot == 2) ? "时间格式 2" : "时间格式 1";
            jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ConfiguratorOverlay>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 开关配置
// =============================================================================
class TogglesConfig : public tsl::Gui {
private:
    std::string modeName;
    ModeFlags   flags;
    std::string section; // 缓存一次

    void addToggle(tsl::elm::List* list, const std::string& label,
                   const std::string& key, bool defaultVal,
                   const std::string& overrideSection = "") {
        const std::string& sec = overrideSection.empty() ? section : overrideSection;
        auto* item = new tsl::elm::MiniToggleListItem(label, readBool(sec, key, defaultVal));
        item->setStateChangedListener([sec, key](bool state) {
            ult::setIniFileValue(configIniPath, sec, key, state ? "true" : "false");
        });
        list->addItem(item);
    }



public:
    TogglesConfig(const std::string& mode) : modeName(mode), flags(mode) {
        section = modeToSection(mode);
    }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();

        if (flags.isFPSGraph) {
            list->addItem(new tsl::elm::CategoryHeader("全局"));
            addToggle(list, "禁用截图",    "disable_screenshots", false);
            addToggle(list, "显示信息",    "show_info",           true);
            addToggle(list, "动态色彩",    "use_dynamic_colors",  true);
            addToggle(list, "整数 FPS",    "integer_fps",         true);

        } else if (flags.isFull) {
            list->addItem(new tsl::elm::CategoryHeader("全局"));
            addToggle(list, "禁用截图",    "disable_screenshots", false);
            addToggle(list, "实际频率",    "show_real_freqs",     true);
            addToggle(list, "目标频率",    "show_target_freqs",   true);
            addToggle(list, "频率差值",    "show_deltas",         true);
            addToggle(list, "FPS",         "show_fps",            true);
            addToggle(list, "分辨率",      "show_res",            true);
            addToggle(list, "读取速度",    "show_read_speed",     true);
            addToggle(list, "动态色彩",    "use_dynamic_colors",  true);

        } else if (flags.isMini || flags.isMicro) {
            list->addItem(new tsl::elm::CategoryHeader("全局"));
            addToggle(list, "1080p 底座",  "use_1080p_docked",   true);
            addToggle(list, "禁用截图",    "disable_screenshots", false);

            if (flags.isMini)
                addToggle(list, "显示标签", "show_labels", true);

            addToggle(list, "实际频率",    "real_freqs",          true);
            addToggle(list, "实际电压",    "real_volts",          true);
            addToggle(list, "动态色彩",    "use_dynamic_colors",  true);

            list->addItem(new tsl::elm::CategoryHeader("CPU"));
            addToggle(list, "完整 CPU",              "show_full_cpu",              true);
            addToggle(list, "完整 CPU 最大核心 0-2", "show_full_cpu_max_core_012", flags.isMini);
            addToggle(list, "堆叠完整 CPU",          "show_stacked_full_cpu",      flags.isMicro);
            addToggle(list, "CPU 温度",              "show_cpu_temp",              flags.isMicro);
            addToggle(list, "堆叠 CPU 温度",         "show_stacked_cpu_temp",      true);
            addToggle(list, "末端电压",              "voltage_at_end_cpu",         flags.isMicro);

            list->addItem(new tsl::elm::CategoryHeader("GPU"));
            addToggle(list, "GPU 温度",         "show_gpu_temp",         flags.isMicro);
            addToggle(list, "堆叠 GPU 温度",    "show_stacked_gpu_temp", true);
            addToggle(list, "末端电压",         "voltage_at_end_gpu",    true);

            list->addItem(new tsl::elm::CategoryHeader("RAM"));
            addToggle(list, "内存带宽",               "show_ram_bandwidth",               true);
            addToggle(list, "堆叠内存带宽",           "show_stacked_ram_bandwidth",       true);
            addToggle(list, "内存负载 CPU/GPU",       "show_RAM_load_CPU_GPU",            false);
            addToggle(list, "堆叠内存负载 CPU/GPU",   "show_stacked_ram_load_cpu_gpu",    true);

            if (isMariko)
                addToggle(list, "VDD2", "show_vdd2", true);

            addToggle(list, "VDDQ", "show_vddq", flags.isMini);

            if (isMariko)
                addToggle(list, "堆叠 VDD2/VDDQ", "show_stacked_vddq", true);

            addToggle(list, "RAM 温度",         "show_ram_temp",         flags.isMicro);
            addToggle(list, "堆叠 RAM 温度",    "show_stacked_ram_temp", true);
            addToggle(list, "末端电压",         "voltage_at_end_ram",    flags.isMicro);

            list->addItem(new tsl::elm::CategoryHeader("温度"));
            {
                // 互斥对
                auto* compTemps   = new tsl::elm::MiniToggleListItem("CPU/GPU/RAM 温度",
                    readBool(section, "show_component_temps", true));
                auto* socPcbTemps = new tsl::elm::MiniToggleListItem("SOC/PCB/Skin 温度",
                    readBool(section, "show_soc_pcb_skin_temps", true));

                compTemps->setStateChangedListener([this, compTemps, socPcbTemps](bool state) {
                    if (!state && !socPcbTemps->getState()) {
                        compTemps->setState(true);
                        ult::setIniFileValue(configIniPath, section, "show_component_temps", "true");
                    } else {
                        ult::setIniFileValue(configIniPath, section, "show_component_temps",
                                            state ? "true" : "false");
                    }
                });
                socPcbTemps->setStateChangedListener([this, compTemps, socPcbTemps](bool state) {
                    if (!state && !compTemps->getState()) {
                        socPcbTemps->setState(true);
                        ult::setIniFileValue(configIniPath, section, "show_soc_pcb_skin_temps", "true");
                    } else {
                        ult::setIniFileValue(configIniPath, section, "show_soc_pcb_skin_temps",
                                            state ? "true" : "false");
                    }
                });
                list->addItem(compTemps);
                list->addItem(socPcbTemps);

                if (flags.isMicro)
                    addToggle(list, "堆叠温度", "show_stacked_temps", true);
            }

            addToggle(list, "SOC 电压",       "show_soc_voltage",     true);
            addToggle(list, "堆叠风扇/SOC",   "show_stacked_fan_soc", true);
            addToggle(list, "末端电压",       "voltage_at_end_tmp",   true);

            list->addItem(new tsl::elm::CategoryHeader("分辨率"));
            addToggle(list, "完整分辨率", "show_full_res", true);
            addToggle(list, "仅主屏",    "show_primary_res", flags.isMicro);

            list->addItem(new tsl::elm::CategoryHeader("FPS"));
            addToggle(list, "整数 FPS", "integer_fps", true);

            list->addItem(new tsl::elm::CategoryHeader("电池"));
            addToggle(list, "反转电池显示", "invert_battery_display", true);
            addToggle(list, "堆叠",         "show_stacked_bat",       flags.isMicro);

            list->addItem(new tsl::elm::CategoryHeader("DTC"));
            addToggle(list, "使用 DTC 符号", "use_dtc_symbol",   true);
            addToggle(list, "堆叠",          "show_stacked_dtc", flags.isMicro);

        } else if (flags.isGameRes) {
            list->addItem(new tsl::elm::CategoryHeader("全局"));
            addToggle(list, "禁用截图", "disable_screenshots", false);

        } else if (flags.isFPSCounter) {
            list->addItem(new tsl::elm::CategoryHeader("全局"));
            addToggle(list, "1080p 底座",  "use_1080p_docked",   true);
            addToggle(list, "禁用截图",    "disable_screenshots", false);
            addToggle(list, "整数 FPS",    "integer_fps",         true);
        }

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        clearJump();

        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            tsl::goBack();
            return true;
        }
        return false;
    }
};

// =============================================================================
// 刷新率配置
// =============================================================================
class RefreshRateConfig : public tsl::Gui {
private:
    std::string modeName;
    ModeFlags   flags;
    int         currentRate;

public:
    RefreshRateConfig(const std::string& mode) : modeName(mode), flags(mode) {
        const std::string section = modeToSection(mode);
        const std::string value = ult::parseValueFromIniSection(configIniPath, section, "refresh_rate");
        const int defaultRate = (flags.isFPSCounter || flags.isFPSGraph) ? 5 : 3;
        currentRate = value.empty() ? defaultRate : std::clamp(atoi(value.c_str()), 1, 60);
    }
    ~RefreshRateConfig() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("刷新率"));

        const std::string section = modeToSection(modeName);
        static const int rates[] = {1, 2, 3, 5, 10, 15, 30, 60};
        for (int rate : rates) {
            auto* rateItem = new tsl::elm::MiniListItem(std::to_string(rate) + " Hz");
            if (rate == currentRate)
                selectItem(lastSelectedListItem, rateItem, ult::CHECKMARK_SYMBOL);
            rateItem->setClickListener([this, rateItem, rate, section](uint64_t keys) {
                if (keys & KEY_A) {
                    ult::setIniFileValue(configIniPath, section, "refresh_rate", std::to_string(rate));
                    selectItem(lastSelectedListItem, rateItem, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(rateItem);
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
            return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = "刷新率"; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ConfiguratorOverlay>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 模式快捷键配置
// =============================================================================
class ModeComboConfig : public tsl::Gui {
private:
    std::string modeName;
    int         modeIdx;
    std::string currentCombo;

public:
    ModeComboConfig(const std::string& mode)
        : modeName(mode)
        , modeIdx(modeComboIndexFor(mode))
        , currentCombo(readModeCombo(modeIdx)) {}

    ~ModeComboConfig() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("模式快捷键"));

        {
            auto* noneItem = new tsl::elm::ListItem(ult::OPTION_SYMBOL);
            if (currentCombo.empty())
                selectItem(lastSelectedListItem, noneItem, ult::CHECKMARK_SYMBOL);
            noneItem->setClickListener([this, noneItem](uint64_t keys) -> bool {
                if (!(keys & KEY_A)) return false;
                writeModeCombo(modeIdx, "");
                currentCombo.clear();
                selectItem(lastSelectedListItem, noneItem, ult::CHECKMARK_SYMBOL);
                return true;
            });
            list->addItem(noneItem);
        }

        const u64 launchKeys  = tsl::cfg::launchCombo;
        const u64 currentKeys = currentCombo.empty() ? 0
                              : tsl::hlp::comboStringToKeys(currentCombo);

        for (const auto& combo : g_defaultModeCombos) {
            const u64 comboKeys = tsl::hlp::comboStringToKeys(combo);
            if (comboKeys == launchKeys) continue;

            std::string display = combo;
            ult::convertComboToUnicode(display);

            auto* item = new tsl::elm::ListItem(display);
            if (currentKeys != 0 && comboKeys == currentKeys)
                selectItem(lastSelectedListItem, item, ult::CHECKMARK_SYMBOL);

            const std::string comboStr(combo);
            item->setClickListener([this, item, comboStr](uint64_t keys) -> bool {
                if (!(keys & KEY_A)) return false;
                removeModeComboFromOthers(comboStr);
                writeModeCombo(modeIdx, comboStr);
                currentCombo = comboStr;
                selectItem(lastSelectedListItem, item, ult::CHECKMARK_SYMBOL);
                return true;
            });
            list->addItem(item);
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = "模式快捷键"; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ConfiguratorOverlay>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 帧填充配置
// =============================================================================
class FramePaddingConfig : public tsl::Gui {
private:
    std::string modeName;
    std::string section;
    int currentPadding;

public:
    FramePaddingConfig(const std::string& mode) : modeName(mode) {
        section = modeToSection(mode);
        const std::string value = ult::parseValueFromIniSection(configIniPath, section, "frame_padding");
        currentPadding = value.empty() ? 10 : std::clamp(atoi(value.c_str()), 0, 14);
    }
    ~FramePaddingConfig() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("帧填充"));

        for (int padding = 0; padding <= 14; ++padding) {
            auto* paddingItem = new tsl::elm::MiniListItem(std::to_string(padding) + " px");
            if (padding == currentPadding)
                selectItem(lastSelectedListItem, paddingItem, ult::CHECKMARK_SYMBOL);
            paddingItem->setClickListener([this, paddingItem, padding](uint64_t keys) {
                if (keys & KEY_A) {
                    ult::setIniFileValue(configIniPath, section, "frame_padding", std::to_string(padding));
                    selectItem(lastSelectedListItem, paddingItem, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(paddingItem);
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
            return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = "帧填充"; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ConfiguratorOverlay>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// Micro 填充配置（水平 / 垂直 / 标签）
// =============================================================================

// 所有三个 Micro 填充屏幕的共享实现。
template<int DEFAULT, int MIN_P, int MAX_P>
class MicroPaddingConfigBase : public tsl::Gui {
protected:
    int         currentPadding;
    std::string iniKey;
    std::string headerLabel;
    std::string jumpLabel;

    virtual std::string formatLabel(int p) const { return std::to_string(p) + " px"; }

public:
    MicroPaddingConfigBase(const std::string& key, const std::string& header, const std::string& jump)
        : iniKey(key), headerLabel(header), jumpLabel(jump) {
        const std::string value = ult::parseValueFromIniSection(configIniPath, "micro", iniKey);
        currentPadding = value.empty() ? DEFAULT : std::clamp(atoi(value.c_str()), MIN_P, MAX_P);
    }
    ~MicroPaddingConfigBase() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader(headerLabel));
        for (int p = MIN_P; p <= MAX_P; ++p) {
            auto* item = new tsl::elm::MiniListItem(formatLabel(p));
            if (p == currentPadding)
                selectItem(lastSelectedListItem, item, ult::CHECKMARK_SYMBOL);
            item->setClickListener([this, item, p](uint64_t keys) {
                if (keys & KEY_A) {
                    ult::setIniFileValue(configIniPath, "micro", iniKey, std::to_string(p));
                    selectItem(lastSelectedListItem, item, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(item);
        }
        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
        return makeFrame("Micro " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = jumpLabel; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ConfiguratorOverlay>(SwapDepth(2), "Micro");
            return true;
        }
        return false;
    }
};

class MicroHPaddingConfig : public MicroPaddingConfigBase<8, 0, 20> {
public:
    MicroHPaddingConfig() : MicroPaddingConfigBase("horizontal_padding", "水平填充", "水平填充") {}
};

class MicroVPaddingConfig : public MicroPaddingConfigBase<2, 0, 20> {
public:
    MicroVPaddingConfig() : MicroPaddingConfigBase("vertical_padding", "垂直填充", "垂直填充") {}
};

class MicroLabelPaddingConfig : public MicroPaddingConfigBase<0, 4, 12> {
    int autoDefault;
protected:
    std::string formatLabel(int p) const override {
        std::string s = std::to_string(p) + " px";
        if (p == autoDefault) s += " (默认)";
        return s;
    }
public:
    MicroLabelPaddingConfig() : MicroPaddingConfigBase("label_padding", "标签填充", "标签填充") {
        const std::string fsVal = ult::parseValueFromIniSection(configIniPath, "micro", "handheld_font_size");
        const int fs = fsVal.empty() ? 15 : atoi(fsVal.c_str());
        autoDefault = (fs <= 16) ? 6 : (fs <= 20 ? 8 : 10);
        // currentPadding==0 表示 "auto"；将显示值设置为 autoDefault
        if (currentPadding == 0) currentPadding = autoDefault;
    }
};

// =============================================================================
// 字号选择器
// =============================================================================
class FontSizeSelector : public tsl::Gui {
private:
    std::string modeName;
    std::string fontType;
    ModeFlags   flags;
    std::string title;

public:
    FontSizeSelector(const std::string& mode, const std::string& type)
        : modeName(mode), fontType(type), flags(mode) {
        // 为标题和返回导航跳转构建可读标题。
        if (fontType == "handheld")
            title = "掌机模式 字号";
        else if (fontType == "docked")
            title = "底座字号";
        else if (fontType == "docked_1080p")
            title = "1080p 底座字号";
        else {
            title = fontType;
            title[0] = std::toupper(title[0]);
            title += " 字号";
        }
    }
    ~FontSizeSelector() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader(title));

        const std::string section = modeToSection(modeName);
        const std::string keyName = fontType + "_font_size";
        const std::string currentValue = ult::parseValueFromIniSection(configIniPath, section, keyName);

        // 每种类型的默认大小；1080p 默认值约为 720p 底座默认值的 1.5 倍。
        int defaultSize;
        if (fontType == "docked_1080p")
            defaultSize = flags.isFPSCounter ? 60 : 22;
        else
            defaultSize = flags.isFPSCounter ? 40 : 15;

        const int currentSize = currentValue.empty() ? defaultSize : atoi(currentValue.c_str());

        const int minSize = 8;
        // 1080p 允许更大的值，因为 1px = 1px（无 1.5× VI 缩放）。
        int maxSize;
        if (fontType == "docked_1080p")
            maxSize = flags.isFPSCounter ? 225 : (flags.isMini ? 33 : 27);
        else
            maxSize = flags.isFPSCounter ? 150 : (flags.isMini ? 22 : 18);

        for (int size = minSize; size <= maxSize; size++) {
            auto* sizeItem = new tsl::elm::MiniListItem(std::to_string(size) + " pt");
            if (size == currentSize)
                selectItem(lastSelectedListItem, sizeItem, ult::CHECKMARK_SYMBOL);
            sizeItem->setClickListener([this, sizeItem, size, keyName, section](uint64_t keys) {
                if (keys & KEY_A) {
                    ult::setIniFileValue(configIniPath, section, keyName, std::to_string(size));
                    selectItem(lastSelectedListItem, sizeItem, ult::CHECKMARK_SYMBOL);
                    return true;
                }
                return false;
            });
            list->addItem(sizeItem);
        }

        list->jumpToItem("", ult::CHECKMARK_SYMBOL, false);
        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置 " + ult::DIVIDER_SYMBOL + " 字号", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = title; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<FontSizeConfig>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 字号配置
// =============================================================================
class FontSizeConfig : public tsl::Gui {
private:
    std::string modeName;
    ModeFlags   flags;

public:
    FontSizeConfig(const std::string& mode) : modeName(mode), flags(mode) {}

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("字号"));

        const std::string section = modeToSection(modeName);
        const int defaultSize      = flags.isFPSCounter ? 40 : 15;
        const int default1080pSize = flags.isFPSCounter ? 60 : 22;

        auto makeItem = [&](const std::string& label, const std::string& key,
                            const std::string& type, int defSz) {
            const std::string val = ult::parseValueFromIniSection(configIniPath, section, key);
            const int sz = val.empty() ? defSz : atoi(val.c_str());
            auto* item = new tsl::elm::ListItem(label);
            item->setValue(std::to_string(sz) + " pt");
            item->setClickListener([this, type](uint64_t keys) {
                if (keys & KEY_A) {
                    tsl::changeTo<FontSizeSelector>(modeName, type);
                    return true;
                }
                return false;
            });
            list->addItem(item);
        };

        makeItem("掌机模式",     "handheld_font_size",     "handheld",    defaultSize);
        makeItem("底座",       "docked_font_size",       "docked",      defaultSize);
        makeItem("1080p 底座", "docked_1080p_font_size", "docked_1080p", default1080pSize);

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        clearJump();

        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            tsl::goBack();
            return true;
        }
        return false;
    }
};

// =============================================================================
// 颜色选择器
// =============================================================================
class ColorSelector : public tsl::Gui {
private:
    std::string modeName;
    std::string modeTitle;
    std::string colorKey;
    std::string defaultValue;
    ModeFlags   flags;
    bool isBackgroundColor;
    bool isTextBasedColor;

public:
    ColorSelector(const std::string& mode, const std::string& title,
                  const std::string& key, const std::string& def)
        : modeName(mode), modeTitle(title), colorKey(key), defaultValue(def), flags(mode) {
        isBackgroundColor = (key == "background_color" || key == "focus_background_color" ||
                            (flags.isFPSGraph && (key == "fps_counter_color" || key == "dashed_line_color")));
        isTextBasedColor  = (key == "text_color" || key == "separator_color" || key == "cat_color" ||
                            key == "cat_color_1" || key == "cat_color_2" ||
                            (flags.isFPSGraph && (key == "border_color" || key == "max_fps_text_color" ||
                             key == "min_fps_text_color" || key == "main_line_color" ||
                             key == "rounded_line_color" || key == "perfect_line_color")));
    }
    ~ColorSelector() { lastSelectedListItem = nullptr; }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader(modeTitle));

        const std::string section = modeToSection(modeName);
        std::string currentValue = ult::parseValueFromIniSection(configIniPath, section, colorKey);
        if (currentValue.empty()) currentValue = defaultValue;

        const std::string currentColorWithoutAlpha = extractColorWithoutAlpha(currentValue);

        std::string _jumpItemValue;
        for (const auto& color : g_colorPalette) {
            auto* colorItem = new MiniColorSwatchListItem(color.first);
            const std::string hexRgb = extractColorWithoutAlpha(color.second); // #RGB
            const tsl::Color swatchColor = hexToSwatchColor(hexRgb);
            colorItem->setSwatchColor(swatchColor);
            colorItem->setValue(COLOR_SWATCH);

            const bool isSelected = (hexRgb == currentColorWithoutAlpha);
            if (isSelected) {
                colorItem->setValue(COLOR_SWATCH + " " + ult::CHECKMARK_SYMBOL);
                lastSelectedListItem = colorItem;
                _jumpItemValue = COLOR_SWATCH + " " + ult::CHECKMARK_SYMBOL;
            }

            colorItem->setClickListener([this, colorItem, color, section, hexRgb, swatchColor](uint64_t keys) {
                if (keys & KEY_A) {
                    std::string valueToSave = color.second;
                    if (isBackgroundColor) {
                        std::string existing = ult::parseValueFromIniSection(configIniPath, section, colorKey);
                        char alpha = (existing.length() == 5) ? existing[4]
                                   : (defaultValue.length() == 5) ? defaultValue[4]
                                   : '9';
                        valueToSave = setAlphaInColor(color.second, alpha);
                    } else if (isTextBasedColor) {
                        valueToSave = setAlphaInColor(color.second, 'F');
                    }
                    ult::setIniFileValue(configIniPath, section, colorKey, valueToSave);

                    if (lastSelectedListItem && lastSelectedListItem != colorItem) {
                        for (const auto& c : g_colorPalette) {
                            if (lastSelectedListItem->getText() == c.first) {
                                lastSelectedListItem->setValue(COLOR_SWATCH);
                                break;
                            }
                        }
                    }
                    colorItem->setValue(COLOR_SWATCH + " " + ult::CHECKMARK_SYMBOL);
                    lastSelectedListItem = colorItem;
                    return true;
                }
                return false;
            });
            list->addItem(colorItem);
        }

        list->jumpToItem("", _jumpItemValue, false);
        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置 " + ult::DIVIDER_SYMBOL + " 颜色", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            jumpItemName = modeTitle; jumpItemValue = ""; jumpItemExactMatch = false;
            tsl::swapTo<ColorConfig>(SwapDepth(2), modeName);
            return true;
        }
        return false;
    }
};

// =============================================================================
// 颜色配置
// =============================================================================
class ColorConfig : public tsl::Gui {
private:
    std::string modeName;
    ModeFlags   flags;

    std::string getCurrentColor(const std::string& key, const std::string& def) const {
        const std::string section = modeToSection(modeName);
        std::string value = ult::parseValueFromIniSection(configIniPath, section, key);
        return value.empty() ? def : value;
    }

    void addColorItem(tsl::elm::List* list, const std::string& label,
                      const std::string& key, const std::string& def) {
        const std::string currentColor = getCurrentColor(key, def);
        auto* item = new ColorSwatchListItem(label);
        item->setValue(COLOR_SWATCH);
        item->setSwatchColor(hexToSwatchColor(extractColorWithoutAlpha(currentColor)));
        item->setClickListener([this, label, key, def](uint64_t keys) {
            if (keys & KEY_A) {
                tsl::changeTo<ColorSelector>(modeName, label, key, def);
                return true;
            }
            return false;
        });
        list->addItem(item);
    }

    void addColorWithAlpha(tsl::elm::List* list, const std::string& colorLabel,
                           const std::string& key, const std::string& def,
                           const std::string& alphaLabel) {
        const std::string currentColor = getCurrentColor(key, def);
        auto* colorItem = new ColorSwatchListItem(colorLabel);
        colorItem->setValue(COLOR_SWATCH);
        colorItem->setSwatchColor(hexToSwatchColor(extractColorWithoutAlpha(currentColor)));
        colorItem->setClickListener([this, colorLabel, key, def](uint64_t keys) {
            if (keys & KEY_A) {
                tsl::changeTo<ColorSelector>(modeName, colorLabel, key, def);
                return true;
            }
            return false;
        });
        list->addItem(colorItem);

        auto* alphaItem = new tsl::elm::ListItem(alphaLabel);
        alphaItem->setValue(getAlphaPercentage(currentColor));
        alphaItem->setClickListener([this, key, alphaLabel](uint64_t keys) {
            if (keys & KEY_A) {
                tsl::changeTo<AlphaSelector>(modeName, key, alphaLabel);
                return true;
            }
            return false;
        });
        list->addItem(alphaItem);
    }

public:
    ColorConfig(const std::string& mode) : modeName(mode), flags(mode) {}

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("颜色"));

        if (!flags.isFull) {
            addColorWithAlpha(list, "背景颜色", "background_color",       "#000A", "背景透明度");
            if (flags.isMini || flags.isMicro || flags.isFPSCounter || flags.isFPSGraph || flags.isGameRes)
                addColorWithAlpha(list, "焦点颜色",  "focus_background_color", "#000F", "焦点透明度");
        } else {
            addColorWithAlpha(list, "背景颜色", "background_color",       "#000A", "背景透明度");
        }

        addColorItem(list, "文字颜色", "text_color", "#FFFF");

        if (flags.isFPSGraph) {
            addColorItem(list, "分类颜色", "cat_color", "#2DFF");

            struct FPSGraphColorSetting {
                const char* name; const char* key; const char* def; bool hasAlpha;
            };
            static const FPSGraphColorSetting fpsGraphColors[] = {
                {"FPS 计数器",   "fps_counter_color",  "#888C", true},
                {"边框",       "border_color",        "#2DFF", false},
                {"虚线",       "dashed_line_color",   "#8888", true},
                {"最大 FPS 文字","max_fps_text_color",  "#FFFF", false},
                {"最小 FPS 文字","min_fps_text_color",  "#FFFF", false},
                {"主线",        "main_line_color",     "#FFFF", false},
                {"圆角线",      "rounded_line_color",  "#F0FF", false},
                {"完美线",      "perfect_line_color",  "#0C0F", false},
            };
            for (const auto& c : fpsGraphColors) {
                if (c.hasAlpha)
                    addColorWithAlpha(list,
                        std::string(c.name) + " 颜色", c.key, c.def,
                        std::string(c.name) + " 透明度");
                else
                    addColorItem(list, std::string(c.name) + " 颜色", c.key, c.def);
            }

        } else if (flags.isFull) {
            addColorItem(list, "分类颜色 1", "cat_color_1",    "#8FFF");
            addColorItem(list, "分类颜色 2", "cat_color_2",    "#2DFF");
            addColorItem(list, "分隔线颜色",  "separator_color","#888F");

        } else if (flags.isMini || flags.isMicro) {
            addColorItem(list, "分类颜色", "cat_color",       "#2DFF");
            addColorItem(list, "分隔线颜色", "separator_color", "#888F");

        } else if (flags.isGameRes) {
            addColorItem(list, "分类颜色", "cat_color", "#2DFF");
        }

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        clearJump();

        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            tsl::goBack();
            return true;
        }
        return false;
    }
};

// =============================================================================
// 显示配置（仅 Mini/Micro）
// =============================================================================
class ShowConfig : public tsl::Gui {
private:
    std::string modeName;
    bool isMiniMode;
    std::vector<std::string> elementOrder;
    std::unordered_set<std::string> enabledElements;

public:
    ShowConfig(const std::string& mode) : modeName(mode) {
        isMiniMode = (mode == "Mini");
    }

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader(
            "条目 " + ult::DIVIDER_SYMBOL + " \uE0E3 下移 " + ult::DIVIDER_SYMBOL + " \uE0E2 上移"));

        const std::string section = isMiniMode ? "mini" : "micro";
        std::string showValue  = ult::parseValueFromIniSection(configIniPath, section, "show");
        std::string orderValue = ult::parseValueFromIniSection(configIniPath, section, "element_order");

        if (showValue.empty())
            showValue = isMiniMode ? "DTC+BAT+CPU+GPU+RAM+TMP+FPS+RES" : "FPS+CPU+GPU+RAM+TMP+BAT+DTC";
        convertToUpper(showValue);

        enabledElements.clear();
        {
            ult::StringStream ss(showValue);
            std::string item;
            while (ss.getline(item, '+'))
                if (!item.empty()) enabledElements.insert(item);
        }

        static constexpr std::string_view miniElements[]  = {"DTC","BAT","CPU","GPU","RAM","MEM","READ","SOC","TMP","FPS","RES"};
        static constexpr std::string_view microElements[] = {"FPS","CPU","GPU","RAM","READ","SOC","TMP","RES","BAT","DTC"};
        const auto* allElements    = isMiniMode ? miniElements : microElements;
        const size_t allElementsSize = isMiniMode ? std::size(miniElements) : std::size(microElements);

        elementOrder.clear();
        if (!orderValue.empty()) {
            convertToUpper(orderValue);
            ult::StringStream orderSS(orderValue);
            std::string orderItem;
            while (orderSS.getline(orderItem, '+'))
                if (!orderItem.empty()) elementOrder.push_back(orderItem);
        } else {
            for (size_t i = 0; i < allElementsSize; i++) {
                auto elem = allElements[i];
                if (!isMiniMode && elem == "MEM") continue;
                elementOrder.emplace_back(elem);
            }
        }

        for (size_t i = 0; i < elementOrder.size(); i++) {
            const std::string& element = elementOrder[i];
            const bool isEnabled = enabledElements.count(element) > 0;

            auto* elementItem = new tsl::elm::MiniListItem(element);
            elementItem->enableShortHoldKey();
            elementItem->enableLongHoldKey();
            elementItem->setValue(isEnabled ? ult::ON : ult::OFF, !isEnabled);

            elementItem->setClickListener([this, elementItem, element](uint64_t keys) {
                static bool hasNotTriggeredAnimation = false;
                if (hasNotTriggeredAnimation) {
                    elementItem->triggerClickAnimation();
                    hasNotTriggeredAnimation = false;
                }
                if (keys & KEY_A) {
                    // 动态条目（FPS、RES、READ）仅在游戏运行时渲染，
                    // 因此不计为"始终可见"。阻止关闭始终显示的条目，
                    // 如果关闭后没有始终显示的条目保持启用状态。
                    static const std::unordered_set<std::string> dynamicElements = {"FPS", "RES", "READ"};
                    if (enabledElements.count(element)) {
                        // 关闭 — 防止留下零个始终显示的条目
                        if (dynamicElements.count(element) == 0) {
                            // 计算移除后仍保持启用的始终显示条目
                            int alwaysOnAfter = 0;
                            for (const auto& e : enabledElements) {
                                if (e != element && dynamicElements.count(e) == 0)
                                    alwaysOnAfter++;
                            }
                            if (alwaysOnAfter == 0) return true; // 阻止：最后一个始终显示的条目
                        }
                        enabledElements.erase(element);
                    } else {
                        enabledElements.insert(element);
                    }
                    updateShowAndOrder();
                    jumpItemName = element; jumpItemValue = ""; jumpItemExactMatch = true;
                    hasNotTriggeredAnimation = true;
                    tsl::swapTo<ShowConfig>(SwapDepth(1), modeName);
                    return true;
                }
                if (keys & KEY_Y || keys & KEY_X) {
                    size_t currentPos = 0;
                    for (size_t j = 0; j < elementOrder.size(); j++) {
                        if (elementOrder[j] == element) { currentPos = j; break; }
                    }
                    if (keys & KEY_X) {
                        if (currentPos > 0) {
                            std::swap(elementOrder[currentPos], elementOrder[currentPos - 1]);
                        } else {
                            const std::string temp = elementOrder[0];
                            for (size_t j = 0; j < elementOrder.size() - 1; j++)
                                elementOrder[j] = elementOrder[j + 1];
                            elementOrder.back() = temp;
                        }
                        triggerMoveFeedback();
                    } else if (keys & KEY_Y) {
                        if (currentPos < elementOrder.size() - 1) {
                            std::swap(elementOrder[currentPos], elementOrder[currentPos + 1]);
                        } else {
                            const std::string temp = elementOrder.back();
                            for (size_t j = elementOrder.size() - 1; j > 0; j--)
                                elementOrder[j] = elementOrder[j - 1];
                            elementOrder[0] = temp;
                        }
                        triggerMoveFeedback();
                    }
                    updateShowAndOrder();
                    jumpItemName = element; jumpItemValue = ""; jumpItemExactMatch = true;
                    tsl::swapTo<ShowConfig>(SwapDepth(1), modeName);
                    return true;
                }
                return false;
            });
            list->addItem(elementItem);
        }

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        clearJump();

        return makeFrame(modeName + " " + ult::DIVIDER_SYMBOL + " 配置", list);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            tsl::goBack();
            return true;
        }
        return false;
    }

private:
    void updateShowAndOrder() {
        std::string newShowValue, newOrderValue;
        bool showFirst = true, orderFirst = true;
        for (const std::string& element : elementOrder) {
            if (!orderFirst) newOrderValue += "+";
            newOrderValue += element;
            orderFirst = false;
            if (enabledElements.count(element)) {
                if (!showFirst) newShowValue += "+";
                newShowValue += element;
                showFirst = false;
            }
        }
        const std::string section = isMiniMode ? "mini" : "micro";
        ult::setIniFileValue(configIniPath, section, "show",          newShowValue);
        ult::setIniFileValue(configIniPath, section, "element_order", newOrderValue);
    }
};

// =============================================================================
// 主配置器覆盖层
// =============================================================================
class ConfiguratorOverlay : public tsl::Gui {
private:
    std::string modeName;
    ModeFlags   flags;

    int getCurrentRefreshRate() const {
        const std::string section = modeToSection(modeName);
        const std::string value = ult::parseValueFromIniSection(configIniPath, section, "refresh_rate");
        const int defaultRate = (flags.isFPSCounter || flags.isFPSGraph) ? 5 : 3;
        return value.empty() ? defaultRate : atoi(value.c_str());
    }

    int getCurrentFramePadding() const {
        const std::string section = modeToSection(modeName);
        if (section.empty()) return 10;
        const std::string value = ult::parseValueFromIniSection(configIniPath, section, "frame_padding");
        return value.empty() ? 10 : atoi(value.c_str());
    }

    int getCurrentMicroHPadding() const {
        const std::string value = ult::parseValueFromIniSection(configIniPath, "micro", "horizontal_padding");
        return value.empty() ? 8 : std::clamp(atoi(value.c_str()), 0, 20);
    }

    int getCurrentMicroVPadding() const {
        const std::string value = ult::parseValueFromIniSection(configIniPath, "micro", "vertical_padding");
        return value.empty() ? 2 : std::clamp(atoi(value.c_str()), 0, 20);
    }

    int getCurrentMicroLabelPadding() const {
        const std::string value = ult::parseValueFromIniSection(configIniPath, "micro", "label_padding");
        if (value.empty()) {
            const std::string fsVal = ult::parseValueFromIniSection(configIniPath, "micro", "handheld_font_size");
            const int fs = fsVal.empty() ? 15 : atoi(fsVal.c_str());
            return (fs <= 16) ? 6 : 8;
        }
        return std::clamp(atoi(value.c_str()), 4, 12);
    }

    std::string getDTCFormatName(const std::string& formatStr) const {
        if (formatStr == ult::OPTION_SYMBOL) return ult::OPTION_SYMBOL;
        for (const auto& format : getDTCFormatsFlat())
            if (format.second == formatStr) return format.first;
        return formatStr;
    }

    std::string getCurrentDTCFormatLabel(int slot) const {
        if (!(flags.isMini || flags.isMicro)) return "";
        const std::string section = modeToSection(modeName);
        const std::string iniKey  = (slot == 2) ? "dtc_format_2" : "dtc_format_1";
        std::string value = ult::parseValueFromIniSection(configIniPath, section, iniKey);
        if (value.empty()) {
            const std::string legacy = ult::parseValueFromIniSection(configIniPath, section, "dtc_format");
            if (!legacy.empty()) {
                const size_t divPos = legacy.find(ult::DIVIDER_SYMBOL);
                if (divPos != std::string::npos)
                    value = (slot == 1) ? legacy.substr(0, divPos)
                                        : legacy.substr(divPos + ult::DIVIDER_SYMBOL.length());
                else
                    value = (slot == 1) ? legacy : ult::OPTION_SYMBOL;
            } else {
                if (slot == 1) value = std::string("%a, %b %d");
                else           value = std::string("%l:%M:%S %p");
            }
        }
        return getDTCFormatName(value);
    }

    std::string getCurrentTextAlign() const {
        std::string value = ult::parseValueFromIniSection(configIniPath, "micro", "text_align");
        convertToUpper(value);
        if (value == "LEFT")  return "左";
        if (value == "RIGHT") return "右";
        return "居中";
    }

    std::string getCurrentLayerPosRight() const {
        const std::string section = modeToSection(modeName);
        std::string value = ult::parseValueFromIniSection(configIniPath, section, "layer_width_align");
        convertToUpper(value);
        if (value == "RIGHT")  return "右";
        if (!flags.isFull && value == "CENTER") return "居中";
        return "左";
    }

    std::string getCurrentLayerPosBottom() const {
        const std::string section = modeToSection(modeName);
        std::string value = ult::parseValueFromIniSection(configIniPath, section, "layer_height_align");
        convertToUpper(value);
        if (value == "BOTTOM") return "下";
        if (!flags.isMicro && value == "CENTER") return "居中";
        return "上";
    }

    std::string cycleTextAlign() {
        const std::string current = getCurrentTextAlign();
        const std::string next = (current == "左") ? "居中" : (current == "居中") ? "右" : "左";
        ult::setIniFileValue(configIniPath, "micro", "text_align", next);
        return next;
    }

    std::string cycleLayerPosRight() {
        const std::string section = modeToSection(modeName);
        const std::string current = getCurrentLayerPosRight();
        std::string next;
        if (flags.isFull)
            next = (current == "左") ? "右" : "左";
        else
            next = (current == "左") ? "居中" : (current == "居中") ? "右" : "左";
        const std::string value = (next == "右") ? "right" : (next == "居中" ? "center" : "left");
        ult::setIniFileValue(configIniPath, section, "layer_width_align", value);
        return next;
    }

    std::string cycleLayerPosBottom() {
        const std::string section = modeToSection(modeName);
        const std::string current = getCurrentLayerPosBottom();
        std::string next;
        if (flags.isMicro)
            next = (current == "上") ? "下" : "上";
        else
            next = (current == "上") ? "居中" : (current == "居中") ? "下" : "上";
        const std::string value = (next == "下") ? "bottom" : (next == "居中" ? "center" : "top");
        ult::setIniFileValue(configIniPath, section, "layer_height_align", value);
        return next;
    }

public:
    ConfiguratorOverlay(const std::string& mode) : modeName(mode), flags(mode) {}

    virtual tsl::elm::Element* createUI() override {
        auto* list = new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("配置"));

        // 条目（仅 Mini/Micro）
        if (flags.isMini || flags.isMicro) {
            auto* showSettings = new tsl::elm::ListItem("条目");
            showSettings->setValue(ult::DROPDOWN_SYMBOL);
            showSettings->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<ShowConfig>(modeName); return true; }
                return false;
            });
            list->addItem(showSettings);
        }

        // 开关（所有模式）
        {
            auto* toggles = new tsl::elm::ListItem("开关");
            toggles->setValue(ult::DROPDOWN_SYMBOL);
            toggles->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<TogglesConfig>(modeName); return true; }
                return false;
            });
            list->addItem(toggles);
        }

        // 颜色（所有模式）
        {
            auto* colors = new tsl::elm::ListItem("颜色");
            colors->setValue(ult::DROPDOWN_SYMBOL);
            colors->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<ColorConfig>(modeName); return true; }
                return false;
            });
            list->addItem(colors);
        }

        // 字号（Mini/Micro/FPS 计数器）
        if (flags.isMini || flags.isMicro || flags.isFPSCounter) {
            auto* fontSizes = new tsl::elm::ListItem("字号");
            fontSizes->setValue(ult::DROPDOWN_SYMBOL);
            fontSizes->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<FontSizeConfig>(modeName); return true; }
                return false;
            });
            list->addItem(fontSizes);
        }

        // 刷新率（所有模式）
        {
            auto* refreshRate = new tsl::elm::ListItem("刷新率");
            refreshRate->setValue(std::to_string(getCurrentRefreshRate()) + " Hz");
            refreshRate->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<RefreshRateConfig>(modeName); return true; }
                return false;
            });
            list->addItem(refreshRate);
        }

        // 时间格式（仅 Mini/Micro）
        if (flags.isMini || flags.isMicro) {
            auto* dtcFormat1 = new tsl::elm::ListItem("时间格式 1");
            dtcFormat1->setValue(getCurrentDTCFormatLabel(1));
            dtcFormat1->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<DTCFormatConfig>(modeName, 1); return true; }
                return false;
            });
            list->addItem(dtcFormat1);

            auto* dtcFormat2 = new tsl::elm::ListItem("时间格式 2");
            dtcFormat2->setValue(getCurrentDTCFormatLabel(2));
            dtcFormat2->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<DTCFormatConfig>(modeName, 2); return true; }
                return false;
            });
            list->addItem(dtcFormat2);
        }

        // 帧填充（Mini/游戏分辨率/FPS 计数器/FPS 图表）
        if (flags.isMini || flags.isGameRes || flags.isFPSCounter || flags.isFPSGraph) {
            auto* framePadding = new tsl::elm::ListItem("帧填充");
            framePadding->setValue(std::to_string(getCurrentFramePadding()) + " px");
            framePadding->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<FramePaddingConfig>(modeName); return true; }
                return false;
            });
            list->addItem(framePadding);
        }

        // 模式特定位置设置
        if (flags.isMicro) {
            auto* textAlign = new tsl::elm::ListItem("文字对齐");
            textAlign->setValue(getCurrentTextAlign());
            textAlign->setClickListener([this, textAlign](uint64_t keys) {
                if (keys & KEY_A) { textAlign->setValue(cycleTextAlign()); return true; }
                return false;
            });
            list->addItem(textAlign);

            auto* layerPos = new tsl::elm::ListItem("垂直位置");
            layerPos->setValue(getCurrentLayerPosBottom());
            layerPos->setClickListener([this, layerPos](uint64_t keys) {
                if (keys & KEY_A) { layerPos->setValue(cycleLayerPosBottom()); return true; }
                return false;
            });
            list->addItem(layerPos);

            auto* hPadding = new tsl::elm::ListItem("水平填充");
            hPadding->setValue(std::to_string(getCurrentMicroHPadding()) + " px");
            hPadding->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<MicroHPaddingConfig>(); return true; }
                return false;
            });
            list->addItem(hPadding);

            auto* vPadding = new tsl::elm::ListItem("垂直填充");
            vPadding->setValue(std::to_string(getCurrentMicroVPadding()) + " px");
            vPadding->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<MicroVPaddingConfig>(); return true; }
                return false;
            });
            list->addItem(vPadding);

            auto* lPadding = new tsl::elm::ListItem("标签填充");
            lPadding->setValue(std::to_string(getCurrentMicroLabelPadding()) + " px");
            lPadding->setClickListener([this](uint64_t keys) {
                if (keys & KEY_A) { tsl::changeTo<MicroLabelPaddingConfig>(); return true; }
                return false;
            });
            list->addItem(lPadding);

        } else if (flags.isFull) {
            auto* layerPos = new tsl::elm::ListItem("水平位置");
            layerPos->setValue(getCurrentLayerPosRight());
            layerPos->setClickListener([this, layerPos](uint64_t keys) {
                if (keys & KEY_A) { layerPos->setValue(cycleLayerPosRight()); return true; }
                return false;
            });
            list->addItem(layerPos);
        }

        // 模式快捷键
        {
            const int slotIdx = modeComboIndexFor(modeName);
            if (slotIdx >= 0) {
                std::string comboDisplay = readModeCombo(slotIdx);
                if (comboDisplay.empty()) comboDisplay = ult::OPTION_SYMBOL;
                else ult::convertComboToUnicode(comboDisplay);

                auto* modeCombo = new tsl::elm::ListItem("模式快捷键", comboDisplay);
                modeCombo->setClickListener([this](uint64_t keys) {
                    if (keys & KEY_A) { tsl::changeTo<ModeComboConfig>(modeName); return true; }
                    return false;
                });
                list->addItem(modeCombo);
            }
        }

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(std::memory_order_acquire));
        clearJump();

        auto* rootFrame = new tsl::elm::OverlayFrame("状态监控", modeName);
        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B) {
            triggerExitFeedback();
            lastSelectedItem = modeName;
            tsl::swapTo<MainMenu>();
            return true;
        }
        return false;
    }
};
