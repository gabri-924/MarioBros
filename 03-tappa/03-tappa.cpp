#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>

//window
const char* window_title = "Mario Bros";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

//terreno
const sf::Vector2f ground_size = {800.0f, 50.0f};
const sf::Color ground_color = sf::Color(139, 69, 19);
const sf::Vector2f ground_pos = {0.0f, 550.0f};

//player
const sf::Vector2f mario_size = {30.0f, 50.0f};
const sf::Color mario_color = sf::Color::Red;
const sf::Vector2f mario_pos_iniziale = {100.0f, 500.0f};

//costanti per la fisica
const float gravita = 1500.0f;         // Accelerazione verso il basso
const float forza_salto = -600.0f;     // Impulso verso l'alto (velocità iniziale y)
const float accel_movimento = 1000.0f; // Quanto velocemente prende velocità Mario
const float attrito = 10.0f;           // Smorzamento del movimento orizzontale
const float max_velocita_x = 350.0f;   // Limite di velocità orizzontale

                                //mondo
//-------------------------------------------------------------------------//
struct Mondo {
    sf::RectangleShape ground; 

    Mondo(); 
    void draw(sf::RenderWindow& window);
};

Mondo::Mondo() {
    ground.setSize(ground_size);
    ground.setPosition(ground_pos);
    ground.setFillColor(ground_color);
}

void Mondo::draw(sf::RenderWindow& window) {
    window.draw(ground);
}
//-------------------------------------------------------------------------//

                                //mario
//-------------------------------------------------------------------------//
struct Player{
    sf::RectangleShape player;
    float velocita_movimento = 300.0f;
    sf::Vector2f pos = mario_pos_iniziale;
    sf::Vector2f velocita = {0.0f, 0.0f};
    sf::Vector2f accelerazione = {0.0f, 0.0f};

    bool sul_terreno = false; //per verificare se può saltare

    Player();
    void update(float dt);
    void draw(sf::RenderWindow& window);
};

Player::Player(){
    player.setSize(mario_size);
    player.setPosition(mario_pos_iniziale);
    player.setFillColor(mario_color);
}

void Player::update(float dt) {
    accelerazione.x = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            accelerazione.x = -accel_movimento;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            accelerazione.x = +accel_movimento;
    }

    accelerazione.y = gravita;

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) && sul_terreno) {
        velocita.y = forza_salto; // Impulso istantaneo verso l'alto
        sul_terreno = false;
    }

    velocita += accelerazione * dt;

    if(accelerazione.x == 0.0f){
        velocita.x -= velocita.x * attrito * dt; //per lo smorzamento della velocità
    }

    pos += velocita * dt;

    if (pos.y >= mario_pos_iniziale.y) {
        pos.y = mario_pos_iniziale.y;
        velocita.y = 0.0f;
        sul_terreno = true;
    } else {
        sul_terreno = false;
    }

    player.setPosition(pos);
}

void Player::draw(sf::RenderWindow& window){
    window.draw(player);
}

//-------------------------------------------------------------------------//

                            //state
//-------------------------------------------------------------------------//
struct State {
    Mondo mondo;
    Player mario;

    bool pausa = false;
    bool focus = true;

   void update(float dt) {
        if(pausa)
            return;
        mario.update(dt);
    }

    void draw(sf::RenderWindow& window) {
        mondo.draw(window);
        mario.draw(window);
    }
};
//-------------------------------------------------------------------------//


                            //handle
//-------------------------------------------------------------------------//
void handle_close (sf::RenderWindow& window)
{
    window.close();
}

void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window)
{   
    float aspect = static_cast<float>(window_width)/static_cast<float>(window_height);
    sf::Vector2u ws = resized.size;
    float new_aspect = static_cast<float>(ws.x)/static_cast<float>(ws.y);
    if (new_aspect < aspect)
        ws = {ws.x,static_cast<unsigned>(ws.x/aspect)};
        else
        ws = {static_cast<unsigned>(ws.y*aspect),ws.y};
    window.setSize(ws);
}

void handle(const sf::Event::FocusGained&, State& state)
{
    state.focus = true;
    state.pausa = false;
}

void handle(const sf::Event::FocusLost& event, State& state){
    state.pausa = true;
    state.focus = false;
}

template <typename T>
void handle(const T &, State &gs)
{
    // All unhandled events will end up here
}

//-------------------------------------------------------------------------//

int main(){
    sf::RenderWindow window(sf::VideoMode ({window_width, window_height}), window_title);
    window.setFramerateLimit(max_frame_rate);
    window.setMinimumSize(window.getSize());

    sf::Clock clock;
    State state;

    while(window.isOpen()){

        float dt = clock.restart().asSeconds();

        window.handleEvents(
            [&window](const sf::Event::Closed&) { handle_close (window); },
            [&window](const sf::Event::Resized& event) { handle_resize (event, window); },
            [&state](const auto& event){ handle(event, state);}
        );

        state.update(dt);
        window.clear(sf::Color::Black);
        state.draw(window);
        window.display();
    }
}