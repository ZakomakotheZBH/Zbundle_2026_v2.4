# Zbundle 2026 v2.4: The OS combo web and terminal!
Welcome to Zbundle! here are 2 great OS for your computer and web browser


1. ## ZTK Business Edition v3.0 - Ultimate Edition

An ISO C11 compliant, multi-threaded text user interface (TUI) operating layer featuring DeepSeek AI integration, text-based games, terminal browsing, and disk management utilities.

## Features
* 🤖 **ZewAI Bot:** Powered by DeepSeek API for smart terminal assistance.
* 🎮 **Built-in Games:** Snake, Tetris, Pong, and Space Invaders.
* 🌐 **Terminal Browser:** Links/Lynx-style text web browsing.
* 💾 **Disk Management:** Direct mount, format, and partition controls.
* ⚙️ **Self-Modifying:** Modify and recompile code from within the running system.

### Compilation & Usage

 Linux & macOS
Ensure you have the `ncurses` development libraries installed:
```bash
# Ubuntu/Debian
sudo apt install libncurses5-dev libncursesw5-dev

# Compile
gcc -std=c11 -Wall -Wextra -o ztk ztk.c -lpthread -lncurses

# Run
./ztk --ai-token YOUR_DEEPSEEK_TOKEN
```

Windows
```bash
gcc -std=c11 -Wall -Wextra -o ztk ztk.c -lpthread
./ztk
```

### License
Distributed under the MIT License. See `LICENSE` for more information.

2. ## ZewpolOS 7 · Web Edition

A sleek, retro-futuristic web terminal experience styled with sharp glassmorphism aesthetics. This interactive client-side shell simulates a fully functional command-line interface mimicking **ZewpolOS 7**, bundled with real-time status telemetry and micro-widget extensions.

---

### 🚀 Key Features

* **Glassmorphism Shell Architecture:** Modern responsive grid with rich blur dynamics, glowing typography anchors, and native UI styling.
* **Integrated Telemetry Panel:** Dynamic performance tracking modules built alongside a modular game launch architecture.
* **Fira/Cascadia Layouts:** Clean typography stack mimicking real production UNIX consoles.
* **Lightweight Single-File Footprint:** Pure dependency-free core stack making use of standard Content Delivery Networks (CDNs) for high reliability.

---

### 🛠️ Tech Stack

* **Markup & Structure:** HTML5 Semantic Engine
* **Styling Framework:** Custom CSS3 with dynamic variables and CSS Backdrop-Filters (`blur`)
* **Design Accents:** Font Awesome 6.0 SVG Integration

---

### 📦 Getting Started

You can load and configure the interface locally without building compilation assets.

 Prerequisites
A modern evergreen browser supporting CSS custom properties and the Web Backdrop Filter API (e.g., Google Chrome, Mozilla Firefox, Microsoft Edge, Safari).

 Local Installation
1. Clone this repository to your machine:
```bash
git clone https://github.com/ZakomakotheZBH/ZTK-3.0-and-ZewpolOS7_web_download/
```
2. Open the main file layout inside your preferred browser:
```bash
# macOS
open index.html

# Linux
xdg-open index.html

# Windows
start index.html
```

---

### 📂 Project Structure

```text
ZTK-3.0-and-ZewpolOS7_web_download/
├── ztk_bootloader.c    
└── README.md
└── ZewpolOS_web_7.html
```

---

### 🔧 Deployment

Because the application operates entirely client-side, you can host it for free on several popular distribution networks:

 Deploying via GitHub Pages
1. Push your code framework directly to your target GitHub repository.
2. Head into the **Settings** tab of the page layout.
3. Locate the **Pages** menu inside the sidebar dashboard.
4. Set the build source parameter tracking over to the `main` or `master` branch.
5. Hit **Save** to distribute your link live across the cloud engine!

---

### 📝 Future Scope / Feature Map

- [ ] Connect interactive JavaScript logic layer handling core execution strings.
- [ ] Build multi-threaded custom file tracking systems (`mkdir`, `ls`, `cd`).
- [ ] Map full engine controls across terminal game modules.
- [ ] Expand UI scaling mechanics for smaller tablet and mobile layouts.

---

### 📄 License

This asset architecture is open-source software and is licensed freely under the terms of the **MIT License**.
