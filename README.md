# fb

fb is an open-source project for building and operating a 2D MMORPG game server.

It provides the server-side foundation for a persistent online game world—player sessions, in-game systems, and script-driven content such as quests, items, and NPC behavior. The stack is split into microservices (gateway, login, game, internal APIs, write-back, and supporting services) with Lua-driven game logic.

This repository contains **server code and deployment tooling only**. It does not include a game client or commercial game assets.

## Getting started

| Goal | Guide |
|------|-------|
| Run locally on Windows | [Installation (Windows)](https://github.com/boyism80/fb/wiki/Installation-Windows) |
| Deploy on Linux (Docker + Kubernetes) | [Installation (Linux)](https://github.com/boyism80/fb/wiki/Installation-Linux) |
| Understand the design | [Architecture](https://github.com/boyism80/fb/wiki/Architecture) |

## Documentation

Full documentation lives in the [project wiki](https://github.com/boyism80/fb/wiki).

- [Technology Stack](https://github.com/boyism80/fb/wiki/Technology-Stack)
- [Map Division](https://github.com/boyism80/fb/wiki/Map-Division) · [Thread Model](https://github.com/boyism80/fb/wiki/Thread-Model)
- [Builtin functions](https://github.com/boyism80/fb/wiki/builtin) (Lua scripting reference)

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

## Contact

- [YouTube](https://www.youtube.com/channel/UCPcH5qX7aLTFs3mgh32_FVQ?view_as=subscriber)
- [Blog](https://blog.naver.com/boyism)
