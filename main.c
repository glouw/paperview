#include <SDL2/SDL.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include <stdarg.h>

typedef struct
{
    char** path;
    size_t size;
}
Paths;

typedef struct
{
    SDL_Texture** texture;
    size_t size;
}
Textures;

typedef struct
{
    Display* x11d;
    SDL_Window* window;
    SDL_Renderer* renderer;
}
Video;

static void Quit(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
    exit(1);
}

static int32_t Compare(const void* a, const void* b)
{
    char* const pa = *(char**) a;
    char* const pb = *(char**) b;
    const size_t la = strlen(pa);
    const size_t lb = strlen(pb);
    return (la > lb) ? 1 : (la < lb) ? -1 : strcmp(pa, pb);
}

static void Sort(Paths* self)
{
    qsort(self->path, self->size, sizeof(*self->path), Compare);
}

static Paths Populate(const char* base)
{
    DIR* const dir = opendir(base);
    if(dir == NULL)
        Quit("Directory '%s' failed to open\n", base);
    size_t max = 8;
    Paths self;
    self.size = 0;
    self.path = malloc(max * sizeof(*self.path));
    for(struct dirent* entry; (entry = readdir(dir));)
    {
        const char* const path = entry->d_name;
        if(strstr(path, ".bmp"))
        {
            char* const slash = "/";
            char* const buffer = malloc(strlen(base) + strlen(slash) + strlen(path) + 1);
            strcpy(buffer, base);
            strcat(buffer, slash);
            strcat(buffer, path);
            if(self.size == max)
            {
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

static void Depopulate(Paths* self)
{
    for(size_t i = 0; i < self->size; i++)
        free(self->path[i]);
    free(self->path);
}

static Textures Cache(Paths* paths, SDL_Renderer* renderer)
{
    Textures self;
    self.size = paths->size;
    self.texture = malloc(self.size * sizeof(*self.texture));
    for(size_t i = 0; i < self.size; i++)
    {
        const char* const path = paths->path[i];
        SDL_Surface* const surface = SDL_LoadBMP(path);
        if(surface == NULL)
            Quit("File '%s' failed to open. %s\n", path, SDL_GetError());
        self.texture[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    return self;
}

static void Destroy(Textures* self)
{
    for(size_t i = 0; i < self->size; i++)
        SDL_DestroyTexture(self->texture[i]);
    free(self->texture);
}

static Video Setup(void)
{
    Video self;
    self.x11d = XOpenDisplay(NULL);
    const Window x11w = RootWindow(self.x11d, DefaultScreen(self.x11d));
    SDL_Init(SDL_INIT_VIDEO);
    self.window = SDL_CreateWindowFrom((void*) x11w);
    self.renderer = SDL_CreateRenderer(self.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return self;
}

static void Teardown(Video* self)
{
    XCloseDisplay(self->x11d);
    SDL_Quit();
    SDL_DestroyWindow(self->window);
    SDL_DestroyRenderer(self->renderer);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
        Quit("paperview FOLDER SPEED\n");
    const char* const base = argv[1];
    const uint32_t speed = atoi(argv[2]);
    if(speed == 0)
        Quit("Invalid speed value\n");
    Video video = Setup();
    Paths paths = Populate(base);
    Textures textures = Cache(&paths, video.renderer);
    for(int32_t cycles = 0; /* TRUE */; cycles++)
    {
        const int32_t index = cycles / speed;
        const int32_t frame = index % textures.size;
        SDL_RenderCopy(video.renderer, textures.texture[frame], NULL, NULL);
        SDL_RenderPresent(video.renderer);
        SDL_Event event;
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
    }
    Destroy(&textures);
    Depopulate(&paths);
    Teardown(&video);
}
