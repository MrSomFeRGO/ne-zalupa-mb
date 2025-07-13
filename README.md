# VoxelEngine - OpenGL Edition

Пиздатый воксельный движок написанный на мужчинском OpenGL! 🚀

## Особенности

- **Бесконечная генерация мира** с радиусом загрузки 8 чанков
- **Палитра для блоков** - экономия памяти (1 байт на блок вместо 4!)
- **Многопоточная генерация** чанков в фоновом режиме
- **Продвинутый terrain generation** с биомами, пещерами, деревьями и рудами
- **Оптимизированный рендеринг** с frustum culling и mesh генерацией
- **Современный OpenGL 4.5** с shaders и uniform buffers
- **Chunk система 16x16x16** с соседями для оптимизации
- **Палитра текстур** с texture arrays
- **FastNoiseLite** для процедурной генерации

## Зависимости

- **OpenGL 4.5+**
- **SDL3** - окна и ввод
- **GLM** - математика
- **GLAD** - загрузка OpenGL функций
- **FastNoiseLite** - генерация шума
- **STB Image** - загрузка текстур

## Сборка

### Linux/macOS

```bash
# Установка зависимостей (Ubuntu/Debian)
sudo apt install build-essential cmake libsdl3-dev

# Создание external директорий
mkdir -p external/FastNoiseLite external/stb

# Загрузка FastNoiseLite
cd external/FastNoiseLite
wget https://raw.githubusercontent.com/Auburn/FastNoiseLite/master/Cpp/FastNoiseLite.h
cd ../..

# Загрузка STB
cd external/stb
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize2.h
cd ../..

# Сборка
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Windows (Visual Studio)

```cmd
# Установить vcpkg и зависимости
vcpkg install sdl3 glm glad

# Создать external папки и скачать FastNoiseLite + STB
mkdir external\\FastNoiseLite external\\stb

# Сборка
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Структура проекта

```
VoxelEngine/
├── src/
│   ├── core/           # Ядро движка
│   │   ├── Application.h/cpp
│   │   ├── Camera.h/cpp
│   │   └── Input.h/cpp
│   ├── rendering/      # Рендеринг
│   │   ├── VoxelRenderer.h/cpp
│   │   ├── Shader.h/cpp
│   │   ├── Texture.h/cpp
│   │   └── OpenGLUtils.h
│   ├── world/          # Мировая логика
│   │   ├── Chunk.h/cpp
│   │   ├── ChunkManager.h/cpp
│   │   ├── Block.h/cpp
│   │   └── WorldGenerator.h/cpp
│   ├── utils/
│   │   └── Math.h
│   └── main.cpp
├── external/           # Внешние библиотеки
│   ├── FastNoiseLite/
│   └── stb/
├── assets/
│   └── textures/       # Текстуры блоков
├── shaders/            # GLSL шейдеры
└── CMakeLists.txt
```

## Управление

- **WASD** - движение
- **Space** - вверх
- **Shift** - вниз
- **Мышь** - поворот камеры
- **Tab** - переключение захвата мыши
- **Esc** - выход

## Палитра блоков

Каждый чанк использует палитру уникальных блоков:
- **1 байт на блок** вместо 4 байтов
- **Автоматическая оптимизация** палитры
- **До 255 типов блоков** в одном чанке
- **Экономия памяти** до 75%

## Биомы

- **Plains** - равнины с травой и деревьями
- **Desert** - пустыня с песком и кактусами
- **Forest** - лес с большими деревьями
- **Mountains** - горы с камнем
- **Ocean** - океан с водой

## Генерация мира

- **Terrain noise** - основная высота террейна
- **Detail noise** - мелкие детали рельефа
- **Biome noise** - размещение биомов
- **Cave noise** - генерация пещер
- **Tree noise** - размещение деревьев
- **Ore noise** - размещение руд

## Оптимизации

- **Frustum culling** - отсечение невидимых чанков
- **Mesh caching** - кэширование мешей чанков
- **Многопоточность** - генерация в отдельном потоке
- **Palette compression** - сжатие блоков через палитру
- **Neighbor optimization** - оптимизация граней между чанками

## Текстуры

Поместите текстуры в `assets/textures/`:
- `stone.png`, `dirt.png`, `grass_top.png`, `grass_side.png`
- `sand.png`, `wood_top.png`, `wood_side.png`, `leaves.png`
- `water.png`, `cobblestone.png`, `planks.png`
- Размер: **16x16 пикселей** для лучшей производительности

## Расширение

Добавление новых блоков:
1. Добавьте тип в `BlockType` enum
2. Добавьте информацию в `Block::Initialize()`
3. Добавьте текстуры в `assets/textures/`
4. Обновите массив текстур в `VoxelRenderer`

## Производительность

- **~60 FPS** на среднем железе
- **~500+ чанков** загружено одновременно
- **~75% экономии памяти** благодаря палитре
- **Многопоточная генерация** без блокировки рендера

## Баги и TODO

- [ ] Добавить физику воды
- [ ] Улучшить освещение
- [ ] Добавить shadows
- [ ] Оптимизировать texture array loading
- [ ] Добавить block breaking/placing
- [ ] Улучшить LOD система
- [ ] Добавить сериализацию чанков

## Лицензия

MIT License - делай что хочешь, но не забывай про автора! 😎

---

**Сделано с ❤️ и большим количеством кофе ☕**