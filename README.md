# ASCIIpaper

ASCIIpaper is a lightweight ASCII desktop wallpaper engine written in C++20. It runs efficiently in the background as a calm ambient simulation without draining your CPU.

Currently, ASCIIpaper is strictly a Windows application.

## Usage & Examples

Run the compiled executable. ASCIIpaper will attach to the desktop background and create a system tray icon.
* Right-click the tray icon to switch scenes, toggle weather and system sync, or exit the application.
* To toggle the application on startup, use the provided `toggle_startup.ps1` PowerShell script.

**Aquarium Scene**
![Aquarium Simulation](assets/aquarium_example.gif)

**City Scene with "storm" weather**
![City Simulation](assets/city_storm_example.gif)

## Built-in Worlds

ASCIIpaper includes two primary ecosystems, each populated by unique ASCII entities. Note that only moving entities are listed below.

**Aquarium**
* **Fish:** `<><` and `><>`
* **Hermit Crab:** `@<` (moving right) and `>@` (moving left)
* **Jellyfish:** `( _ )` bell with flowing `~` and `:` tentacles
* **Shrimp:** `j` and `,`
* **Coral:** `&`, `%`, and `Y`
* **Bubbles:** `O` and `o`
* **Seaweed:** `|` stalk with `/` and `\` movement
* **Whale:**
```text
       .
      ":" 
    ___:____     |"\/"|
  ,'        `.    \  /
  |  O        \___/  |
   \________________/
```

**City**
* **Cars:** `[o-o]`
* **Train:** `[_]-` for cargo cars and `[_]>` for the engine car
* **Rain / Snow:** `|` for rain and lightning, `*` and `.` for snow

## Configuration

ASCIIpaper generates a default `config.ini` file in the same directory as the executable. The engine uses a built-in file watcher and CLI commands to hot reload these values instantly. Run `ASCIIpaper set [parameter] [value]` to apply updates.

| Parameter | Type | Description |
| --- | --- | --- |
| scene | string | The active environment (`aquarium` or `city`). |
| weather | string | Weather override (`none`, `rain`, `snow`, `storm`). |
| target_fps | int | The target rendering framerate. |
| system_sync | bool | Toggles CPU/RAM monitoring to dynamically affect simulation variables. |
| fish_count | int | Aquarium: Base number of fish. |
| bubble_count | int | Aquarium: Number of ambient bubbles. |
| jellyfish_count | int | Aquarium: Number of jellyfish. |
| car_count | int | City: Base traffic density. |
| star_count | int | City: Number of background stars. |

## Architecture & Future Goals

The codebase is partitioned to maintain strict abstractions for future cross-platform compatibility, using the Pimpl idiom for OS-specific implementations.

Development is currently focused on the 1.0 release for Windows, but the roadmap includes several major expansions:
* **Linux (X11) Support:** Native background attachment and daemon support for Linux environments.
* **Lua Scripting:** Embedding a Lua virtual machine to allow users to build and hot-load their own custom worlds and ASCII entities without modifying the C++ engine.
* **Packaged Releases:** Simple, one-click installation packages to make onboarding frictionless.

## License
[GPLv3 License](LICENSE)