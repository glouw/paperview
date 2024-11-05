#include <GL/glx.h>
#include <SDL2/SDL.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>

typedef struct {
  char **path;
  unsigned size;
} Paths;

typedef struct {
  SDL_Texture **texture;
  unsigned size;
} Textures;

typedef struct ViewConfig {
  int speed;
  char *base;
  SDL_Rect *rect;
} ViewConfig;

typedef struct Config {
  bool create_desktop;
  int view_count;
  ViewConfig **views;
} Config;

typedef struct View {
  int speed;
  Textures textures;
  SDL_Rect *rect;
  struct View *next;
} View;

typedef struct {
  Display *x11d;
  SDL_Window *window;
  SDL_Renderer *renderer;
} Video;

static void Quit(const char *const message, ...) {
  va_list args;
  va_start(args, message);
  vfprintf(stdout, message, args);
  va_end(args);
  exit(1);
}

static int Compare(const void *a, const void *b) {
  char *const pa = *(char **)a;
  char *const pb = *(char **)b;
  const unsigned la = strlen(pa);
  const unsigned lb = strlen(pb);
  return (la > lb) ? 1 : (la < lb) ? -1 : strcmp(pa, pb);
}

static void Sort(Paths *self) {
  qsort(self->path, self->size, sizeof(*self->path), Compare);
}

static Paths Populate(const char *base) {
  DIR *const dir = opendir(base);
  if (dir == NULL)
    Quit("Directory '%s' failed to open\n", base);
  unsigned max = 8;
  Paths self;
  self.size = 0;
  self.path = malloc(max * sizeof(*self.path));
  for (struct dirent *entry; (entry = readdir(dir));) {
    const char *const path = entry->d_name;
    if (strstr(path, ".bmp")) {
      char *const slash = "/";
      char *const buffer =
          malloc(strlen(base) + strlen(slash) + strlen(path) + 1);
      strcpy(buffer, base);
      strcat(buffer, slash);
      strcat(buffer, path);
      if (self.size == max) {
        max *= 2;
        self.path = realloc(self.path, max * sizeof(*self.path));
      }
      self.path[self.size] = buffer;
      self.size += 1;
    }
  }
  closedir(dir);
  Sort(&self);
  return self;
}

static void Depopulate(Paths *self) {
  for (unsigned i = 0; i < self->size; i++)
    free(self->path[i]);
  free(self->path);
}

static Textures Cache(Paths *paths, SDL_Renderer *renderer) {
  Textures self;
  self.size = paths->size;
  self.texture = malloc(self.size * sizeof(*self.texture));
  for (unsigned i = 0; i < self.size; i++) {
    const char *const path = paths->path[i];
    SDL_Surface *const surface = SDL_LoadBMP(path);
    if (surface == NULL)
      Quit("File '%s' failed to open. %s\n", path, SDL_GetError());
    self.texture[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
  }
  return self;
}

static void Destroy(Textures *self) {
  for (unsigned i = 0; i < self->size; i++)
    SDL_DestroyTexture(self->texture[i]);
  free(self->texture);
}

static Window CreateDesktop(Display *x11d) {
  const Window root = RootWindow(x11d, DefaultScreen(x11d));
  int width = DisplayWidth(x11d, 0);
  int height = DisplayHeight(x11d, 0);

  XVisualInfo *vi;
  int visual_attribs[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
  vi = glXChooseVisual(x11d, 0, visual_attribs);
  if (!vi) {
    Quit("No appropriate visual found\n");
  }

  Colormap cmap = XCreateColormap(x11d, root, vi->visual, AllocNone);

  XSetWindowAttributes swa;
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.override_redirect = True;

  const Window x11w = XCreateWindow(
      x11d, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
      CWBorderPixel | CWColormap | CWOverrideRedirect, &swa);

  // Set window type to desktop and make it stay below other windows
  Atom atom_type = XInternAtom(x11d, "_NET_WM_WINDOW_TYPE", 0);
  Atom atom_desktop = XInternAtom(x11d, "_NET_WM_WINDOW_TYPE_DESKTOP", 0);
  XChangeProperty(x11d, x11w, atom_type, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)&atom_desktop, 1);

  // Additional hints to keep the window below others
  Atom atom_state = XInternAtom(x11d, "_NET_WM_STATE", 0);
  Atom atom_below = XInternAtom(x11d, "_NET_WM_STATE_BELOW", 0);
  XChangeProperty(x11d, x11w, atom_state, XA_ATOM, 32, PropModeAppend,
                  (unsigned char *)&atom_below, 1);

  // Map and lower the window
  XMapWindow(x11d, x11w);
  XLowerWindow(x11d, x11w); // Ensures the window is below others
  XSync(x11d, 0);

  return x11w;
}

static Window GetX11Window(Display *x11d, bool create_desktop) {
  if (create_desktop) {
    return CreateDesktop(x11d);
  }
  return RootWindow(x11d, DefaultScreen(x11d));
}

static Video Setup(bool create_desktop) {
  Video self;
  self.x11d = XOpenDisplay(NULL);
  const Window x11w = GetX11Window(self.x11d, create_desktop);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  self.window = SDL_CreateWindowFrom((void *)x11w);
  self.renderer = SDL_CreateRenderer(
      self.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  return self;
}

static void Teardown(Video *self) {
  SDL_DestroyRenderer(self->renderer);
  SDL_DestroyWindow(self->window);
  XCloseDisplay(self->x11d);
  SDL_Quit();
}

static View *Init(const char *const base, const int speed, SDL_Rect *rect,
                  Video *video) {
  View *self = malloc(sizeof(*self));
  self->speed = speed;
  Paths paths = Populate(base);
  self->textures = Cache(&paths, video->renderer);
  self->rect = rect;
  Depopulate(&paths);
  self->next = NULL;
  return self;
}

static View *Push(View *views, View *view) {
  view->next = views;
  return view;
}

static void Cleanup(View *views) {
  View *view = views;
  while (view) {
    View *next = view->next;
    Destroy(&view->textures);
    free(view->rect);
    free(view);
    view = next;
  }
}

static View *Load(int view_count, ViewConfig **configs, Video *video) {
  View *views = NULL;
  for (int i = 0; i < view_count; i += 1) {
    views = Push(views, Init(configs[i]->base, configs[i]->speed,
                             configs[i]->rect, video));
  }
  return views;
}

static Config Parse(int argc, char **argv) {
  Config self;
  self.create_desktop = strcmp(argv[1], "--compositor") == 0;

  int arg_offset = 1;
  if (self.create_desktop) {
    arg_offset += 1;
  }

  const int args = argc - arg_offset;
  if (args < 2)
    Quit("Usage: paperview FOLDER SPEED\n"); // LEGACY PARAMETER SUPPORT.
  const int params = 6;
  if (args > 2 && args % params != 0)
    Quit("Usage: paperview [--compositor] FOLDER SPEED X Y W H FOLDER "
         "SPEED X Y W H # ... And so on\n"); // MULTI-MONITOR PARAMETER SUPPORT.

  self.view_count = args / params;
  self.views = malloc(self.view_count * sizeof(self.views));
  for (int i = arg_offset; i < argc; i += params) {
    const int view_num = (i - arg_offset) / params;
    const int a = i + 0;
    const int b = i + 1;
    const int c = i + 2;
    const int d = i + 3;
    const int e = i + 4;
    const int f = i + 5;
    ViewConfig *view = malloc(sizeof(*view));
    view->base = argv[a];
    view->speed = atoi(argv[b]);
    if (view->speed == 0)
      Quit("Invalid speed value\n");
    if (view->speed < 0)
      view->speed = INT32_MAX; // NEGATIVE SPEED VALUES CREATE STILL WALLPAPERS.
    if (c != argc) {
      view->rect = malloc(sizeof(*view->rect));
      view->rect->x = atoi(argv[c]);
      view->rect->y = atoi(argv[d]);
      view->rect->w = atoi(argv[e]);
      view->rect->h = atoi(argv[f]);
    }
    self.views[view_num] = view;
  }
  return self;
}

int main(int argc, char **argv) {
  Config config = Parse(argc, argv);
  Video video = Setup(config.create_desktop);
  View *views = Load(config.view_count, config.views, &video);
  for (int cycles = 0; /* true */; cycles++) {
    for (View *view = views; view; view = view->next) {
      const int index = cycles / view->speed;
      const int frame = index % view->textures.size;
      SDL_RenderCopy(video.renderer, view->textures.texture[frame], NULL,
                     view->rect);
    }
    SDL_RenderPresent(video.renderer);
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      break;
  }
  Cleanup(views);
  Teardown(&video);
}
