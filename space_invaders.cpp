// ============================================================
//  Space Invaders — C++ / SFML 2.x
//  Compile: g++ space_invaders.cpp -o space_invaders -lsfml-graphics -lsfml-window -lsfml-system
// ============================================================
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>
#include <cmath>

// ── Constants ────────────────────────────────────────────────
const int W = 800, H = 600;
const int ALIEN_COLS = 11, ALIEN_ROWS = 5;
const float PLAYER_SPEED = 280.f;
const float BULLET_SPEED = 500.f;
const float ALIEN_BULLET_SPEED = 220.f;
const float ALIEN_DROP = 14.f;

// ── Data types ───────────────────────────────────────────────
struct Bullet {
    sf::RectangleShape shape;
    bool active = true;
    bool friendly;  // true = player bullet
    Bullet(float x, float y, bool friendly)
        : friendly(friendly)
    {
        shape.setSize({3.f, friendly ? 12.f : 8.f});
        shape.setPosition(x, y);
        shape.setFillColor(friendly ? sf::Color::White : sf::Color(255, 120, 40));
    }
};

struct Alien {
    sf::RectangleShape shape;
    int type;  // 0=bottom(30pts) 1=mid(20pts) 2=top(10pts)
    bool alive = true;
    float animTimer = 0;
    int animFrame = 0;
};

struct ShieldCell {
    sf::RectangleShape shape;
    bool alive = true;
};

struct UFO {
    sf::RectangleShape body;
    bool active = false;
    float timer = 0;
    float speed = 140.f;
};

struct Particle {
    sf::CircleShape shape;
    sf::Vector2f vel;
    float life;
};

// ── Helper: random float ─────────────────────────────────────
float randf(float lo, float hi) {
    return lo + (hi - lo) * (rand() / (float)RAND_MAX);
}

// ── Game class ───────────────────────────────────────────────
class SpaceInvaders {
public:
    SpaceInvaders()
        : window(sf::VideoMode(W, H), "Space Invaders", sf::Style::Titlebar | sf::Style::Close),
          font(), score(0), level(1), lives(3),
          alienDir(1.f), alienSpeed(40.f),
          shootCooldown(0.f), alienShootTimer(0.f),
          state(MENU)
    {
        window.setFramerateLimit(60);
        srand((unsigned)time(nullptr));

        // Try to load a font — fall back to a simple rectangle if unavailable
        // Download a free font (e.g. "PressStart2P-Regular.ttf") and place it next to the binary
        fontLoaded = font.loadFromFile("PressStart2P-Regular.ttf");
        if (!fontLoaded) fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");

        setupHUD();
        initLevel();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            handleEvents();
            if (state == PLAYING) update(dt);
            render();
        }
    }

private:
    enum State { MENU, PLAYING, GAME_OVER, WIN };

    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;

    // Game objects
    sf::RectangleShape player;
    sf::RectangleShape cannon;   // barrel
    std::vector<Bullet> bullets;
    std::vector<Alien> aliens;
    std::vector<ShieldCell> shieldCells;
    UFO ufo;
    std::vector<Particle> particles;

    // HUD
    sf::Text scoreText, levelText, livesText, centerText, subText;

    // State
    int score, level, lives;
    float alienDir, alienSpeed;
    float shootCooldown, alienShootTimer, alienShootInterval;
    State state;

    // ── Setup ──────────────────────────────────────────────
    void setupHUD() {
        auto initText = [&](sf::Text &t, unsigned sz, sf::Color col) {
            if (fontLoaded) t.setFont(font);
            t.setCharacterSize(sz);
            t.setFillColor(col);
        };
        initText(scoreText,  16, sf::Color(100, 200, 255));
        initText(levelText,  16, sf::Color(100, 200, 255));
        initText(livesText,  16, sf::Color(100, 200, 255));
        initText(centerText, 40, sf::Color(100, 200, 255));
        initText(subText,    16, sf::Color(180, 180, 180));
    }

    void initLevel() {
        bullets.clear();
        particles.clear();

        // Player
        player.setSize({36.f, 24.f});
        player.setPosition(W / 2.f - 18.f, H - 60.f);
        player.setFillColor(sf::Color(80, 180, 255));

        cannon.setSize({6.f, 12.f});
        cannon.setFillColor(sf::Color(80, 180, 255));

        // Aliens
        aliens.clear();
        alienDir = 1.f;
        alienSpeed = 40.f + level * 6.f;
        alienShootInterval = std::max(0.4f, 1.2f - level * 0.08f);
        alienShootTimer = 0.f;

        int types[ALIEN_ROWS] = {2, 2, 1, 1, 0};
        for (int r = 0; r < ALIEN_ROWS; r++) {
            for (int c = 0; c < ALIEN_COLS; c++) {
                Alien a;
                a.type = types[r];
                a.shape.setSize({28.f, 20.f});
                a.shape.setPosition(60.f + c * 56.f, 60.f + r * 44.f);
                a.shape.setFillColor(
                    a.type == 0 ? sf::Color(80, 255, 140) :
                    a.type == 1 ? sf::Color(180, 100, 255) :
                                  sf::Color(255, 140, 60));
                aliens.push_back(a);
            }
        }

        // Shields
        shieldCells.clear();
        for (int s = 0; s < 4; s++) {
            float sx = 70.f + s * 180.f;
            float sy = H - 130.f;
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 8; c++) {
                    bool skip = (r == 0 && (c <= 1 || c >= 6)) ||
                                (r == 3 && (c == 3 || c == 4));
                    if (skip) continue;
                    ShieldCell sc;
                    sc.shape.setSize({9.f, 9.f});
                    sc.shape.setPosition(sx + c * 9.f, sy + r * 9.f);
                    sc.shape.setFillColor(sf::Color(60, 220, 60));
                    shieldCells.push_back(sc);
                }
            }
        }

        // UFO
        ufo.active = false;
        ufo.timer = randf(8.f, 18.f);
        ufo.body.setSize({44.f, 18.f});
        ufo.body.setFillColor(sf::Color(220, 60, 60));
        ufo.speed = 120.f + level * 15.f;
    }

    // ── Events ─────────────────────────────────────────────
    void handleEvents() {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) window.close();
                if ((state == MENU || state == GAME_OVER || state == WIN) &&
                    ev.key.code == sf::Keyboard::Enter) {
                    score = 0; level = 1; lives = 3;
                    state = PLAYING;
                    initLevel();
                }
            }
        }
    }

    // ── Update ─────────────────────────────────────────────
    void update(float dt) {
        shootCooldown -= dt;

        // Player movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.move(-PLAYER_SPEED * dt, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.move(PLAYER_SPEED * dt, 0);
        }
        float px = player.getPosition().x;
        if (px < 0) player.setPosition(0, player.getPosition().y);
        if (px + 36 > W) player.setPosition(W - 36.f, player.getPosition().y);

        // Cannon position
        cannon.setPosition(player.getPosition().x + 15.f,
                           player.getPosition().y - 10.f);

        // Shoot
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) &&
            shootCooldown <= 0.f) {
            float bx = player.getPosition().x + 16.5f;
            float by = player.getPosition().y;
            bullets.push_back(Bullet(bx, by, true));
            shootCooldown = 0.35f;
        }

        // Move bullets
        for (auto &b : bullets) {
            float dir = b.friendly ? -1.f : 1.f;
            float spd = b.friendly ? BULLET_SPEED : ALIEN_BULLET_SPEED;
            b.shape.move(0, dir * spd * dt);
            if (b.shape.getPosition().y < 0 || b.shape.getPosition().y > H)
                b.active = false;
        }

        // Alien movement
        float minX = W, maxX = 0, maxY = 0;
        for (auto &a : aliens) {
            if (!a.alive) continue;
            auto pos = a.shape.getPosition();
            minX = std::min(minX, pos.x);
            maxX = std::max(maxX, pos.x + 28.f);
            maxY = std::max(maxY, pos.y + 20.f);
        }
        bool drop = false;
        if (maxX >= W - 10 && alienDir > 0) drop = true;
        if (minX <= 10 && alienDir < 0) drop = true;
        if (drop) {
            alienDir = -alienDir;
            for (auto &a : aliens) if (a.alive) a.shape.move(0, ALIEN_DROP);
        }
        for (auto &a : aliens) {
            if (!a.alive) continue;
            a.shape.move(alienDir * alienSpeed * dt, 0);
            a.animTimer += dt;
            if (a.animTimer > 0.5f) { a.animTimer = 0; a.animFrame ^= 1; }
        }

        // Alien reaches player row → game over
        if (maxY >= player.getPosition().y - 5.f) { endGame(false); return; }

        // Alien shooting
        alienShootTimer += dt;
        if (alienShootTimer >= alienShootInterval) {
            alienShootTimer = 0;
            std::vector<Alien *> alive;
            for (auto &a : aliens) if (a.alive) alive.push_back(&a);
            if (!alive.empty()) {
                Alien *shooter = alive[rand() % alive.size()];
                auto pos = shooter->shape.getPosition();
                bullets.push_back(Bullet(pos.x + 12.f, pos.y + 20.f, false));
            }
        }

        // UFO
        ufo.timer -= dt;
        if (ufo.timer <= 0 && !ufo.active) {
            ufo.active = true;
            ufo.body.setPosition(-50.f, 24.f);
            ufo.timer = randf(8.f, 18.f);
        }
        if (ufo.active) {
            ufo.body.move(ufo.speed * dt, 0);
            if (ufo.body.getPosition().x > W + 60) ufo.active = false;
        }

        // Collision: player bullets vs aliens
        for (auto &b : bullets) {
            if (!b.friendly || !b.active) continue;
            sf::FloatRect br = b.shape.getGlobalBounds();
            for (auto &a : aliens) {
                if (!a.alive) continue;
                if (br.intersects(a.shape.getGlobalBounds())) {
                    a.alive = false;
                    b.active = false;
                    int pts = a.type == 0 ? 30 : a.type == 1 ? 20 : 10;
                    score += pts;
                    spawnParticles(a.shape.getPosition().x + 14,
                                   a.shape.getPosition().y + 10,
                                   a.shape.getFillColor(), 8);
                    break;
                }
            }
        }

        // Collision: player bullets vs UFO
        if (ufo.active) {
            for (auto &b : bullets) {
                if (!b.friendly || !b.active) continue;
                if (b.shape.getGlobalBounds().intersects(ufo.body.getGlobalBounds())) {
                    b.active = false;
                    ufo.active = false;
                    score += 150;
                    spawnParticles(ufo.body.getPosition().x + 22, ufo.body.getPosition().y + 9,
                                   sf::Color(220, 60, 60), 12);
                }
            }
        }

        // Collision: bullets vs shields
        for (auto &b : bullets) {
            if (!b.active) continue;
            sf::FloatRect br = b.shape.getGlobalBounds();
            for (auto &sc : shieldCells) {
                if (!sc.alive) continue;
                if (br.intersects(sc.shape.getGlobalBounds())) {
                    sc.alive = false;
                    b.active = false;
                    break;
                }
            }
        }

        // Collision: alien bullets vs player
        for (auto &b : bullets) {
            if (b.friendly || !b.active) continue;
            if (b.shape.getGlobalBounds().intersects(player.getGlobalBounds())) {
                b.active = false;
                lives--;
                spawnParticles(player.getPosition().x + 18,
                               player.getPosition().y + 12,
                               sf::Color(80, 180, 255), 10);
                if (lives <= 0) { endGame(false); return; }
                player.setPosition(W / 2.f - 18.f, H - 60.f);
            }
        }

        // Clean up dead bullets
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                     [](const Bullet &b){ return !b.active; }),
                      bullets.end());

        // Particles
        for (auto &p : particles) {
            p.shape.move(p.vel);
            p.vel.y += 0.15f;
            p.life -= dt;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
                                       [](const Particle &p){ return p.life <= 0; }),
                        particles.end());

        // All aliens dead → next level
        if (std::all_of(aliens.begin(), aliens.end(), [](const Alien &a){ return !a.alive; })) {
            level++;
            initLevel();
        }
    }

    // ── Helpers ────────────────────────────────────────────
    void spawnParticles(float x, float y, sf::Color col, int n) {
        for (int i = 0; i < n; i++) {
            Particle p;
            p.shape.setRadius(2.f);
            p.shape.setFillColor(col);
            p.shape.setPosition(x, y);
            p.vel = {randf(-2.5f, 2.5f), randf(-4.f, 0.5f)};
            p.life = randf(0.4f, 0.9f);
            particles.push_back(p);
        }
    }

    void endGame(bool win) {
        state = win ? WIN : GAME_OVER;
    }

    // ── Render ─────────────────────────────────────────────
    void setTextCentered(sf::Text &t, const std::string &s, float y) {
        t.setString(s);
        sf::FloatRect r = t.getLocalBounds();
        t.setPosition((W - r.width) / 2.f - r.left, y);
    }

    void render() {
        window.clear(sf::Color(5, 5, 15));

        if (state == PLAYING) {
            // Shields
            for (auto &sc : shieldCells)
                if (sc.alive) window.draw(sc.shape);

            // Aliens (simple pixel-art style using rectangles)
            for (auto &a : aliens) {
                if (!a.alive) continue;
                window.draw(a.shape);
                // Simple "legs" animation
                sf::RectangleShape leg({5.f, 6.f});
                leg.setFillColor(a.shape.getFillColor());
                auto pos = a.shape.getPosition();
                float off = a.animFrame ? 4.f : 0.f;
                leg.setPosition(pos.x + off,       pos.y + 20.f);
                window.draw(leg);
                leg.setPosition(pos.x + 20.f - off, pos.y + 20.f);
                window.draw(leg);
            }

            // UFO
            if (ufo.active) window.draw(ufo.body);

            // Player
            window.draw(player);
            window.draw(cannon);

            // Bullets
            for (auto &b : bullets) window.draw(b.shape);

            // Particles
            for (auto &p : particles) window.draw(p.shape);

            // Ground line
            sf::RectangleShape ground({(float)W, 1.f});
            ground.setPosition(0, H - 40.f);
            ground.setFillColor(sf::Color(60, 180, 60));
            window.draw(ground);

            // HUD
            scoreText.setString("Score: " + std::to_string(score));
            scoreText.setPosition(10, 8);
            window.draw(scoreText);

            levelText.setString("Level: " + std::to_string(level));
            levelText.setPosition(W / 2.f - 40.f, 8.f);
            window.draw(levelText);

            livesText.setString("Lives: " + std::to_string(lives));
            livesText.setPosition(W - 120.f, 8.f);
            window.draw(livesText);
        }

        // Overlay screens
        if (state == MENU) {
            centerText.setFillColor(sf::Color(100, 200, 255));
            setTextCentered(centerText, "SPACE INVADERS", H / 2.f - 60.f);
            window.draw(centerText);
            subText.setString("Press ENTER to start");
            sf::FloatRect r = subText.getLocalBounds();
            subText.setPosition((W - r.width) / 2.f, H / 2.f + 20.f);
            window.draw(subText);
            subText.setString("Arrows / A-D = Move   |   Space = Fire");
            r = subText.getLocalBounds();
            subText.setPosition((W - r.width) / 2.f, H / 2.f + 50.f);
            window.draw(subText);
        }

        if (state == GAME_OVER) {
            centerText.setFillColor(sf::Color(255, 60, 60));
            setTextCentered(centerText, "GAME OVER", H / 2.f - 60.f);
            window.draw(centerText);
            subText.setString("Score: " + std::to_string(score) + "   |   Press ENTER");
            sf::FloatRect r = subText.getLocalBounds();
            subText.setPosition((W - r.width) / 2.f, H / 2.f + 20.f);
            window.draw(subText);
        }

        if (state == WIN) {
            centerText.setFillColor(sf::Color(60, 255, 120));
            setTextCentered(centerText, "YOU WIN!", H / 2.f - 60.f);
            window.draw(centerText);
            subText.setString("Score: " + std::to_string(score) + "   |   Press ENTER");
            sf::FloatRect r = subText.getLocalBounds();
            subText.setPosition((W - r.width) / 2.f, H / 2.f + 20.f);
            window.draw(subText);
        }

        window.display();
    }
};

// ── Entry point ──────────────────────────────────────────────
int main() {
    SpaceInvaders game;
    game.run();
    return 0;
}
