#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>

#define MUSIC_PATH "../music/CornfieldChase.wav"

#define COSMOS_PATH "../image/Cosmos.jpg"
#define SUN_PATH "../image/Sun.png"
#define MERCURY_PATH "../image/Mercury.png"
#define VENUS_PATH "../image/Venus.png"
#define EARTH_PATH "../image/Earth.png"
#define MOON_PATH "../image/Moon.png"
#define MARS_PATH "../image/Mars.png"
#define JUPITER_PATH "../image/Jupiter.png"
#define SATURN_PATH "../image/Saturn.png"
#define URANUS_PATH "../image/Uranus.png"
#define NEPTUNE_PATH "../image/Neptune.png"

using namespace std;

class Object {
private:
    SDL_Texture* texture = nullptr;
    SDL_Rect* rect = nullptr;
    double sizeScale = 0;
    double radiusScale = 0;
    double angle = 0;
    double angleInc = 0;
    bool isMoon = false;

public:
    friend bool createObject(SDL_Renderer* renderer, string path, double sizeScale, double radiusScale, double inc,
                             bool isMoon);

    void calculateNextPos(SDL_Window* window) {
        // background
        if (rect == nullptr) {
            return;
        }
        angle += angleInc;
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        rect->w = rect->h = width * sizeScale;
        double radius = width * radiusScale;

        int y = round(radius * sin(angle / 180.0 * M_PI) * sqrt(2) / 4.0);
        int x = round(radius * cos(angle / 180.0 * M_PI) - y);

        if (isMoon) {
            // add the earth's coordinate
            rect->y = objects[4]->getY() - y - rect->h / 2;
            rect->x = objects[4]->getX() - x - rect->w / 2;
        } else {
            rect->y = height / 2 - y - rect->h / 2;
            rect->x = width / 2 - x - rect->w / 2;
        }
    }

    int getX() {
        // centerX
        if (rect) return rect->x + rect->w / 2;
        return 0;
    }

    int getY() {
        // centerY
        if (rect) return rect->y + rect->h / 2;
        return 0;
    }

    void draw(SDL_Renderer* renderer) { SDL_RenderCopy(renderer, texture, nullptr, rect); }

    void destory() {
        if (texture) SDL_DestroyTexture(texture);
    }

    static vector<Object*> objects;
};

vector<Object*> Object::objects;

bool createObject(SDL_Renderer* renderer, string path, double sizeScale, double radiusScale, double inc,
                  bool isMoon = false) {
    Object* obj = new Object;
    SDL_Surface* suface = IMG_Load(path.c_str());
    if (suface == nullptr) {
        cerr << "Unable to load image! IMG_Error: " << IMG_GetError() << endl;
        return false;
    }
    obj->texture = SDL_CreateTextureFromSurface(renderer, suface);

    if (sizeScale > 0) {
        obj->rect = new SDL_Rect();
        obj->rect->h = 0;
        obj->rect->w = 0;
        obj->rect->x = 0;
        obj->rect->y = 0;
    }
    obj->sizeScale = sizeScale;
    obj->radiusScale = radiusScale;
    srand(time(0));
    obj->angle = rand() % 360;

    obj->angleInc = inc;

    obj->isMoon = isMoon;
    Object::objects.push_back(obj);
    return true;
}

void destoryObject() {
    for (auto obj : Object::objects) {
        obj->destory();
    }
}

bool loadObjects(SDL_Renderer* renderer) {
    return createObject(renderer, COSMOS_PATH, 0, 0, 0) && createObject(renderer, SUN_PATH, 0.15625, 0, 0) &&
           createObject(renderer, MERCURY_PATH, 0.006, 0.0625, 4) &&
           createObject(renderer, VENUS_PATH, 0.012, 0.086, 1.5) &&
           createObject(renderer, EARTH_PATH, 0.015, 0.125, 1) &&
           createObject(renderer, MOON_PATH, 0.005, 0.025, 12, true) &&
           createObject(renderer, MARS_PATH, 0.01, 0.2, 0.5) && createObject(renderer, JUPITER_PATH, 0.06, 0.3, 0.25) &&
           createObject(renderer, SATURN_PATH, 0.05, 0.45, 0.2) &&
           createObject(renderer, URANUS_PATH, 0.045, 0.55, 0.15) &&
           createObject(renderer, NEPTUNE_PATH, 0.045, 0.75, 0.1);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
        return -1;
    }

    Mix_Chunk* sound = Mix_LoadWAV(MUSIC_PATH);
    if (sound == nullptr) {
        cerr << "Failed to load WAV file! Mix_Error: " << Mix_GetError() << endl;
        return -1;
    }

    Mix_PlayChannel(-1, sound, -1);

    // reduce 50% volume
    Mix_VolumeChunk(sound, 64);

    SDL_Window* window = SDL_CreateWindow("SolarSystemModel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0,
                                          SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    auto clearSDL = [&]() {
        destoryObject();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        Mix_FreeChunk(sound);
        Mix_CloseAudio();

        SDL_Quit();
    };

    if (!loadObjects(renderer)) {
        clearSDL();
        return -1;
    }

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        SDL_RenderClear(renderer);

        for (auto obj : Object::objects) {
            obj->calculateNextPos(window);
        }

        // reorder and draw
        auto copy = Object::objects;
        sort(copy.begin(), copy.end(), [](Object* obj1, Object* obj2) { return obj1->getY() < obj2->getY(); });
        for (auto obj : copy) {
            obj->draw(renderer);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
                break;
            }
        }

        this_thread::sleep_for(chrono::milliseconds(60));
    }

    clearSDL();

    return 0;
}