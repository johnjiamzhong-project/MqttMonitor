# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# MqttMonitor — Claude 工作指南

## 快速开始

### 构建
```bash
cmake --preset default && cmake --build build
```
输出产物：`build/MqttMonitor.exe`

### 依赖
- Qt 5（Widgets）
- Eclipse Paho MQTT C++
- CMake 3.16+
- vcpkg（用于依赖管理）

---

## 架构概览

```
UI 层（src/ui/）
  ├─ MainWindow（主窗口容器，导航栏 + QStackedWidget）
  ├─ ConfigPanel（MQTT 配置页）
  ├─ DeviceView（设备卡片网格 + 群发面板）
  └─ DeviceCard（单个设备卡片，支持多选）
       ↕ Qt 信号槽（QueuedConnection）

Bridge 层（src/bridge/）
  └─ MqttBridge（线程安全消息代理，Paho 回调 → Qt 信号）
       ↕ 注册 callback

Core 层（src/core/）
  ├─ MqttClient（Paho 封装：连接、订阅、发布）
  ├─ MessageBuffer（线程安全消息队列）
  └─ 存储层（ConfigStore、CardRuleStore、CmdPresetStore）
```

**关键设计：** Paho 消息回调运行在子线程。MqttBridge 通过 `Qt::QueuedConnection` 将消息跨线程安全地投递到主线程，UI 线程只负责显示，不与 Paho 直接交互。

---

## 消息格式契约

所有设备消息应遵循以下 JSON 结构（字段名可通过卡片规则自定义）：

```json
{
  "device_id":  "device001",
  "name":       "温控器-1号",
  "status":     "online",
  "timestamp":  1712750000,
  "data": {
    "temperature": 25.3,
    "humidity": 60
  }
}
```

| 字段 | 说明 |
|------|------|
| `device_id` | 设备唯一标识，**必须存在** 才能创建/更新卡片 |
| `name` | 卡片标题 |
| `status` | 状态值（默认 `online`/`offline`，可通过卡片规则自定义） |
| `timestamp` | Unix 时间戳 |
| `data` | 任意 KV 对象，以列表形式展示在卡片下方 |

---

## 配置持久化

配置存储在 `%APPDATA%\MqttMonitor\`：
- `profiles.json` — MQTT Broker 多套配置切换
- `card_rules.json` — 卡片字段名/状态值映射规则
- `cmd_presets.json` — 指令群发预设 + 上次使用的 topic/payload/QoS

---

## 必须遵守的规则

### 新增源文件

每次新增 `.cpp/.h` 文件时，必须同步加入 `CMakeLists.txt` 的 `add_executable` 列表，否则 CMake 的 `AUTOMOC`/`AUTOUIC` 不会处理该文件。

### 新增 UI 类

构造函数中必须调用 `setObjectName("ClassName")`。QSS 样式表通过 `objectName` 进行精确定位，是连接 C++ 代码与 dark.qss 的关键。

### JSON 解析与容错

所有 MQTT 消息解析必须容错：
- 字段缺失时直接丢弃消息，**禁止崩溃**
- **必须先确认 `device_id` 存在**，才能创建/更新卡片
- 若 device_id 为空或不存在，该消息应被忽略

### 线程安全

Paho MQTT 的消息回调运行在内部子线程。规则：
- **禁止在回调中直接操作 UI**（QWidget、QAbstractModel 等）
- 跨线程通信**只能**通过 `MqttBridge` 的 Qt 信号（使用 `Qt::QueuedConnection`）
- 例：`MqttClient` → `MqttBridge::messageReceived(topic, payload)` 信号 → `MainWindow` 的槽函数（运行在主线程）

### 对象析构顺序

`mainwindow.h` 中的成员声明顺序决定析构顺序：
- **`MqttBridge` 必须在 `MqttClient` 之前销毁**
- 当前 mainwindow.h 的顺序已正确（bridge 先声明，后销毁），**不要调换声明位置**

---

## 关键文件导读

| 文件 | 作用 |
|-----|------|
| `main.cpp` | 应用入口，创建 MainWindow 并启动事件循环 |
| `mainwindow.h/.cpp` | 主窗口容器，管理 MqttClient/MqttBridge 的生命周期（**注意析构顺序**），协调 UI 层 |
| `src/core/MqttClient.h/.cpp` | Paho MQTT 封装，实现连接、订阅、发布、消息回调 |
| `src/bridge/MqttBridge.h/.cpp` | 关键线程安全层，将 Paho 子线程回调转为 Qt 信号，投递到主线程 |
| `src/core/MessageBuffer.h/.cpp` | 线程安全消息队列，批量推送消息给 UI 避免频繁信号 |
| `src/core/ConfigStore.h/.cpp` | MQTT Broker 配置的 JSON 持久化（`profiles.json`） |
| `src/ui/ConfigPanel.h/.cpp` | 配置页面（包含消息列表展示） |
| `src/ui/DeviceView.h/.cpp` | 设备卡片网格 + 群发面板 |
| `src/ui/DeviceCard.h/.cpp` | 单个设备卡片，展示状态/数据，支持多选 |
| `src/core/CardRuleStore.h/.cpp` | 卡片规则持久化（字段名映射、状态值定义） |
| `src/core/CmdPresetStore.h/.cpp` | 群发指令预设持久化 |

**首次理解代码流程的建议顺序：** `main.cpp` → `mainwindow.h` → `MqttBridge.h` → `ConfigPanel` → `DeviceView`

---

## 常见坑点与解决方案

### MSVC 链接错误：unresolved external for static members

**问题：** `mqtt::message::EMPTY_STR` 等 Paho 静态成员出现链接错误。

**解决：** CMakeLists.txt 已配置 `PAHO_MQTTPP_IMPORTS` 宏（用于 Windows DLL）。**新增文件不需要重复添加这个宏**。

### 中文 Windows GBK 编码问题

**问题：** 中文 Windows 默认编码为 GBK，源文件如保存为 GBK 会导致编译出警告或解析错误。

**解决：** 所有 `.cpp`/`.h` 文件**必须保存为 UTF-8**。编译选项 `/utf-8` 已在 CMakeLists.txt 全局配置。

### 动态样式与 QSS 的协调

**问题：** 设备卡片的选中/取消选中状态如何处理样式？

**方案：**
- 选中时：用 `card->setStyleSheet("...")` 直接覆盖该控件的样式
- 取消选中时：用 `card->setStyleSheet("")` 回退到全局 dark.qss 的样式
- **不要在 dark.qss 中处理动态状态**，因为 setStyleSheet 的局部样式优先级更高
