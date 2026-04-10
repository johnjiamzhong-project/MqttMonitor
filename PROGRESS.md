# 开发进度

最后更新：2026-04-10

---

## Phase 1 — 核心连通 ✅ 完成

- [x] CMakeLists 集成 Paho MQTT C++
- [x] MqttClient 封装（连接、订阅、发布）
- [x] MessageBuffer 线程安全消息队列
- [x] CMakePresets.json 替代 configure.bat / build.bat（VS Code CMake Tools 集成）
- [x] 编译验证通过（Ninja + MSVC + Qt MOC/UIC 正常运行）

**说明**
- 构建工具链：CMake → Ninja → Qt MOC/UIC + MSVC cl.exe
- vcpkg 管理依赖：Qt5 Widgets、PahoMqttCpp（x64-windows triplet）
- 已加 `PAHO_MQTTPP_IMPORTS` 解决 MSVC dllimport 链接问题

---

## Phase 2 — 基础 UI ✅ 完成

- [x] MqttBridge（Paho 回调 → Qt 信号跨线程转发）
- [x] ConfigPanel（连接配置面板，含输入校验，基于 .ui 文件）
- [x] ConfigPanel 内部布局：QSplitter（左 300px 配置表单 + 右 QPlainTextEdit 消息列表）
- [x] ConfigPanel 新增 `appendMessage(topic, payload)` public slot
- [x] 消息格式：`[hh:mm:ss.zzz]  topic` + 换行 + payload（simplified 压缩为单行）
- [x] MainWindow 主框架：navBar（配置/设备按钮，72px）+ QStackedWidget
- [x] MqttBridge 信号直连 ConfigPanel::appendMessage（移除 MainWindow 中转）
- [x] DeviceView 占位类（QStackedWidget index 1）
- [x] MSVC `/utf-8 /wd4819` 编译选项

**涉及文件**
```
src/bridge/MqttBridge.h / .cpp        ✅
src/ui/ConfigPanel.h / .cpp / .ui     ✅ QSplitter 重构
src/ui/DeviceView.h / .cpp            ✅ 占位
mainwindow.h / .cpp                   ✅ navBar + QStackedWidget
CMakeLists.txt                        ✅
```

---

## Phase 3 — 设备管理 ⏳ 进行中

### 消息 JSON 结构（已确定）

```json
{
  "device_id":  "device001",
  "name":       "温控器-1号",
  "status":     "online",
  "timestamp":  1712750000,
  "data": {
    "temperature": 25.3,
    "humidity":    60
  }
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `device_id` | string | 设备唯一标识 |
| `name` | string | 可读名称（卡片标题） |
| `status` | string | `online` / `offline` / `error` |
| `timestamp` | number | Unix 时间戳（秒） |
| `data` | object | 业务 KV，value 为 number 或 string |

- Topic 格式：`devices/<device_id>/status`
- 离线方式：设备主动发 `status: offline`，不依赖心跳超时

### 实施计划（最小改动，编译可见）

- [x] **Step 1** 新建 `DeviceCard`（`src/ui/DeviceCard.h / .cpp`）
  - QFrame 固定尺寸
  - 顶部：设备名（粗体）+ 状态色块 Label（在线绿 / 离线红 / 未知黄）
  - 下方：`data` KV 用 QFormLayout 逐行展示
  - 公开方法：`void update(const QJsonObject& obj)`

- [x] **Step 2** 改造 `DeviceView`（`src/ui/DeviceView.h / .cpp`）
  - 替换占位 Label，改为 QScrollArea + QGridLayout（每行 3 列）
  - 新增 `updateDevice(const QString& topic, const QString& payload)` public slot
    - 解析 JSON（QJsonDocument）
    - 按 `device_id` 查找已有 DeviceCard → 调用 update()，或新建并插入网格

- [x] **Step 3** `mainwindow.cpp` 加一条 connect
  - `onConnectRequested` 中，mqttBridge_ 信号额外连接到 `deviceView_->updateDevice`

- [x] **Step 4** `CMakeLists.txt` 添加 `DeviceCard.h / .cpp`

### 后续待做

- [ ] 手动添加/删除设备
- [ ] 自动发现（监听通配符 Topic，如 `devices/#`）
- [ ] 批量指令群发

---

## Phase 4 — 扩展模块 ⏳ 后续迭代

- [ ] 日志模块
- [ ] 数据库模块 + 多配置切换
