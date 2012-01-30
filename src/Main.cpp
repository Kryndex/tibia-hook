#include "Hook.h"
#include "Main.h"

#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <X11/Xlib.h>

Window __XCreateWindow(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, unsigned long, XSetWindowAttributes*);
int __XNextEvent(Display*, XEvent*);

void hook_constructor() __attribute__((constructor));
void hook_destructor() __attribute__((destructor));

pthread_t hook_id;

void* hook_thread(void*) {
	Hook* hook = Hook::initialize();
	hook->exec();
	return NULL;
}

void hook_constructor() {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&hook_id, &attr, hook_thread, NULL);
}

void hook_destructor() {
	pthread_join(hook_id, NULL);
}

int X11Socket = -1;
Display* X11Display = NULL;
Window X11Window = 0;

int connect(int socket, const struct sockaddr* address, socklen_t length) {
	if (X11Socket == -1) {
		X11Socket = socket;
	}
	else {
		Hook::getInstance()->setSocket(socket);
	}
	return __connect(socket, address, length);
}

int poll(struct pollfd* fds, nfds_t nfds, int timeout) {
	Hook* hook = Hook::getInstance();
	if (nfds == 1 && fds[0].events == POLLIN && hook && hook->socket() == fds[0].fd && hook->hasClientMessages()) {
		fds[0].revents = POLLIN;
		return 1;
	}
	return __poll(fds, nfds, timeout);
}

ssize_t read(int socket, void* buffer, size_t length) {
	Hook* hook = Hook::getInstance();
	if (hook && hook->socket() == socket && length > 0) {
		return hook->receiveFromServer((quint8*) buffer, length);
	}
	return __read(socket, buffer, length);
}

ssize_t write(int socket, const void* buffer, size_t length) {
	Hook* hook = Hook::getInstance();
	if (hook && hook->socket() == socket && length > 0) {
		return hook->receiveFromClient((const quint8*) buffer, length);
	}
	return __write(socket, buffer, length);
}

Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class_, Visual* visual, unsigned long valuemask, XSetWindowAttributes* attributes) {
	Window window = __XCreateWindow(display, parent, x, y, width, height, border_width, depth, class_, visual, valuemask, attributes);
	if (X11Window == 0) {
		X11Window = window;
		Hook::getInstance()->setWindow(X11Window);
	}
	return window;
}

int XNextEvent(Display* display, XEvent* event_return) {
	if (X11Display == NULL) {
		X11Display = display;
		Hook::getInstance()->setDisplay(X11Display);
	}
	return __XNextEvent(display, event_return);
}

int __connect(int socket, const struct sockaddr* address, socklen_t length) {
	static int (*original)(int, const struct sockaddr*, socklen_t);
	if (original == NULL) {
		original = (int(*)(int, const struct sockaddr*, socklen_t)) dlsym(RTLD_NEXT, "connect");assert(original != NULL);
	}
	return original(socket, address, length);
}

int __poll(struct pollfd* fds, nfds_t nfds, int timeout) {
	static int (*original)(struct pollfd*, nfds_t, int);
	if (original == NULL) {
		original = (int(*)(struct pollfd*, nfds_t, int)) dlsym(RTLD_NEXT, "poll");assert(original != NULL);
	}
	return original(fds, nfds, timeout);
}

ssize_t __read(int socket, void* buffer, size_t length) {
	static ssize_t (*original)(int, void*, size_t);
	if (original == NULL) {
		original = (ssize_t(*)(int, void*, size_t)) dlsym(RTLD_NEXT, "read");assert(original != NULL);
	}
	return original(socket, buffer, length);
}

ssize_t __write(int socket, const void* buffer, size_t length) {
	static ssize_t (*original)(int, const void*, size_t);
	if (original == NULL) {
		original = (ssize_t(*)(int, const void*, size_t)) dlsym(RTLD_NEXT, "write");assert(original != NULL);
	}
	return original(socket, buffer, length);
}

Window __XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class_, Visual* visual, unsigned long valuemask, XSetWindowAttributes* attributes) {
	static Window (*original)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, unsigned long, XSetWindowAttributes*);
	if (original == NULL) {
		original = (Window(*)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, unsigned long, XSetWindowAttributes*)) dlsym(RTLD_NEXT, "XCreateWindow");assert(original != NULL);
	}
	return original(display, parent, x, y, width, height, border_width, depth, class_, visual, valuemask, attributes);
}

int __XNextEvent(Display* display, XEvent* event_return) {
	static int (*original)(Display*, XEvent*);
	if (original == NULL) {
		original = (int(*)(Display*, XEvent*)) dlsym(RTLD_NEXT, "XNextEvent");assert(original != NULL);
	}
	return original(display, event_return);
}