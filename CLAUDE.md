# MqttMonitor — Claude 工作指南

## 构建命令

```bash
cmake --preset default && cmake --build build
```

输出产物在 `build/MqttMonitor.exe`。

---

## 必须遵守的规则

**新增源文件**：每次新增 `.cpp/.h` 必须同步加入 `CMakeLists.txt` 的 `add_executable` 列表，否则 MOC/UIC 不会处理。

**新增 UI 类**：构造函数里必须调用 `setObjectName("ClassName")`，QSS 通过 objectName 精确定位样式。

**JSON 解析**：所有解析必须容错——字段缺失时直接丢弃消息，不崩溃。必须先确认 `device_id` 存在才创建/更新卡片。

**线程安全**：Paho 消息回调运行在子线程，禁止在回调中操作 UI。跨线程通信只能通过 `MqttBridge` 的 Qt 信号（`QueuedConnection`）。

**对象析构顺序**：`MqttBridge` 必须在 `MqttClient` 之前销毁。`mainwindow.h` 中的声明顺序决定析构顺序，不要调换。

---

## 易踩的坑

- MSVC 需要 `PAHO_MQTTPP_IMPORTS` 宏，否则链接时 `mqtt::message::EMPTY_STR` 等静态成员报 unresolved external。已在 CMakeLists.txt 配置，新增文件不需要重复处理。
- 中文 Windows 默认 GBK 编码，源文件必须保存为 UTF-8，编译选项 `/utf-8` 已全局加入。
- 选中/状态样式用 `setStyleSheet()` 直接写在控件上覆盖，取消选中时调用 `setStyleSheet("")` 回退到全局 QSS，不要在 dark.qss 里处理动态状态。
