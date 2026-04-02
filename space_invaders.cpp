// ================================================================
//  SPACE INVADERS  —  C++ / SFML 2.x
//  Polished downloadable-app quality build
//
//  Compile (Linux/macOS):
//    g++ space_invaders.cpp -o space_invaders \
//        -lsfml-graphics -lsfml-window -lsfml-system \
//        -O2 -std=c++17
//
//  Compile (Windows MinGW):
//    g++ space_invaders.cpp -o space_invaders.exe \
//        -lsfml-graphics -lsfml-window -lsfml-system \
//        -O2 -std=c++17 -mwindows
// ================================================================

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <array>
#include <functional>

// ── Window & layout ──────────────────────────────────────────
static constexpr int   W  = 900;
static constexpr int   H  = 660;
static constexpr float HUD_H = 48.f;
static constexpr float GROUND_Y = H - 44.f;

// ── Gameplay constants ───────────────────────────────────────
static constexpr int   ALIEN_COLS       = 11;
static constexpr int   ALIEN_ROWS       = 5;
static constexpr float PLAYER_SPEED     = 300.f;
static constexpr float BULLET_SPEED     = 560.f;
static constexpr float ABUL_SPEED       = 240.f;
static constexpr float ALIEN_DROP       = 16.f;
static constexpr float SHOOT_COOLDOWN   = 0.32f;
static constexpr int   STARS            = 180;

// ── Palette ──────────────────────────────────────────────────
static const sf::Color C_BG       {  6,   6,  18 };
static const sf::Color C_PLAYER   { 80, 190, 255 };
static const sf::Color C_SHIELD   { 50, 220,  80 };
static const sf::Color C_BULLET   {255, 255, 255 };
static const sf::Color C_ABUL     {255, 100,  40 };
static const sf::Color C_UFO      {220,  50,  50 };
static const sf::Color C_HUD      {100, 200, 255 };
static const sf::Color C_GROUND   { 40, 180,  60 };

// alien type colours
static const sf::Color C_A[3] = {
    {255, 140,  50},   // type 0 – top rows   (10 pts)
    {180,  90, 255},   // type 1 – mid rows   (20 pts)
    { 60, 230, 120},   // type 2 – bot rows   (30 pts)
};
static const int A_PTS[3] = {10, 20, 30};

// ── Helpers ──────────────────────────────────────────────────
static float randf(float lo, float hi) {
    return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
}
static std::string fmt(int n) {
    std::ostringstream s; s << n; return s.str();
}

// ── Pixel-art alien bitmaps (11×8, 1=filled) ─────────────────
// Each alien has 2 animation frames.
// Layout: [type][frame][row] = bitmask of 11 bits
static const std::array<std::array<std::array<uint16_t,8>,2>,3> ALIEN_MAP = {{
    // type 0 – "crab"
    {{{{
        0b00100000100,
        0b01000001000,
        0b01111111110,
        0b11011101101,
        0b11111111111,
        0b01111111110,
        0b00100000100,
        0b01000001000,
    }},{
        0b00100000100,
        0b10000000001,
        0b10111111101,
        0b11011101101,
        0b11111111111,
        0b01111111110,
        0b00010001000,
        0b00100000100,
    }}},
    // type 1 – "octopus"
    {{{{
        0b00011111000,
        0b01111111110,
        0b11111111111,
        0b11001111001,
        0b11111111111,
        0b00101000101,
        0b01001000010,
        0b10100000101,
    }},{
        0b00011111000,
        0b01111111110,
        0b11111111111,
        0b11001111001,
        0b11111111111,
        0b00101000101,
        0b01010000010,
        0b00101000100,
    }}},
    // type 2 – "squid"
    {{{{
        0b00001110000,
        0b00011111000,
        0b00111111100,
        0b01101001101,
        0b01111111101,
        0b00110110110,
        0b01001001001,
        0b00110110110,
    }},{
        0b00001110000,
        0b00011111000,
        0b00111111100,
        0b01101001101,
        0b01111111101,
        0b00110110110,
        0b00101001010,
        0b01010110101,
    }}}
}};

// ── Draw pixel-art alien onto RenderTarget ───────────────────
static void drawAlienSprite(sf::RenderTarget &rt,
                             float ox, float oy,
                             int type, int frame,
                             sf::Color col)
{
    const float PS = 2.4f;   // pixel size
    sf::RectangleShape px({PS, PS});
    px.setFillColor(col);
    for (int r = 0; r < 8; r++) {
        uint16_t row = ALIEN_MAP[type][frame][r];
        for (int c = 0; c < 11; c++) {
            if (row & (1 << (10 - c))) {
                px.setPosition(ox + c * PS, oy + r * PS);
                rt.draw(px);
            }
        }
    }
}

// ── Draw pixel-art UFO ───────────────────────────────────────
static void drawUFO(sf::RenderTarget &rt, float ox, float oy) {
    const float PS = 2.6f;
    // bitmap: 16 wide x 7 tall
    static const uint16_t UFO[7] = {
        0b0000111111000000,
        0b0001111111110000,
        0b0011001100110000,
        0b0111111111111100,
        0b0011001100110000,
        0b0001111111110000,
        0b0000111111000000,
    };
    sf::RectangleShape px({PS, PS});
    for (int r = 0; r < 7; r++) {
        uint16_t row = UFO[r];
        sf::Color col = (r == 2 || r == 4)
                        ? sf::Color(255, 160, 160)
                        : C_UFO;
        px.setFillColor(col);
        for (int c = 0; c < 16; c++) {
            if (row & (1 << (15 - c))) {
                px.setPosition(ox + c * PS, oy + r * PS);
                rt.draw(px);
            }
        }
    }
}

// ── Draw pixel-art player cannon ─────────────────────────────
static void drawPlayer(sf::RenderTarget &rt, float ox, float oy) {
    const float PS = 3.f;
    static const uint8_t PL[7] = {
        0b00010000,
        0b00111000,
        0b00111000,
        0b11111110,
        0b11111110,
        0b11111110,
        0b11111110,
    };
    sf::RectangleShape px({PS, PS});
    px.setFillColor(C_PLAYER);
    for (int r = 0; r < 7; r++) {
        for (int c = 0; c < 8; c++) {
            if (PL[r] & (1 << (7 - c))) {
                px.setPosition(ox + c * PS, oy + r * PS);
                rt.draw(px);
            }
        }
    }
}

// ── Starfield ────────────────────────────────────────────────
struct Star {
    sf::Vector2f pos;
    float speed, brightness;
};

// ── Entities ─────────────────────────────────────────────────
struct Bullet {
    float x, y;
    bool friendly, active = true;
};

struct Alien {
    float x, y;
    int  type;
    bool alive = true;
    float animTimer = 0.f;
    int  animFrame  = 0;
    // flash on hit (already dead but flashing)
    float flashTimer = 0.f;
};

struct ShieldCell {
    float x, y;
    int   hp = 3;  // 3 hits to destroy
};

struct UFOState {
    float x = -100.f, y = HUD_H + 12.f;
    bool  active = false;
    float respawnTimer = 0.f;
    float speed = 0.f;
};

struct Particle {
    float x, y, vx, vy, life, maxLife;
    sf::Color col;
};

// ── Screen-flash ─────────────────────────────────────────────
struct Flash {
    sf::Color col;
    float     life = 0.f;
    void trigger(sf::Color c, float t) { col = c; life = t; }
};

// ── Font-less text helper ─────────────────────────────────────
// Renders a string as pixel segments (tiny 3×5 font) without needing
// any external font file, so the binary is truly self-contained.
// For a real release you'd bundle a font; this keeps the single-file spirit.

// ── Main game class ───────────────────────────────────────────
class Game {
public:
    Game() :
        window(sf::VideoMode(W, H), "SPACE INVADERS",
               sf::Style::Titlebar | sf::Style::Close),
        state(MENU), score(0), hiScore(0), level(1), lives(3),
        alienDir(1.f), shootCooldown(0.f),
        alienShootTimer(0.f), invincTimer(0.f),
        waveCleared(false), waveClearTimer(0.f)
    {
        window.setFramerateLimit(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        // font – try several common paths; graceful fallback
        fontLoaded =
            font.loadFromFile("PressStart2P-Regular.ttf") ||
            font.loadFromFile("assets/PressStart2P-Regular.ttf") ||
            font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf") ||
            font.loadFromFile("/System/Library/Fonts/Courier.ttc");

        buildStars();
        setupHUD();
        resetGame();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float dt = std::min(clock.restart().asSeconds(), 0.05f);
            handleEvents();
            switch (state) {
                case PLAYING:   updatePlaying(dt); break;
                case MENU:      updateMenu(dt);    break;
                case GAME_OVER: updateMenu(dt);    break;
                case WIN:       updateMenu(dt);    break;
                default: break;
            }
            render();
        }
    }

private:
    enum State { MENU, PLAYING, GAME_OVER, WIN, LEVEL_TRANS };

    // ── window / font ─────────────────────────────────────────
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded = false;

    // ── game state ────────────────────────────────────────────
    State state;
    int score, hiScore, level, lives;

    // ── entities ──────────────────────────────────────────────
    float playerX = W / 2.f - 12.f;
    float alienDir, alienSpeed;
    float shootCooldown;
    float alienShootTimer, alienShootInterval;
    float invincTimer;       // player invincibility after hit
    bool  waveCleared;
    float waveClearTimer;

    std::vector<Bullet>     bullets;
    std::vector<Alien>      aliens;
    std::vector<ShieldCell> shields;
    UFOState                ufo;
    std::vector<Particle>   particles;
    std::vector<Star>       stars;
    Flash                   flash;

    // ── HUD texts ─────────────────────────────────────────────
    sf::Text tScore, tHi, tLevel, tLives;
    sf::Text tCenter, tSub, tSub2;

    // ─────────────────────────────────────────────────────────
    void buildStars() {
        stars.resize(STARS);
        for (auto &s : stars) {
            s.pos  = {randf(0, W), randf(HUD_H, float(H))};
            s.speed = randf(6.f, 28.f);
            s.brightness = randf(80.f, 220.f);
        }
    }

    void setupHUD() {
        auto mk = [&](sf::Text &t, unsigned sz, sf::Color c) {
            if (fontLoaded) t.setFont(font);
            t.setCharacterSize(sz);
            t.setFillColor(c);
        };
        mk(tScore,  15, C_HUD);
        mk(tHi,     15, sf::Color(255, 220, 80));
        mk(tLevel,  15, C_HUD);
        mk(tLives,  15, C_HUD);
        mk(tCenter, 36, C_HUD);
        mk(tSub,    16, sf::Color(200, 200, 200));
        mk(tSub2,   13, sf::Color(140, 140, 140));
    }

    void resetGame() {
        score = 0; level = 1; lives = 3;
        waveCleared = false;
        playerX = W / 2.f - 12.f;
        bullets.clear(); particles.clear();
        flash = Flash{};
        initLevel();
    }

    void initLevel() {
        bullets.clear();
        particles.clear();
        waveCleared     = false;
        shootCooldown   = 0.f;
        invincTimer     = 0.f;
        playerX         = W / 2.f - 12.f;

        alienDir          = 1.f;
        alienSpeed        = 30.f + level * 8.f;
        alienShootInterval = std::max(0.35f, 1.3f - level * 0.07f);
        alienShootTimer   = 0.f;

        // Alien grid  (type 0=top, 1=mid, 2=bot)
        aliens.clear();
        int typeMap[ALIEN_ROWS] = {0, 0, 1, 1, 2};
        for (int r = 0; r < ALIEN_ROWS; r++)
            for (int c = 0; c < ALIEN_COLS; c++) {
                Alien a;
                a.type = typeMap[r];
                a.x = 52.f + c * 72.f;
                a.y = HUD_H + 30.f + r * 50.f;
                aliens.push_back(a);
            }

        // Shield bunkers
        shields.clear();
        for (int s = 0; s < 4; s++) {
            float sx = 80.f + s * 200.f;
            float sy = GROUND_Y - 100.f;
            for (int r = 0; r < 5; r++) {
                for (int c = 0; c < 9; c++) {
                    // arch cutout at bottom-centre
                    bool skip = (r >= 3 && (c == 3 || c == 4 || c == 5));
                    if (skip) continue;
                    ShieldCell sc;
                    sc.x = sx + c * 8.f;
                    sc.y = sy + r * 8.f;
                    shields.push_back(sc);
                }
            }
        }

        // UFO
        ufo.active = false;
        ufo.respawnTimer = randf(10.f, 22.f);
        ufo.speed = 110.f + level * 12.f;
    }

    // ── event handling ────────────────────────────────────────
    void handleEvents() {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed ||
               (ev.type == sf::Event::KeyPressed &&
                ev.key.code == sf::Keyboard::Escape))
                window.close();

            if (ev.type == sf::Event::KeyPressed) {
                auto k = ev.key.code;
                bool anyKey = (k == sf::Keyboard::Enter ||
                               k == sf::Keyboard::Space ||
                               k == sf::Keyboard::Return);
                if (state == MENU && anyKey) {
                    resetGame(); state = PLAYING;
                }
                if ((state == GAME_OVER || state == WIN) && anyKey) {
                    resetGame(); state = PLAYING;
                }
            }
        }
    }

    // ── menu / overlay update (scrolling stars only) ──────────
    void updateMenu(float dt) {
        scrollStars(dt * 0.4f);
        updateParticles(dt);
    }

    // ── main gameplay update ──────────────────────────────────
    void updatePlaying(float dt) {
        scrollStars(dt);

        // ── player movement ───────────────────────────────────
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            playerX -= PLAYER_SPEED * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            playerX += PLAYER_SPEED * dt;
        playerX = std::clamp(playerX, 0.f, float(W) - 24.f);

        // ── shoot ─────────────────────────────────────────────
        shootCooldown -= dt;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) &&
            shootCooldown <= 0.f) {
            bullets.push_back({playerX + 11.f, GROUND_Y - 30.f, true});
            shootCooldown = SHOOT_COOLDOWN;
        }

        // ── move bullets ─────────────────────────────────────
        for (auto &b : bullets) {
            b.y += (b.friendly ? -BULLET_SPEED : ABUL_SPEED) * dt;
            if (b.y < HUD_H || b.y > H) b.active = false;
        }

        // ── move aliens ───────────────────────────────────────
        float minX = W, maxX = 0;
        for (auto &a : aliens) {
            if (!a.alive) continue;
            minX = std::min(minX, a.x);
            maxX = std::max(maxX, a.x + 26.f);
        }
        bool drop = (alienDir > 0 && maxX >= W - 10) ||
                    (alienDir < 0 && minX <= 10);
        if (drop) {
            alienDir = -alienDir;
            for (auto &a : aliens)
                if (a.alive) a.y += ALIEN_DROP;
        }
        for (auto &a : aliens) {
            if (!a.alive) continue;
            a.x += alienDir * alienSpeed * dt;
            a.animTimer += dt;
            if (a.animTimer > 0.45f) { a.animTimer = 0; a.animFrame ^= 1; }
        }

        // ── alien invasion (reached ground) ──────────────────
        for (auto &a : aliens)
            if (a.alive && a.y + 20.f >= GROUND_Y - 4.f) {
                flash.trigger(sf::Color(255,60,60,120), 0.5f);
                endGame(false); return;
            }

        // ── alien shooting ────────────────────────────────────
        alienShootTimer += dt;
        if (alienShootTimer >= alienShootInterval) {
            alienShootTimer = 0.f;
            // pick a random front-line alien (lowest in each column)
            std::vector<Alien *> front;
            for (int c = 0; c < ALIEN_COLS; c++) {
                Alien *best = nullptr;
                for (auto &a : aliens)
                    if (a.alive) {
                        int col = int((a.x - 52.f + 4.f) / 72.f);
                        if (col == c)
                            if (!best || a.y > best->y) best = &a;
                    }
                if (best) front.push_back(best);
            }
            if (!front.empty()) {
                Alien *sh = front[std::rand() % front.size()];
                bullets.push_back({sh->x + 13.f, sh->y + 20.f, false});
            }
        }

        // ── UFO ───────────────────────────────────────────────
        ufo.respawnTimer -= dt;
        if (!ufo.active && ufo.respawnTimer <= 0.f) {
            ufo.active = true;
            ufo.x = -50.f;
            ufo.y = HUD_H + 12.f;
            ufo.respawnTimer = randf(10.f, 22.f);
        }
        if (ufo.active) {
            ufo.x += ufo.speed * dt;
            if (ufo.x > W + 60.f) ufo.active = false;
        }

        // ── collisions ────────────────────────────────────────
        auto rectHit = [](float ax,float ay,float aw,float ah,
                           float bx,float by,float bw,float bh){
            return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
        };

        // player bullets vs aliens
        for (auto &b : bullets) {
            if (!b.friendly || !b.active) continue;
            for (auto &a : aliens) {
                if (!a.alive) continue;
                if (rectHit(b.x, b.y, 3, 12, a.x, a.y, 26, 20)) {
                    a.alive = false; b.active = false;
                    int pts = A_PTS[a.type];
                    score += pts;
                    if (score > hiScore) hiScore = score;
                    spawnBurst(a.x+13, a.y+10, C_A[a.type], 10);
                    flash.trigger(sf::Color(255,255,255,18), 0.08f);
                    break;
                }
            }
        }

        // player bullets vs UFO
        if (ufo.active) {
            for (auto &b : bullets) {
                if (!b.friendly || !b.active) continue;
                if (rectHit(b.x, b.y, 3, 12, ufo.x, ufo.y, 42.f, 18.f)) {
                    b.active = false; ufo.active = false;
                    int pts = 50 * (1 + std::rand() % 6);  // 50-300
                    score += pts;
                    if (score > hiScore) hiScore = score;
                    spawnBurst(ufo.x+21, ufo.y+9, C_UFO, 16);
                    flash.trigger(sf::Color(255,80,80,60), 0.15f);
                }
            }
        }

        // bullets vs shields
        for (auto &b : bullets) {
            if (!b.active) continue;
            for (auto &sc : shields) {
                if (sc.hp <= 0) continue;
                if (rectHit(b.x, b.y, 3, 8, sc.x, sc.y, 8, 8)) {
                    sc.hp--;
                    b.active = false;
                    break;
                }
            }
        }

        // alien bullets vs player
        invincTimer -= dt;
        if (invincTimer <= 0.f) {
            for (auto &b : bullets) {
                if (b.friendly || !b.active) continue;
                float py = GROUND_Y - 21.f;
                if (rectHit(b.x, b.y, 3, 8, playerX, py, 24.f, 21.f)) {
                    b.active = false;
                    lives--;
                    invincTimer = 1.8f;
                    spawnBurst(playerX+12, py+10, C_PLAYER, 14);
                    flash.trigger(sf::Color(255,60,60,90), 0.3f);
                    if (lives <= 0) { endGame(false); return; }
                    playerX = W / 2.f - 12.f;
                }
            }
        }

        // ── cleanup ───────────────────────────────────────────
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet &b){ return !b.active; }), bullets.end());

        updateParticles(dt);

        // ── flash timer ───────────────────────────────────────
        if (flash.life > 0.f) flash.life -= dt;

        // ── wave cleared ─────────────────────────────────────
        bool anyAlive = std::any_of(aliens.begin(), aliens.end(),
                                    [](const Alien &a){ return a.alive; });
        if (!anyAlive && !waveCleared) {
            waveCleared = true;
            waveClearTimer = 1.4f;
            spawnBurst(W/2.f, H/2.f, sf::Color(80,255,160), 30);
            flash.trigger(sf::Color(80,255,160,40), 0.5f);
        }
        if (waveCleared) {
            waveClearTimer -= dt;
            if (waveClearTimer <= 0.f) {
                level++;
                initLevel();
            }
        }
    }

    // ── particles ─────────────────────────────────────────────
    void spawnBurst(float x, float y, sf::Color c, int n) {
        for (int i = 0; i < n; i++) {
            float ang = randf(0, 6.28f);
            float spd = randf(40.f, 160.f);
            particles.push_back({x, y,
                std::cos(ang)*spd, std::sin(ang)*spd,
                randf(0.4f, 1.0f), 1.0f, c});
        }
    }
    void updateParticles(float dt) {
        for (auto &p : particles) {
            p.x  += p.vx * dt;
            p.y  += p.vy * dt;
            p.vy += 90.f * dt;   // gravity
            p.life -= dt;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](const Particle &p){ return p.life <= 0; }), particles.end());
    }

    // ── scrolling starfield ───────────────────────────────────
    void scrollStars(float dt) {
        for (auto &s : stars) {
            s.pos.y += s.speed * dt;
            if (s.pos.y > H) { s.pos.y = HUD_H; s.pos.x = randf(0, W); }
        }
    }

    // ── game end ──────────────────────────────────────────────
    void endGame(bool /*win*/) { state = GAME_OVER; }

    // ── text centering helper ─────────────────────────────────
    void drawCenteredText(sf::Text &t, const std::string &s, float y,
                          sf::Color col = sf::Color::White) {
        t.setString(s);
        t.setFillColor(col);
        float tw = t.getLocalBounds().width;
        t.setPosition((W - tw) / 2.f, y);
        window.draw(t);
    }

    // ── RENDER ────────────────────────────────────────────────
    void render() {
        window.clear(C_BG);

        // ── starfield ─────────────────────────────────────────
        sf::CircleShape star(1.f);
        for (auto &s : stars) {
            uint8_t b = uint8_t(s.brightness);
            star.setFillColor({b, b, b});
            star.setPosition(s.pos);
            window.draw(star);
        }

        // ── screen flash overlay ──────────────────────────────
        if (flash.life > 0.f) {
            sf::RectangleShape overlay({float(W), float(H)});
            sf::Color fc = flash.col;
            fc.a = uint8_t(fc.a * (flash.life / 0.5f));
            overlay.setFillColor(fc);
            window.draw(overlay);
        }

        // ── HUD bar ───────────────────────────────────────────
        {
            sf::RectangleShape bar({float(W), HUD_H - 4.f});
            bar.setFillColor({10, 10, 28});
            window.draw(bar);
            sf::RectangleShape line({float(W), 1.f});
            line.setPosition(0, HUD_H - 4.f);
            line.setFillColor({40, 60, 100});
            window.draw(line);
        }

        if (fontLoaded) {
            tScore.setString("SCORE " + fmt(score));
            tScore.setPosition(14, 10);
            window.draw(tScore);

            tHi.setString("BEST " + fmt(hiScore));
            tHi.setPosition(W/2.f - tHi.getLocalBounds().width/2.f - tHi.getLocalBounds().left, 10);
            window.draw(tHi);

            tLevel.setString("LV " + fmt(level));
            tLevel.setPosition(W - 160.f, 10);
            window.draw(tLevel);

            // life icons
            for (int i = 0; i < lives; i++)
                drawPlayer(window, W - 46.f - i * 26.f, 9.f);
        }

        // ── ground line ───────────────────────────────────────
        {
            sf::RectangleShape g({float(W), 2.f});
            g.setPosition(0, GROUND_Y);
            g.setFillColor(C_GROUND);
            window.draw(g);
        }

        if (state == PLAYING || state == GAME_OVER) {

            // ── shields ───────────────────────────────────────
            for (auto &sc : shields) {
                if (sc.hp <= 0) continue;
                uint8_t bright = uint8_t(50 + sc.hp * 55);
                sf::RectangleShape cell({7.f, 7.f});
                cell.setFillColor({0, bright, 0});
                cell.setPosition(sc.x, sc.y);
                window.draw(cell);
            }

            // ── aliens ────────────────────────────────────────
            for (auto &a : aliens) {
                if (!a.alive) continue;
                sf::Color col = C_A[a.type];
                // blink if invincible (reuse invincTimer for alien flash – simple)
                drawAlienSprite(window, a.x, a.y, a.type, a.animFrame, col);
            }

            // ── UFO ───────────────────────────────────────────
            if (ufo.active) drawUFO(window, ufo.x, ufo.y);

            // ── player (blink when invincible) ────────────────
            bool showPlayer = invincTimer <= 0.f ||
                              int(invincTimer * 10) % 2 == 0;
            if (showPlayer)
                drawPlayer(window, playerX, GROUND_Y - 21.f);

            // ── bullets ───────────────────────────────────────
            sf::RectangleShape bshape({3.f, 12.f});
            for (auto &b : bullets) {
                if (b.friendly) {
                    bshape.setSize({3.f, 12.f});
                    bshape.setFillColor(C_BULLET);
                } else {
                    bshape.setSize({3.f, 9.f});
                    bshape.setFillColor(C_ABUL);
                }
                bshape.setPosition(b.x, b.y);
                window.draw(bshape);
            }

            // ── particles ─────────────────────────────────────
            sf::CircleShape dot(2.f);
            for (auto &p : particles) {
                sf::Color pc = p.col;
                pc.a = uint8_t(255 * (p.life / p.maxLife));
                dot.setFillColor(pc);
                dot.setPosition(p.x - 2, p.y - 2);
                window.draw(dot);
            }

            // ── wave cleared banner ───────────────────────────
            if (waveCleared && fontLoaded) {
                drawCenteredText(tCenter, "WAVE CLEARED!",
                                 H/2.f - 24.f, sf::Color(80,255,160));
                drawCenteredText(tSub, "Level " + fmt(level+1) + " incoming...",
                                 H/2.f + 26.f, sf::Color(180,255,200));
            }
        }

        // ── MENU overlay ──────────────────────────────────────
        if (state == MENU && fontLoaded) {
            sf::RectangleShape dim({float(W), float(H)});
            dim.setFillColor({0,0,0,160});
            window.draw(dim);

            // draw sample aliens as decoration
            for (int i = 0; i < 5; i++)
                drawAlienSprite(window, 180.f + i*90.f, H/2.f - 120.f,
                                i%3, 0, C_A[i%3]);

            drawCenteredText(tCenter, "SPACE INVADERS", H/2.f - 60.f, C_HUD);
            drawCenteredText(tSub,  "PRESS  ENTER  OR  SPACE  TO  START",
                             H/2.f + 10.f, sf::Color(220,220,220));
            drawCenteredText(tSub2, "LEFT / RIGHT  or  A / D  =  MOVE      SPACE / UP  =  FIRE",
                             H/2.f + 46.f, sf::Color(130,130,130));

            // scoring legend
            float lx = W/2.f - 160.f, ly = H/2.f + 90.f;
            for (int t = 0; t < 3; t++) {
                drawAlienSprite(window, lx, ly + t*36.f, t, 0, C_A[t]);
                if (fontLoaded) {
                    tSub2.setString("= " + fmt(A_PTS[t]) + " pts");
                    tSub2.setFillColor(C_A[t]);
                    tSub2.setPosition(lx + 40.f, ly + t*36.f + 4.f);
                    window.draw(tSub2);
                }
            }
            drawUFO(window, lx - 2.f, ly + 3*36.f);
            if (fontLoaded) {
                tSub2.setString("= ??? pts");
                tSub2.setFillColor(C_UFO);
                tSub2.setPosition(lx + 40.f, ly + 3*36.f + 4.f);
                window.draw(tSub2);
            }
        }

        // ── GAME OVER overlay ─────────────────────────────────
        if (state == GAME_OVER && fontLoaded) {
            sf::RectangleShape dim({float(W), float(H)});
            dim.setFillColor({0,0,0,170});
            window.draw(dim);

            drawCenteredText(tCenter, "GAME  OVER",
                             H/2.f - 70.f, sf::Color(255, 60, 60));
            drawCenteredText(tSub,
                             "SCORE: " + fmt(score) + "     BEST: " + fmt(hiScore),
                             H/2.f + 0.f,  sf::Color(200,200,200));
            drawCenteredText(tSub2,
                             "PRESS  ENTER  TO  PLAY  AGAIN",
                             H/2.f + 40.f, sf::Color(140,140,140));
        }

        window.display();
    }
};

// ── Entry point ───────────────────────────────────────────────
int main() {
    Game game;
    game.run();
    return 0;
}
