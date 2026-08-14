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
const float blocco_size = 32.0f;

//player
const sf::Vector2f mario_pos_iniziale = {100.0f, 416.0f};

//costanti per la fisica
const float gravita = 1500.0f;         // Accelerazione verso il basso
const float forza_salto = -650.0f;     // Impulso verso l'alto (velocità iniziale y)
const float accel_movimento = 1000.0f; // Quanto velocemente prende velocità Mario
const float attrito_corsa = 10.0f;           // Smorzamento del movimento orizzontale
const float attrito_scivolata = 3.0f;
const float max_velocita_terra_x = 300.0f;   // Limite di velocità orizzontale
const float max_velocita_aria_x = 200.0f;   //limite velocità orizzontale quando in aria
const float accel_aria = 0.5f;
const float max_velocita_caduta_muro = 120.0f;
                                //mondo
//-------------------------------------------------------------------------//
struct Mondo {
    sf::Texture texture_blocco;
    sf::Sprite sprite_blocco;
    std::vector<std::string> mappa;
    sf::Texture texture_tubo;
    sf::Sprite sprite_tubo;

    Mondo();

    //funzioni per la view
    float getLarghezzaPx();
    float getAltezzaPx();
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
    
        sf::Color sfondo_moneta = sf::Color(148,148,255); //ricavato tramite Photopea
        image_tubo.createMaskFromColor(sfondo_moneta);
        if (!texture_tubo.loadFromImage(image_tubo)) 
            std::cerr << "Errore nella creazione della texture tubi dall'immagine!" << std::endl;
    

    sprite_blocco.setScale({2.0f, 2.0f}); // Scala i blocchi 16x16 a 32x32
    sprite_tubo.setScale({2.0f, 2.0f}); // Scala i blocchi 16x16 a 32x32
}

float Mondo::getLarghezzaPx(){
    if(mappa.empty())
        return 0.0f;
    else
        return mappa[0].size() * blocco_size;
}

float Mondo::getAltezzaPx(){
    if(mappa.empty())
        return 0.0f;
    else
        return mappa.size() * blocco_size;
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
    for (size_t r = 0; r < mappa.size(); r++) {
        for (size_t c = 0; c < mappa[r].size(); c++) {
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

bool BloccoSolido(char blocco){
    if(blocco == 'X' || blocco == '#' || blocco == 'S' || blocco == '?' || blocco == 'Q' || 
       blocco == '<' || blocco == '>' || blocco == '[' || blocco == ']')
        return true;
    else
        return false;
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
    bool sta_accovacciato = false; //per farlo abbassare
    bool schiacciata = false; //per fare la caduta rapida
    //per il salto dal muro
    bool tocca_muro_sx = false;
    bool tocca_muro_dx = false;
    bool scivolata_al_muro = false;

    //grafica
    sf::Texture texture;
    sf::Sprite sprite;

    sf::Texture tex_mov_aggiuntivi;
    sf::Sprite spr_mov_aggiuntivi;

    //animazione corsa
    float timer = 0.0f;
    int frame_corrente = 0;
    bool guarda_a_destra = true;

    Player();
    sf::Vector2f getSize();

    //metodi ausiliari per collisioni
    sf::FloatRect getBounds();
    void collisioniX(const Mondo& mondo);
    void collisioniY(const Mondo& mondo);

    void inputMovimento();
    void aggiuntaFisica(float dt, const Mondo& mondo);
    void aggiuntaAnimazioni(float dt);
    void update(float dt, const Mondo& mondo);
    void draw(sf::RenderWindow& window);
};

Player::Player() : sprite(texture) , spr_mov_aggiuntivi(tex_mov_aggiuntivi) {
    player.setPosition(mario_pos_iniziale);
    
    //movimenti base
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

    //movimenti aggiuntivi
    sf::Image image_aggiuntivi;
    if(!image_aggiuntivi.loadFromFile("risorse/Custom _ Edited - Mario Customs - Mario - Mario (Super Mario Bros. 1 NES-Style) (1).png"))
        std::cerr << "Errore nel caricamento della texture aggiuntive di Mario" << std::endl;

    //rimuove il colore di sfondo leggendolo dal primo pixel (0,0)
    sf::Color sfondo_aggiuntivi = image_aggiuntivi.getPixel({0, 8});
    image_aggiuntivi.createMaskFromColor(sfondo_aggiuntivi);

    if(!tex_mov_aggiuntivi.loadFromImage(image_aggiuntivi))
        std::cerr << "Errore nel creazione della texture dall'immagine" << std::endl;

    spr_mov_aggiuntivi.setTexture(tex_mov_aggiuntivi);
    spr_mov_aggiuntivi.setTextureRect(sf::IntRect({0, 8}, {16, 16}));
    spr_mov_aggiuntivi.setScale({2.0f, 2.0f});
}

sf::Vector2f Player::getSize(){
    if(stato == StatoMario::piccolo || sta_accovacciato)
        return {28.0f, 32.0f};
    else
        return {28.0f, 64.0f};
}

//per  ottenere il rettangolo di collisione
sf::FloatRect Player::getBounds(){
    return sf::FloatRect(pos, getSize());
}

void Player::collisioniX(const Mondo& mondo){
    sf::FloatRect boundsMario = getBounds();
    
    tocca_muro_sx = false;
    tocca_muro_dx = false;

    //ottengo le righe e colonne rispettive della posizione di Mario
    int minC = static_cast<int>(boundsMario.position.x / blocco_size);
    int maxC = static_cast<int>((boundsMario.position.x + boundsMario.size.x) / blocco_size);
    int minR = static_cast<int>(boundsMario.position.y / blocco_size);
    int maxR = static_cast<int>((boundsMario.position.y + boundsMario.size.y) / blocco_size);

    for(int r = minR; r <= maxR; r++){
        for(int c = minC; c <= maxC; c++){
            //controllo sia all'interno della mappa
            if(r >= 0 && r < static_cast<int>(mondo.mappa.size()) &&
            c >= 0 && c < static_cast<int>(mondo.mappa[r].size())){

                if(BloccoSolido(mondo.mappa[r][c])){
                    //creao la hitbox del blocco
                    sf::FloatRect hitboxBlocco({c * blocco_size, r * blocco_size}, {blocco_size, blocco_size});

                    if(auto inters = boundsMario.findIntersection(hitboxBlocco)){ //quanti px Mario è dento al blocco
                        if(velocita.x > 0){//Mario va verso destra
                            pos.x -= inters->size.x; //spingi a sinistra
                            tocca_muro_dx = true; //per la scivolata a muro
                        }
                        else if(velocita.x < 0){//Mario va verso sinistra
                            pos.x += inters->size.x; //spingi a destra                        
                            tocca_muro_sx = true; //per la scivolata a muro
                        }

                        velocita.x = 0.0f;
                        boundsMario = getBounds();
                    }
                }
            }
        }
    }

    //per evitare che Mario cada a sinistra dell'inizio del livello
    if(pos.x < 0.0f){
        pos.x = 0.0f;
        velocita.x = 0.0f;
    }

    //per evitare che Mario vada oltre il limite destro del livello
    float max_x_mappa = mondo.mappa[0].size() * blocco_size;
    float bordo_destro_mario = pos.x + getSize().x;

    if(bordo_destro_mario > max_x_mappa){
        pos.x = max_x_mappa - getSize().x;
        velocita.x = 0.0f;
    }
}

void Player::collisioniY(const Mondo& mondo){
    sf::FloatRect boundsMario = getBounds();
    sul_terreno = false; 

    //ottengo le righe e colonne rispettive della posizione di Mario
    int minC = static_cast<int>(boundsMario.position.x / blocco_size);
    int maxC = static_cast<int>((boundsMario.position.x + boundsMario.size.x) / blocco_size);
    int minR = static_cast<int>(boundsMario.position.y / blocco_size);
    int maxR = static_cast<int>((boundsMario.position.y + boundsMario.size.y) / blocco_size);

    for(int r = minR; r <= maxR; r++){
        for(int c = minC; c <= maxC; c++){
            //controllo sia all'interno della mappa
            if(r >= 0 && r < static_cast<int>(mondo.mappa.size()) &&
            c >= 0 && c < static_cast<int>(mondo.mappa[r].size())){

                if(BloccoSolido(mondo.mappa[r][c])){
                    //creao la hitbox del blocco
                    sf::FloatRect hitboxBlocco({c * blocco_size, r * blocco_size}, {blocco_size, blocco_size});

                    if(auto inters = boundsMario.findIntersection(hitboxBlocco)){ //quanti px Mario è dentro al blocco
                        if(velocita.y > 0){ //quando cade
                            pos.y -= inters->size.y; 
                            velocita.y = 0.0f;
                            sul_terreno = true;
                        }
                        else if(velocita.y < 0){ //qunado salta
                            pos.y += inters->size.y;                         
                            velocita.y = 0.0f;
                        }
                        
                        boundsMario = getBounds();
                    }
                }
            }
        }
    }
    //per far respawnare Mario quando cade
    if(pos.y > window_height){
        pos = mario_pos_iniziale;
        velocita.x = 0.0f;
    }
}

void Player::inputMovimento(){
    accelerazione.x = 0.0f;
    float accel_corrente = accel_movimento;

    if(sul_terreno)
        schiacciata = false;

    if(!sul_terreno)
        accel_corrente *= accel_aria; //per limitare la velocità in aria

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            accelerazione.x = -accel_corrente;
            guarda_a_destra = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            accelerazione.x = +accel_corrente;
            guarda_a_destra = true;
    }
    bool premuto_salto = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || 
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);

    if(premuto_salto){
        if(sul_terreno){
            velocita.y = forza_salto; // Impulso istantaneo verso l'alto
            sul_terreno = false;
        }
        else if(scivolata_al_muro){
            velocita.y = forza_salto;
            float forza_rimbalzo_dal_muro = 350.0f; //per staccarsi dal muro

            if(tocca_muro_dx){
                velocita.x = -forza_rimbalzo_dal_muro;
                guarda_a_destra = false;
            }
            else if(tocca_muro_sx){
                velocita.x = forza_rimbalzo_dal_muro;
                guarda_a_destra = true;
            }
            scivolata_al_muro = false;
        }
    }
    
    
    
    bool premuto_giu = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    
    if(premuto_giu && sul_terreno){ //per accovacciarsi
        sta_accovacciato = true;
        accelerazione.x = 0.0f;
    }
    else if(premuto_giu && !sul_terreno){   //per la schiacciata
        schiacciata = true;
        velocita.x = 0.0f;
        velocita.y = 800.0f;
    }
    else{
        sta_accovacciato = false;
    }
}

void Player::aggiuntaFisica(float dt, const Mondo& mondo){
    accelerazione.y = gravita;
    velocita += accelerazione * dt;

    //fisica scivolata a muro
    bool spinge_destra = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    bool spinge_sinistra = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);

    bool aderisce_al_muro = (tocca_muro_dx && spinge_destra) || (tocca_muro_sx && spinge_sinistra);

    scivolata_al_muro = !sul_terreno && aderisce_al_muro && velocita.y > 0.0f;

    if(scivolata_al_muro && (velocita.y > max_velocita_caduta_muro))
        velocita.y = max_velocita_caduta_muro;
    
    //fisica movimenti normali
    float max_vel = sul_terreno ? max_velocita_terra_x : max_velocita_aria_x;

    if(velocita.x > max_vel)
        velocita.x = max_vel;
    if(velocita.x < -max_vel)
        velocita.x = -max_vel;
    

    if(accelerazione.x == 0.0f){
        float attrito_corrente = (sta_accovacciato && sul_terreno) ? attrito_scivolata : attrito_corsa; //per fare scivolare Mario
        velocita.x -= velocita.x * attrito_corrente * dt; //per lo smorzamento della velocità
    }

    pos.x += velocita.x * dt;
    collisioniX(mondo);
    
    pos.y += velocita.y * dt;
    collisioniY(mondo);
}

void Player::aggiuntaAnimazioni(float dt){
    //Coordinate X esatte per i 3 frame della corsa prese con Photopea.com
    const int frames_corsa_x[] = {20, 38, 56};
    float offsetY = 0.0f; //serve per mario accucciato perchè senno fluttuerebbe
    
    if(schiacciata &&!sul_terreno){
        spr_mov_aggiuntivi.setTextureRect(sf::IntRect({240,263}, {14,14})); //schiacciata
    }

    else if(scivolata_al_muro){ //per la scivolata al muro
        spr_mov_aggiuntivi.setTextureRect(sf::IntRect({271,315}, {16,19}));
        if(tocca_muro_dx)
            guarda_a_destra = false;
        if(tocca_muro_sx)
            guarda_a_destra = true;
    }

    else if(!sul_terreno)
        sprite.setTextureRect(sf::IntRect({96, 8}, {16, 16}));

    else if(sta_accovacciato && sul_terreno){ 
        if(std::abs(velocita.x) > 20.0f){
            spr_mov_aggiuntivi.setTextureRect(sf::IntRect({237,340}, {15,15}));//scivolata
        }
        else{
            spr_mov_aggiuntivi.setTextureRect(sf::IntRect({447,264}, {13,13}));//accucciato
            offsetY = 6.0f;
            frame_corrente = 0;
            timer = 0.0f;
        }
    }
    
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
        spr_mov_aggiuntivi.setScale({2.0f, 2.0f}); //cambiano perchè la sprite non è 16*16 ma 17*17
        spr_mov_aggiuntivi.setPosition({pos.x, pos.y + offsetY});
    }
    else{
        sprite.setScale({-2.0f, 2.0f}); //ribalta la sprite
        sprite.setPosition({pos.x + 32.0f, pos.y});
        spr_mov_aggiuntivi.setScale({-2.0f, 2.0f});
        spr_mov_aggiuntivi.setPosition({pos.x + 32.0f, pos.y + offsetY});
    }
}
void Player::update(float dt, const Mondo& mondo) {
    inputMovimento();
    aggiuntaFisica(dt, mondo);
    aggiuntaAnimazioni(dt);
}

void Player::draw(sf::RenderWindow& window){
    //per scegliere la sprite da usare
    if((sta_accovacciato && sul_terreno) || (schiacciata && !sul_terreno) || scivolata_al_muro)
        window.draw(spr_mov_aggiuntivi);
    else
        window.draw(sprite);
}

//-------------------------------------------------------------------------//

                            //state
//-------------------------------------------------------------------------//
struct State {
    Mondo mondo;
    Player mario;

    sf::View camera;

    bool pausa = false;
    bool focus = true;

    State();
    void aggiornaCamera();
    void update(float dt) {
        if(!pausa){
            mario.update(dt, mondo);
            aggiornaCamera();
        }
    }

    void draw(sf::RenderWindow& window) {
        window.setView(camera);
        mondo.draw(window);
        mario.draw(window);
    }
};

State::State(){
    //inizializzo la camera con le dimensioni della window
    camera.setSize({static_cast<float>(window_width), static_cast<float>(window_height)});
}

void State::aggiornaCamera(){
    float meta_schermo_x = window_width / 2.0f;
    float meta_schermo_y = window_height / 2.0f;

    float centro_mario_x = mario.pos.x + mario.getSize().x / 2.0f;

    float min_x = meta_schermo_x;
    float max_x = std::max(min_x, mondo.getLarghezzaPx() - meta_schermo_x);
    
    float centro_camera_y = meta_schermo_y; //lo mantengo fisso
    float centro_camera_x = centro_mario_x;
    if(centro_camera_x < min_x)
        centro_camera_x = min_x;
    if(centro_camera_x > max_x)
        centro_camera_x = max_x;

    camera.setCenter({centro_camera_x, centro_camera_y});
}
//-------------------------------------------------------------------------//


                            //handle
//-------------------------------------------------------------------------//
void handle_close (sf::RenderWindow& window){
    window.close();
}

void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window){   
    float aspect = static_cast<float>(window_width)/static_cast<float>(window_height);
    sf::Vector2u ws = resized.size;
    float new_aspect = static_cast<float>(ws.x)/static_cast<float>(ws.y);
    if (new_aspect < aspect)
        ws = {ws.x,static_cast<unsigned>(ws.x/aspect)};
    else
        ws = {static_cast<unsigned>(ws.y*aspect),ws.y};
    window.setSize(ws);
}

void handle(const sf::Event::FocusGained&, State& state){
    state.focus = true;
    state.pausa = false;
}

void handle(const sf::Event::FocusLost& event, State& state){
    state.pausa = true;
    state.focus = false;
}

template <typename T>
void handle(const T &, State &gs){
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
        window.handleEvents(
            [&window](const sf::Event::Closed&) { handle_close (window); },
            [&window](const sf::Event::Resized& event) { handle_resize (event, window); },
            [&state](const auto& event){ handle(event, state);}
        );

        float dt = clock.restart().asSeconds();
        state.update(dt);
        window.clear(sf::Color(107, 140, 255));
        state.draw(window);
        window.display();
    }
}