# Astar 项目状态

## 当前进度

- raylib 网格窗口、画墙和擦除功能已完成。
- 起点 `s`、终点 `g` 使用 `std::optional<GridPos>` 独立保存。
- 地图 `Grid<CellState>` 只保存 `Empty` 和 `Wall`。
- 已实现边界检查、四方向邻居和曼哈顿距离。
- 已建立 `gScore`、`cameFrom` 和最小优先 Open 集合。
- 起终点或地图变化时会重置搜索状态。
- 当前项目可以正常编译。

## 尚未实现

- 从 Open 集合取出节点并展开邻居。
- 更新邻居的 `gScore`、`cameFrom` 和 `fScore`。
- Closed 集合、路径重建及搜索过程可视化。

## 当前待修

画墙或擦除时，鼠标停在未变化的格子上仍会每帧重置搜索。下一步应只在地图内容真正改变后调用 `reset_all_state()`。

## 构建

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
.\build\astar_visualizer.exe
```
