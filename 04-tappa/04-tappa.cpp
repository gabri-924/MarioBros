#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>
#include <iostream>
#include <fstream>

//window
const char* window_title = "Mario Bros";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

//terreno
const sf::Vector2f ground_pos = {0.0f, 416.0f}; //416 => 13(riga 14 della mappa) * 32px 

//player
const sf::Vector2f mario_size = {30.0f, 50.0f};
const sf::Vector2f mario_pos_iniziale = {100.0f, 416.0f};

//costanti per la fisica
const float gravita = 1500.0f;         // Accelerazione verso il basso
const float forza_salto = -650.0f;     // Impulso verso l'alto (velocità iniziale y)
const float accel_movimento = 1000.0f; // Quanto velocemente prende velocità Mario
const float attrito = 10.0f;           // Smorzamento del movimento orizzontale
const float max_velocita_x = 350.0f;   // Limite di velocità orizzontale

                                //mondo
//-------------------------------------------------------------------------//
struct Mondo {
    sf::Texture texture_blocco;
    sf::Sprite sprite_blocco;
    std::vector<std::string> mappa;
    sf::Texture texture_tubo;
    sf::Sprite sprite_tubo;
    Mondo();
    bool caricaMappa(const std::string& percorsoFile); 
    void draw(sf::RenderWindow& window);
};

Mondo::Mondo() : sprite_blocco(texture_blocco) , sprite_tubo(texture_tubo){
    if (!texture_blocco.loadFromFile("risorse/NES - Super Mario Bros. - Miscellaneous - Items, Objects and NPCs.png")) {
        std::cerr << "Errore nel caricamento del tileset della mappa!" << std::endl;
    }
    sf::Image image_tubo;
    if (!image_tubo.loadFromFile("risorse/NES - Super Mario Bros. - Miscellaneous - Tileset.png")) {
        std::cerr << "Errore nel caricamento del tileset della mappa!" << std::endl;
    }
    else{
        sf::Color sfondo_moneta = sf::Color(148,148,255); //ricavato tramite Photopea
        image_tubo.createMaskFromColor(sfondo_moneta);
        if (!texture_tubo.loadFromImage(image_tubo)) 
            std::cerr << "Errore nella creazione della texture tubi dall'immagine!" << std::endl;
    }

    sprite_blocco.setScale({2.0f, 2.0f}); // Scala i blocchi 16x16 a 32x32
    sprite_tubo.setScale({2.0f, 2.0f}); // Scala i blocchi 16x16 a 32x32
}

bool Mondo::caricaMappa(const std::string& percorsoFile){
    std::ifstream file(percorsoFile);
    if(!file.is_open()){
        std::cerr << "Impossibile aprire il file mappa: " << percorsoFile << std::endl;
        return false;
    }

    std::string riga;
    mappa.clear();
    while (std::getline(file, riga)) {
        if (!riga.empty() && riga.back() == '\r') {
            riga.pop_back(); // Rimuove il ritorno a capo di Windows se presente
        }
        mappa.push_back(riga);
    }
    file.close();
    return true;
}

void Mondo::draw(sf::RenderWindow& window) {
    const float blocco_size = 32.0f;
    
    for (size_t r = 0; r < mappa.size(); ++r) {
        for (size_t c = 0; c < mappa[r].size(); ++c) {
            char blocco = mappa[r][c];

            if (blocco == '-') continue; // Salta l'aria/cielo
        
            sf::Sprite* sprite_attivo = nullptr;

            if (blocco == 'X' || blocco == '#') {
                // Terreno solido
                sprite_blocco.setTextureRect(sf::IntRect({180, 116}, {16, 16}));
                sprite_attivo = &sprite_blocco;
            } 
            else if (blocco == 'S') {
                // Blocco mattone
                sprite_blocco.setTextureRect(sf::IntRect({198, 8}, {16, 16}));
                sprite_attivo = &sprite_blocco;
            } 
            else if (blocco == '?') {
                // Blocco ? pieno
                sprite_blocco.setTextureRect(sf::IntRect({328, 128}, {16, 16}));
                sprite_attivo = &sprite_blocco;
            }
            else if( blocco == 'Q'){
                // Blocco ? vuoto
                sprite_blocco.setTextureRect(sf::IntRect({234, 8}, {16, 16}));
                sprite_attivo = &sprite_blocco;
            }
            else if (blocco == '<') {
                // Cima Tubo (Sinistra)
                sprite_tubo.setTextureRect(sf::IntRect({119, 196}, {16, 16}));
                sprite_attivo = &sprite_tubo;
            }
            else if (blocco == '>') {
                // Cima Tubo (Destra)
                sprite_tubo.setTextureRect(sf::IntRect({136, 196}, {16, 16}));
                sprite_attivo = &sprite_tubo;
            }
            else if (blocco == '[') {
                // lato Tubo (Sinistra)
                sprite_tubo.setTextureRect(sf::IntRect({121, 213}, {14, 16}));
                sprite_attivo = &sprite_tubo;
            }
            else if (blocco == ']') {
                // lato Tubo (Destra)
                sprite_tubo.setTextureRect(sf::IntRect({136, 213}, {14, 16}));
                sprite_attivo = &sprite_tubo;
            }
            else if (blocco == 'o') {
                // moneta
                sprite_tubo.setTextureRect(sf::IntRect({561, 653}, {16, 16}));
                sprite_attivo = &sprite_tubo;
            }

            if (sprite_attivo) {
                sprite_attivo->setPosition({c * blocco_size, r * blocco_size});
                window.draw(*sprite_attivo);
            }
        }
    }
}
//-------------------------------------------------------------------------//

                                //mario
//-------------------------------------------------------------------------//
enum class StatoMario{
    piccolo,
    grande, 
    fiore
};

struct Player{
    sf::RectangleShape player;
    StatoMario stato = StatoMario::piccolo;
    float velocita_movimento = 300.0f;
    sf::Vector2f pos = mario_pos_iniziale;
    sf::Vector2f velocita = {0.0f, 0.0f};
    sf::Vector2f accelerazione = {0.0f, 0.0f};

    bool sul_terreno = false; //per verificare se può saltare

    //grafica
    sf::Texture texture;
    sf::Sprite sprite;

    //animazione corsa
    float timer = 0.0f;
    int frame_corrente = 0;
    bool guarda_a_destra = true;

    Player();
    sf::Vector2f getSize();
    void inputMovimento();
    void aggiuntaFisica(float dt);
    void aggiuntaAnimazioni(float dt);
    void update(float dt);
    void draw(sf::RenderWindow& window);
};

Player::Player() : sprite(texture) {
    player.setSize(mario_size);
    player.setPosition(mario_pos_iniziale);
    
    sf::Image image;
    if(!image.loadFromFile("risorse/NES - Super Mario Bros. - Playable Characters - Mario & Luigi.png"))
        std::cerr << "Errore nel caricamento della texture di Mario" << std::endl;

    //rimuove il colore di sfondo leggendolo dal primo pixel (0,0)
    sf::Color sfondo = image.getPixel({0, 8});
    image.createMaskFromColor(sfondo);

    if(!texture.loadFromImage(image))
        std::cerr << "Errore nel creazione della texture dall'immagine" << std::endl;

    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({0, 8}, {16, 16}));
    sprite.setScale({2.0f, 2.0f});
}

sf::Vector2f Player::getSize(){
    if(stato == StatoMario::piccolo)
        return {28.0f, 32.0f};
    else
        return {28.0f, 64.0f};
}

void Player::inputMovimento(){
    accelerazione.x = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            accelerazione.x = -accel_movimento;
            guarda_a_destra = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            accelerazione.x = +accel_movimento;
            guarda_a_destra = true;
    }
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) && sul_terreno) {
        velocita.y = forza_salto; // Impulso istantaneo verso l'alto
        sul_terreno = false;
    }
}

void Player::aggiuntaFisica(float dt){
    accelerazione.y = gravita;
    velocita += accelerazione * dt;

    if(accelerazione.x == 0.0f){
        velocita.x -= velocita.x * attrito * dt; //per lo smorzamento della velocità
    }

    pos += velocita * dt;
    
    //per evitare che mario fluttui
    float altezza_mario = getSize().y;

    if (pos.y >= ground_pos.y - altezza_mario) {
        pos.y = ground_pos.y - altezza_mario;
        velocita.y = 0.0f;
        sul_terreno = true;
    } else {
        sul_terreno = false;
    }
}

void Player::aggiuntaAnimazioni(float dt){
    //Coordinate X esatte per i 3 frame della corsa prese con Photopea.com
    const int frames_corsa_x[] = {20, 38, 56};

    if(!sul_terreno)
        sprite.setTextureRect(sf::IntRect({96, 8}, {16, 16}));
    else if(std::abs(velocita.x) > 10.0f){
        //mario sta correndo
        timer += dt;

        if(timer >= 0.1f){
            timer = 0.0f;
            frame_corrente = (frame_corrente + 1) % 3; //alterna i frame 0, 1, 2
        }

        int posX = frames_corsa_x[frame_corrente];
        sprite.setTextureRect(sf::IntRect({posX, 8}, {16, 16}));
    }
    else{
        //mario è fermo
        sprite.setTextureRect(sf::IntRect({0, 8}, {16, 16}));
        frame_corrente = 0;
        timer = 0.0f;
    }

    //sguardo
    if(guarda_a_destra){
        sprite.setScale({2.0f, 2.0f});
        sprite.setPosition(pos);
    }
    else{
        sprite.setScale({-2.0f, 2.0f}); //ribalta la sprite
        sprite.setPosition({pos.x + 32.0f, pos.y});
    }
}
void Player::update(float dt) {
    inputMovimento();
    aggiuntaFisica(dt);
    aggiuntaAnimazioni(dt);
}

void Player::draw(sf::RenderWindow& window){
    window.draw(sprite);
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
        if(!pausa)
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
    sf::RenderWindow window(sf::VideoMode ({window_width, window_height}), window_title, sf::Style::Default);
    window.setFramerateLimit(max_frame_rate);
    window.setMinimumSize(window.getSize());

    sf::Clock clock;
    State state;

    if(!state.mondo.caricaMappa("risorse/mario-1-1.txt"))
        std::cerr << "Errore nel caricamento del livello" << std::endl;

    while(window.isOpen()){

        float dt = clock.restart().asSeconds();

        window.handleEvents(
            [&window](const sf::Event::Closed&) { handle_close (window); },
            [&window](const sf::Event::Resized& event) { handle_resize (event, window); },
            [&state](const auto& event){ handle(event, state);}
        );

        state.update(dt);
        window.clear(sf::Color(107, 140, 255));
        state.draw(window);
        window.display();
    }
}