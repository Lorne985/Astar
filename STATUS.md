# Astar 项目状态

## 当前进度

- raylib 网格窗口、画墙和擦除功能已完成。
- 起点 `s`、终点 `g` 使用 `std::optional<GridPos>` 独立保存。
- 地图 `Grid<CellState>` 只保存 `Empty` 和 `Wall`。
- 已实现边界检查、四方向邻居和曼哈顿距离。
- 已建立 `gScore`、`cameFrom` 和最小优先 Open 集合。
- 起终点或地图变化时会重置搜索状态。
- 已实现 A* 节点展开、Closed 集合和无路径判断。
- `fScore` 相同时优先选择 `hScore` 更小的节点。
- 已实现定时单步搜索和最短路径可视化。
- 当前项目可以正常编译。

## TODO

- [ ] 显示 `Unready`、`Searching`、`Found` 和 `NoPath` 状态及颜色图例。
- [ ] 增加暂停、继续、单步执行和搜索速度调节。
- [ ] 为 Open 集合增加独立颜色，区分待展开与已展开节点。
- [ ] 搜索重置时同时清零计时器，统一搜索状态切换逻辑。
- [ ] 将输入处理、单步 A* 和绘制逻辑拆分为独立函数。
- [ ] 验证空地图、绕障碍、完全阻断和编辑后重新搜索等场景。

## 构建

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
.\build\astar_visualizer.exe
```

