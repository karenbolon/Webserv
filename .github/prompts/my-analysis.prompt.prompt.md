---
mode: 'ask'
---

I’m debugging a C++ web server project located at `/Users/karenbolon/Documents/aWebserv`.

Please:
1. Review the overall design, especially CGI handling and poll integration.
2. Check if I'm properly integrating CGI execution into the main event loop.
3. Look for anything that might block the poll loop, fail to close file descriptors, or break static file serving.
4. I’m not even able to serve regular static web pages anymore — what could be interfering?

Relevant files:
- the `srcs/` folder
- CGI logic: `srcs/CgiFunctions.cpp`, `srcs/Method.cpp`
- Poll loop: `srcs/initSocket.cpp`

Please consider the whole project structure if possible. Root folder: `/Users/karenbolon/Documents/aWebserv`
