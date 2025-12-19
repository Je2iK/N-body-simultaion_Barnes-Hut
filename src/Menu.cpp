#include "Menu.h"

#include "Utils.h"
#ifdef ENABLE_AUTH
#include "AuthManager.h"
#endif
#include <iostream>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace sf;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

string getButtonText(const string& key) {
    if (key == "BarnesHutSimulation") return u8"Симуляция\nБарнс-Хат";
    if (key == "BruteForceSimulation") return u8"Симуляция\nПеребор";
    if (key == "GalaxyCollision") return u8"Столкновение\nГалактик";
    if (key == "BenchmarkBarnesHut") return u8"Тест алгоритма\nБарнс-Хата";
    if (key == "BenchmarkBruteForce") return u8"Тест алгоритма\nперебора";
    if (key == "CompareAlgorithms") return u8"Сравнить\nАлгоритмы";
    if (key == "EXIT") return u8"ВЫХОД";
    return key;
}

Menu::Menu(float width, float height) : width(width), height(height), current_username("Guest") {
    if (!loadFont(font)) {
        cerr << "Failed to load any font!" << endl;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> x_dist(0, width);
    uniform_real_distribution<> y_dist(0, height);
    uniform_real_distribution<> speed_dist(0.05, 0.3);
    uniform_real_distribution<> size_dist(0.5, 1.5);
    uniform_real_distribution<> bright_dist(80, 150);

    for (int i = 0; i < 100; ++i) {
        stars.push_back({
            (float)x_dist(gen),
            (float)y_dist(gen),
            (float)speed_dist(gen),
            (float)size_dist(gen),
            (float)bright_dist(gen)
        });
    }

    float gridStartY = 320.0f;
    float cardWidth = 380.0f;
    float cardHeight = 90.0f;
    float gap = 20.0f;
    float col1X = width/2 - cardWidth - gap/2;
    float col2X = width/2 + gap/2;
    
    addButton("BarnesHutSimulation", 1, col1X, gridStartY, cardWidth, cardHeight);
    addButton("BruteForceSimulation", 2, col1X, gridStartY + cardHeight + gap, cardWidth, cardHeight);
    addButton("GalaxyCollision", 6, col1X, gridStartY + (cardHeight + gap) * 2, cardWidth, cardHeight);
    
    addButton("BenchmarkBarnesHut", 3, col2X, gridStartY, cardWidth, cardHeight);
    addButton("BenchmarkBruteForce", 4, col2X, gridStartY + cardHeight + gap, cardWidth, cardHeight);
    addButton("CompareAlgorithms", 5, col2X, gridStartY + (cardHeight + gap) * 2, cardWidth, cardHeight);

    float exitBtnWidth = 200.0f;
    float exitBtnHeight = 45.0f;
    addButton("EXIT", 0, width/2 - exitBtnWidth/2, height - 65, exitBtnWidth, exitBtnHeight);
}


void Menu::addButton(const string& key, int id, float x, float y, float btnWidth, float btnHeight) {
    Button btn(font);
    btn.id = id;
    btn.locKey = key;
    
    btn.shape.setSize(Vector2f(btnWidth, btnHeight));
    btn.shape.setPosition({x, y});
    btn.shape.setFillColor(Color(28, 28, 28, 255));
    btn.shape.setOutlineThickness(0);
    
    btn.text.setFont(font);
    btn.text.setString(ru(getButtonText(key)));
    btn.text.setCharacterSize(id == -1 ? 14 : 16);
    btn.text.setFillColor(Color(200, 200, 200));
    btn.text.setStyle(Text::Bold);
    btn.text.setLineSpacing(1.2f);
    
    FloatRect textRect = btn.text.getLocalBounds();
    btn.text.setPosition({x + btnWidth/2 - textRect.size.x/2 - textRect.position.x,
                         y + btnHeight/2 - textRect.size.y/2 - textRect.position.y});
    
    buttons.push_back(btn);
}

void Menu::updateBackground() {
    for (auto& star : stars) {
        star.y += star.speed;
        if (star.y > height) {
            star.y = 0;
            star.x = static_cast<float>(rand() % (int)width);
        }
    }
}

void Menu::drawBackground(RenderWindow& window) {
    // Адаптивная отрисовка для производительности
    size_t render_count = min(stars.size(), size_t(15000));
    size_t step = stars.size() / render_count;
    if (step == 0) step = 1;
    
    for (size_t i = 0; i < stars.size(); i += step) {
        const auto& star = stars[i];
        float size = (stars.size() > 200000) ? 1.0f : star.size;
        CircleShape starShape(size);
        starShape.setPosition({star.x, star.y});
        
        if (stars.size() > 200000) {
            starShape.setFillColor(Color(180, 180, 180, 120));
        } else {
            int b = static_cast<int>(star.brightness);
            starShape.setFillColor(Color(b, b, b, b/2));
        }
        window.draw(starShape);
    }
}

int Menu::run() {
    RenderWindow window(VideoMode({static_cast<unsigned>(width), static_cast<unsigned>(height)}), "N-Body Simulation", 
                           Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    
    bool showProfileMenu = false;
    
    enum class ProfileState { None, EditUsername, ChangePassword, ConfirmDelete };
    ProfileState profileState = ProfileState::None;
    bool enteringOldPass = false;
    
    InputBox profileInput(font);
    profileInput.shape.setSize({300, 40});
    profileInput.shape.setOrigin({150, 20});
    profileInput.shape.setPosition({width/2, height/2 - 20});
    profileInput.shape.setFillColor(Color(40, 40, 40));
    profileInput.text.setCharacterSize(18);
    profileInput.text.setFillColor(Color::White);
    profileInput.text.setPosition({width/2 - 140, height/2 - 32});
    
    InputBox profileInput2(font);
    profileInput2.shape.setSize({300, 40});
    profileInput2.shape.setOrigin({150, 20});
    profileInput2.shape.setPosition({width/2, height/2 + 30});
    profileInput2.shape.setFillColor(Color(40, 40, 40));
    profileInput2.text.setCharacterSize(18);
    profileInput2.text.setFillColor(Color::White);
    profileInput2.text.setPosition({width/2 - 140, height/2 + 18});
    
    Button saveProfileBtn(font);
    saveProfileBtn.shape.setSize({140, 40});
    saveProfileBtn.shape.setPosition({width/2 - 150, height/2 + 80});
    saveProfileBtn.shape.setFillColor(Color(30, 215, 96));
    saveProfileBtn.text.setString(ru(u8"СОХРАНИТЬ"));
    saveProfileBtn.text.setCharacterSize(14);
    saveProfileBtn.text.setFillColor(Color::Black);
    FloatRect saveRect = saveProfileBtn.text.getLocalBounds();
    saveProfileBtn.text.setOrigin({saveRect.position.x + saveRect.size.x/2.0f, saveRect.position.y + saveRect.size.y/2.0f});
    saveProfileBtn.text.setPosition({width/2 - 80, height/2 + 100});
    
    Button cancelProfileBtn(font);
    cancelProfileBtn.shape.setSize({140, 40});
    cancelProfileBtn.shape.setPosition({width/2 + 10, height/2 + 80});
    cancelProfileBtn.shape.setFillColor(Color(200, 50, 50));
    cancelProfileBtn.text.setString(ru(u8"ОТМЕНА"));
    cancelProfileBtn.text.setCharacterSize(14);
    cancelProfileBtn.text.setFillColor(Color::White);
    FloatRect cancelRect = cancelProfileBtn.text.getLocalBounds();
    cancelProfileBtn.text.setOrigin({cancelRect.position.x + cancelRect.size.x/2.0f, cancelRect.position.y + cancelRect.size.y/2.0f});
    cancelProfileBtn.text.setPosition({width/2 + 80, height/2 + 100});

#ifdef ENABLE_AUTH
    AuthManager auth;
    auth.connect();
    bool is_admin = auth.isAdmin(current_username);
    int currentUserId = auth.getUserId(current_username);
#else
    bool is_admin = false;
    int currentUserId = -1;
#endif

    // Кэшируем русские строки
    String titleStr = ru("N-BODY");
    String subtitleStr = ru(u8"СИМУЛЯЦИЯ");
    String taglineStr = ru(u8"Алгоритм Барнса-Хата / Производительность O(N log N)");
    String simLabelStr = ru(u8"СИМУЛЯЦИИ");
    String benchLabelStr = ru(u8"БЕНЧМАРКИ");
    String accountStr = ru(u8"АККАУНТ");
    String changeUserStr = ru(u8"Сменить имя");
    String changePassStr = ru(u8"Сменить пароль");
    String logoutStr = ru(u8"Выйти");
    String deleteAccStr = ru(u8"Удалить аккаунт");
    String adminPanelStr = ru(u8"Админ панель");
    String confirmStr = ru(u8"ПОДТВЕРДИТЬ");
    String saveStr = ru(u8"СОХРАНИТЬ");
    String newUsernameStr = ru(u8"Новое имя");
    String newPasswordStr = ru(u8"Новый пароль");
    String oldPasswordStr = ru(u8"Старый пароль");

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return 0;
            }
            
            if (profileState == ProfileState::EditUsername || profileState == ProfileState::ChangePassword) {
                profileInput.handleInput(*event);
                if (profileState == ProfileState::ChangePassword) {
                    profileInput2.handleInput(*event);
                }
            }
            
            Vector2i mousePos = Mouse::getPosition(window);
            Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    
                    if (profileState != ProfileState::None) {
                         if (saveProfileBtn.shape.getGlobalBounds().contains(mousePosF)) {
                            #ifdef ENABLE_AUTH
                            bool success = false;
                            if (profileState == ProfileState::EditUsername) {
                                if (!profileInput.value.empty()) {
                                    if (auth.updateUsername(currentUserId, profileInput.value)) {
                                        current_username = profileInput.value;
                                        success = true;
                                    }
                                }
                            } else if (profileState == ProfileState::ChangePassword) {
                                if (!profileInput.value.empty() && !profileInput2.value.empty()) {
                                    if (auth.loginUser(current_username, profileInput.value)) {
                                        if (auth.changePassword(current_username, profileInput.value, profileInput2.value)) {
                                            success = true;
                                        }
                                    } else {
                                        profileInput.value = "";
                                        profileInput2.value = "";
                                    }
                                }
                            } else if (profileState == ProfileState::ConfirmDelete) {
                                if (!is_admin) {
                                    if (auth.deleteUser(currentUserId)) {
                                        window.close();
                                        return -1; 
                                    }
                                }
                            }
                            
                            if (success || profileState == ProfileState::ConfirmDelete) {
                                profileState = ProfileState::None;
                                showProfileMenu = false;
                            }
                            #else
                            profileState = ProfileState::None;
                            #endif
                        }
                        if (cancelProfileBtn.shape.getGlobalBounds().contains(mousePosF)) {
                            profileState = ProfileState::None;
                            saveProfileBtn.text.setString(saveStr);
                        }
                        
                        if (profileInput.shape.getGlobalBounds().contains(mousePosF)) {
                            profileInput.isActive = true;
                            profileInput2.isActive = false;
                        } else if (profileState == ProfileState::ChangePassword && profileInput2.shape.getGlobalBounds().contains(mousePosF)) {
                            profileInput.isActive = false;
                            profileInput2.isActive = true;
                        } else {
                            profileInput.isActive = false;
                            profileInput2.isActive = false;
                        }
                        continue;
                    
                    CircleShape profileBtn(24);
                    profileBtn.setPosition({width - 70, 20});
                    if (profileBtn.getGlobalBounds().contains(mousePosF)) {
                        showProfileMenu = !showProfileMenu;
                        continue;
                    }
                    
                    if (showProfileMenu) {
                        float menuX = width - 220;
                        float menuY = 70;
                        float itemHeight = 40;
                        float currentY = menuY + 65;
                        FloatRect userRect({menuX + 10, currentY}, {180, 35});
                        if (userRect.contains(mousePosF)) {
                            profileState = ProfileState::EditUsername;
                            profileInput.value = current_username;
                            profileInput.isPassword = false;
                            profileInput.label.setString(newUsernameStr);
                            profileInput.isActive = true;
                            saveProfileBtn.text.setString(saveStr);
                            continue;
                        }
                        currentY += 40;
                        
                        FloatRect passRect({menuX + 10, currentY}, {180, 35});
                        if (passRect.contains(mousePosF)) {
                            profileState = ProfileState::ChangePassword;
                            profileInput.value = "";
                            profileInput.isPassword = true;
                            profileInput.label.setString(oldPasswordStr);
                            profileInput.isActive = true;
                            
                            profileInput2.value = "";
                            profileInput2.isPassword = true;
                            profileInput2.label.setString(newPasswordStr);
                            profileInput2.isActive = false;
                            
                            saveProfileBtn.text.setString(saveStr);
                            continue;
                        }
                        currentY += 40;
                        
                        FloatRect logoutRect({menuX + 10, currentY}, {180, 35});
                        if (logoutRect.contains(mousePosF)) {
                            window.close();
                            return -1;
                        }
                        currentY += 40;
                        
                        FloatRect deleteRect({menuX + 10, currentY}, {180, 35});
                        if (deleteRect.contains(mousePosF)) {
                            if (!is_admin) {
                                profileState = ProfileState::ConfirmDelete;
                                saveProfileBtn.text.setString(confirmStr);
                            }
                            continue;
                        }
                        currentY += 40;
                        
                        if (is_admin) {
                            FloatRect adminRect({menuX + 10, currentY}, {180, 35});
                            if (adminRect.contains(mousePosF)) {
                                (void)window.setActive(false);
                                showAdminPanel();
                                (void)window.setActive(true);
                                continue;
                            }
                            currentY += 40;
                        }


                    }
                    
                    for (const auto& btn : buttons) {
                        if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                            window.close();
                            return btn.id;
                        }
                    }
                    
                    if (showProfileMenu) {
                        RectangleShape profilePanel({200, 150});
                        profilePanel.setPosition({width - 220, 60});
                        if (mousePosF.x < width - 220 || mousePosF.y > 400) {
                             showProfileMenu = false;
                        }
                    }
                }
            }
        }
        
        updateBackground();
        
        Text title(font);
        title.setFont(font);
        title.setString(titleStr);
        title.setCharacterSize(64);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        title.setLetterSpacing(1.2f);
        FloatRect titleRect = title.getLocalBounds();
        title.setPosition({width/2 - titleRect.size.x/2 - titleRect.position.x, 120});

        Text subtitle(font);
        subtitle.setFont(font);
        subtitle.setString(subtitleStr);
        subtitle.setCharacterSize(24);
        subtitle.setFillColor(Color(30, 215, 96));
        subtitle.setStyle(Text::Bold);
        subtitle.setLetterSpacing(3.0f);
        FloatRect subRect = subtitle.getLocalBounds();
        subtitle.setPosition({width/2 - subRect.size.x/2 - subRect.position.x, 190});
        
        Text tagline(font);
        tagline.setString(taglineStr);
        tagline.setCharacterSize(13);
        tagline.setFillColor(Color(120, 120, 120));
        FloatRect tagRect = tagline.getLocalBounds();
        tagline.setPosition({width/2 - tagRect.size.x/2 - tagRect.position.x, 230});
        
        Text simLabel(font);
        simLabel.setString(simLabelStr);
        simLabel.setCharacterSize(11);
        simLabel.setFillColor(Color(100, 100, 100));
        simLabel.setLetterSpacing(1.5f);
        simLabel.setPosition({width/2 - 390, 280});
        
        Text benchLabel(font);
        benchLabel.setString(benchLabelStr);
        benchLabel.setCharacterSize(11);
        benchLabel.setFillColor(Color(100, 100, 100));
        benchLabel.setLetterSpacing(1.5f);
        benchLabel.setPosition({width/2 + 10, 280});

        profileInput.updateDisplay();
        
        Vector2i mousePos = Mouse::getPosition(window);
        float dt = 0.15f;

        for (auto& btn : buttons) {
            FloatRect tr = btn.text.getLocalBounds();
            Vector2f bp = btn.shape.getPosition();
            Vector2f bs = btn.shape.getSize();
            btn.text.setPosition({bp.x + bs.x/2 - tr.size.x/2 - tr.position.x,
                                 bp.y + bs.y/2 - tr.size.y/2 - tr.position.y});

            bool hovered = btn.shape.getGlobalBounds().contains(Vector2f(static_cast<float>(mousePos.x), 
                                                               static_cast<float>(mousePos.y)));
            
            if (hovered && profileState == ProfileState::None) {
                btn.targetScale = 1.03f;
                btn.shape.setFillColor(Color(38, 38, 38));
                btn.shape.setOutlineColor(Color(30, 215, 96));
                btn.shape.setOutlineThickness(2);
                btn.text.setFillColor(Color::White);
            } else {
                btn.targetScale = 1.0f;
                btn.shape.setFillColor(Color(28, 28, 28));
                btn.shape.setOutlineThickness(0);
                btn.text.setFillColor(Color(200, 200, 200));
            }
            

            btn.currentScale = lerp(btn.currentScale, btn.targetScale, dt);
        }

        window.clear(Color(18, 18, 18));
        drawBackground(window);
        
        window.draw(title);
        
        window.draw(subtitle);
        
        window.draw(tagline);
        
        window.draw(simLabel);
        window.draw(benchLabel);

        for (const auto& btn : buttons) {
            Vector2f origPos = btn.shape.getPosition();
            Vector2f origTextPos = btn.text.getPosition();
            Vector2f size = btn.shape.getSize();
            
            if (btn.currentScale != 1.0f) {
                Vector2f center = {origPos.x + size.x/2, origPos.y + size.y/2};
                Transform t;
                t.translate(center);
                t.scale({btn.currentScale, btn.currentScale});
                t.translate(-center);
                
                window.draw(btn.shape, t);
                window.draw(btn.text, t);
            } else {
                window.draw(btn.shape);
                window.draw(btn.text);
            }
        }

        
        
        CircleShape profileBtn(20);
        profileBtn.setPosition({width - 60, 20});
        profileBtn.setFillColor(Color(30, 215, 96));
        
        Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        if (profileBtn.getGlobalBounds().contains(mousePosF)) {
            profileBtn.setFillColor(Color(40, 255, 120));
        }
        window.draw(profileBtn);
        Text profileIcon(font);
        profileIcon.setString(current_username.empty() ? "U" : string(1, current_username[0]));
        profileIcon.setCharacterSize(18);
        profileIcon.setFillColor(Color::Black);
        profileIcon.setStyle(Text::Bold);
        FloatRect iconBounds = profileIcon.getLocalBounds();
        profileIcon.setPosition({width - 60 + 20 - iconBounds.size.x/2 - iconBounds.position.x, 
                                 20 + 20 - iconBounds.size.y/2 - iconBounds.position.y - 2});
        window.draw(profileIcon);
        
        if (showProfileMenu) {
            float panelHeight = is_admin ? 300.0f : 260.0f;
            RectangleShape profilePanel({200, panelHeight});
            profilePanel.setPosition({width - 220, 60});
            profilePanel.setFillColor(Color(28, 28, 28));
            profilePanel.setOutlineColor(Color(60, 60, 60));
            profilePanel.setOutlineThickness(1);
            window.draw(profilePanel);
            
            Text usernameLabel(font);
            usernameLabel.setString(accountStr);
            usernameLabel.setCharacterSize(10);
            usernameLabel.setFillColor(Color(100, 100, 100));
            usernameLabel.setPosition({width - 210, 70});
            window.draw(usernameLabel);
            
            Text usernameText(font);
            usernameText.setString(current_username);
            usernameText.setCharacterSize(16);
            usernameText.setFillColor(Color::White);
            usernameText.setStyle(Text::Bold);
            usernameText.setPosition({width - 210, 90});
            window.draw(usernameText);
            
            RectangleShape divider({180, 1});
            divider.setPosition({width - 210, 125});
            divider.setFillColor(Color(60, 60, 60));
            window.draw(divider);
            
            float currentY = 135;
            
            auto drawMenuBtn = [&](const String& text, float y) {
                RectangleShape btn({180, 35});
                btn.setPosition({width - 210, y});
                if (btn.getGlobalBounds().contains(mousePosF)) {
                    btn.setFillColor(Color(80, 80, 80));
                } else {
                    btn.setFillColor(Color(50, 50, 50));
                }
                window.draw(btn);
                
                Text t(font);
                t.setString(text);
                t.setCharacterSize(14);
                t.setFillColor(Color::White);
                FloatRect r = t.getLocalBounds();
                t.setOrigin({r.position.x, r.position.y + r.size.y/2});
                t.setPosition({width - 200, y + 17.5f});
                window.draw(t);
            };
            
            drawMenuBtn(changeUserStr, currentY); currentY += 40;
            drawMenuBtn(changePassStr, currentY); currentY += 40;
            drawMenuBtn(logoutStr, currentY); currentY += 40;
            drawMenuBtn(deleteAccStr, currentY); currentY += 40;
            
            if (is_admin) {
                drawMenuBtn(adminPanelStr, currentY); currentY += 40;
            }
            

        }
        
        if (profileState != ProfileState::None) {
            RectangleShape dim({static_cast<float>(width), static_cast<float>(height)});
            dim.setFillColor(Color(0, 0, 0, 180));
            window.draw(dim);
            
            RectangleShape modal({400, 200});
            modal.setOrigin({200, 100});
            modal.setPosition({width/2, height/2});
            modal.setFillColor(Color(30, 30, 30));
            modal.setOutlineColor(Color(60, 60, 60));
            modal.setOutlineThickness(2);
            window.draw(modal);
            
            Text modalTitle(font);
            if (profileState == ProfileState::EditUsername) modalTitle.setString(ru(u8"Сменить имя"));
            else if (profileState == ProfileState::ChangePassword) modalTitle.setString(ru(u8"Сменить пароль"));
            else if (profileState == ProfileState::ConfirmDelete) modalTitle.setString(ru(u8"Удалить аккаунт?"));
            
            modalTitle.setCharacterSize(20);
            modalTitle.setFillColor(Color::White);
            modalTitle.setStyle(Text::Bold);
            FloatRect titleRect = modalTitle.getLocalBounds();
            modalTitle.setOrigin({titleRect.position.x + titleRect.size.x/2, titleRect.position.y + titleRect.size.y/2});
            modalTitle.setPosition({width/2, height/2 - 60});
            window.draw(modalTitle);
            
            if (profileState == ProfileState::ConfirmDelete) {
                Text warnText(font);
                warnText.setString(ru(u8"Это действие необратимо."));
                warnText.setCharacterSize(14);
                warnText.setFillColor(Color(200, 100, 100));
                FloatRect warnRect = warnText.getLocalBounds();
                warnText.setOrigin({warnRect.position.x + warnRect.size.x/2, warnRect.position.y + warnRect.size.y/2});
                warnText.setPosition({width/2, height/2});
                window.draw(warnText);
            } else {
                window.draw(profileInput.label);
                window.draw(profileInput.shape);
                
                // Mask password
                string displayVal = profileInput.value;
                if (profileInput.isPassword) {
                    displayVal = string(displayVal.length(), '*');
                }
                profileInput.text.setString(displayVal + (profileInput.isActive ? "|" : ""));
                window.draw(profileInput.text);
                
                if (profileInput.isActive) {
                    RectangleShape focusBorder = profileInput.shape;
                    focusBorder.setFillColor(Color::Transparent);
                    focusBorder.setOutlineColor(Color(30, 215, 96));
                    focusBorder.setOutlineThickness(2);
                    window.draw(focusBorder);
                }

                if (profileState == ProfileState::ChangePassword) {
                    window.draw(profileInput2.label);
                    window.draw(profileInput2.shape);
                    
                    // Mask password
                    string displayVal2 = profileInput2.value;
                    if (profileInput2.isPassword) {
                        displayVal2 = string(displayVal2.length(), '*');
                    }
                    profileInput2.text.setString(displayVal2 + (profileInput2.isActive ? "|" : ""));
                    window.draw(profileInput2.text);

                    if (profileInput2.isActive) {
                        RectangleShape focusBorder = profileInput2.shape;
                        focusBorder.setFillColor(Color::Transparent);
                        focusBorder.setOutlineColor(Color(30, 215, 96));
                        focusBorder.setOutlineThickness(2);
                        window.draw(focusBorder);
                    }
                }
            }
            
            window.draw(saveProfileBtn.shape);
            window.draw(saveProfileBtn.text);
            window.draw(cancelProfileBtn.shape);
            window.draw(cancelProfileBtn.text);
        }

        window.display();
    }
    return 0;
}

int Menu::selectParticleCount() {
    RenderWindow window(VideoMode({600, 400}), ru(u8"Выберите кол-во частиц"), 
                           Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    
    int minParticles = 100;
    int maxParticles = 500000;
    int currentParticles = 1000;
    
    float sliderX = 100.0f;
    float sliderY = 200.0f;
    float sliderWidth = 400.0f;
    float sliderHeight = 10.0f;
    
    RectangleShape sliderBar({sliderWidth, sliderHeight});
    sliderBar.setPosition({sliderX, sliderY});
    sliderBar.setFillColor(Color(60, 60, 60));
    sliderBar.setOrigin({0, sliderHeight/2});
    
    CircleShape sliderHandle(15.0f);
    sliderHandle.setOrigin({15.0f, 15.0f});
    sliderHandle.setFillColor(Color(30, 215, 96));
    
    float t = static_cast<float>(currentParticles - minParticles) / (maxParticles - minParticles);
    sliderHandle.setPosition({sliderX + t * sliderWidth, sliderY});
    
    Button confirmBtn(font);
    confirmBtn.shape.setSize({200, 45});
    confirmBtn.shape.setOrigin({100, 22.5f});
    confirmBtn.shape.setPosition({300, 300});
    confirmBtn.shape.setFillColor(Color(30, 215, 96));
    confirmBtn.text.setString(ru(u8"СТАРТ"));
    confirmBtn.text.setCharacterSize(16);
    confirmBtn.text.setFillColor(Color::Black);
    confirmBtn.text.setStyle(Text::Bold);
    FloatRect btnRect = confirmBtn.text.getLocalBounds();
    confirmBtn.text.setOrigin({btnRect.position.x + btnRect.size.x/2.0f, btnRect.position.y + btnRect.size.y/2.0f});
    confirmBtn.text.setPosition({300, 300});
    
    bool isDragging = false;
    String titleStr = ru(u8"Количество частиц");
    
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return 0; 
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    
                    FloatRect handleBounds = sliderHandle.getGlobalBounds();
                                        handleBounds.position.x -= 5; handleBounds.position.y -= 5;
                    handleBounds.size.x += 10; handleBounds.size.y += 10;
                    
                    if (handleBounds.contains(mousePos)) {
                        isDragging = true;
                    }
                    
                    else if (sliderBar.getGlobalBounds().contains(mousePos)) {
                        isDragging = true;
                        
                        float newX = max(sliderX, min(mousePos.x, sliderX + sliderWidth));
                        sliderHandle.setPosition({newX, sliderY});
                        float ratio = (newX - sliderX) / sliderWidth;
                        currentParticles = minParticles + static_cast<int>(ratio * (maxParticles - minParticles));
                        
                        currentParticles = (currentParticles / 100) * 100;
                    }
                    
                    if (confirmBtn.shape.getGlobalBounds().contains(mousePos)) {
                        window.close();
                        return currentParticles;
                    }
                }
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    isDragging = false;
                }
            }
            
            if (event->is<Event::MouseMoved>()) {
                if (isDragging) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    float newX = max(sliderX, min(mousePos.x, sliderX + sliderWidth));
                    sliderHandle.setPosition({newX, sliderY});
                    
                    float ratio = (newX - sliderX) / sliderWidth;
                    currentParticles = minParticles + static_cast<int>(ratio * (maxParticles - minParticles));
                    currentParticles = (currentParticles / 100) * 100;
                }
            }
        }
        
        window.clear(Color(18, 18, 18));
        Text title(font);
        title.setString(titleStr);
        title.setCharacterSize(28);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        FloatRect titleRect = title.getLocalBounds();
        title.setOrigin({titleRect.position.x + titleRect.size.x/2.0f, titleRect.position.y + titleRect.size.y/2.0f});
        title.setPosition({300, 60});
        window.draw(title);
        Text valueText(font);
        valueText.setString(to_string(currentParticles));
        valueText.setCharacterSize(48);
        valueText.setFillColor(Color(30, 215, 96));
        valueText.setStyle(Text::Bold);
        FloatRect valRect = valueText.getLocalBounds();
        valueText.setOrigin({valRect.position.x + valRect.size.x/2.0f, valRect.position.y + valRect.size.y/2.0f});
        valueText.setPosition({300, 130});
        window.draw(valueText);
        
        window.draw(sliderBar);
        window.draw(sliderHandle);
        
        Text minText(font);
        minText.setString(to_string(minParticles));
        minText.setCharacterSize(14);
        minText.setFillColor(Color(150, 150, 150));
        minText.setPosition({sliderX, sliderY + 20});
        window.draw(minText);
        
        Text maxText(font);
        maxText.setString(to_string(maxParticles));
        maxText.setCharacterSize(14);
        maxText.setFillColor(Color(150, 150, 150));
        FloatRect maxRect = maxText.getLocalBounds();
        maxText.setPosition({sliderX + sliderWidth - maxRect.size.x, sliderY + 20});
        window.draw(maxText);
        
        window.draw(confirmBtn.shape);
        window.draw(confirmBtn.text);
        
        window.display();
    }
    
    return currentParticles;
}

#ifdef ENABLE_AUTH
bool Menu::showLoginScreen() {
    RenderWindow window(VideoMode({static_cast<unsigned>(width), static_cast<unsigned>(height)}), "Login - N-Body Simulation", 
                           Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);

    AuthManager auth;
    if (!auth.connect()) {
        cerr << "Failed to connect to database" << endl;
        return false;
    }

    InputBox userBox(font);
    userBox.shape.setSize({300, 40});
    userBox.shape.setOrigin({150, 20});
    userBox.shape.setPosition({width/2, 250});
    userBox.shape.setFillColor(Color(40, 40, 40));
    userBox.text.setCharacterSize(18);
    userBox.text.setFillColor(Color::White);
    userBox.text.setPosition({width/2 - 140, 238});
    userBox.label.setString(ru(u8"Имя пользователя"));
    userBox.label.setCharacterSize(14);
    userBox.label.setFillColor(Color(179, 179, 179));
    userBox.label.setPosition({width/2 - 150, 210});

    InputBox passBox(font);
    passBox.shape.setSize({300, 40});
    passBox.shape.setOrigin({150, 20});
    passBox.shape.setPosition({width/2, 330});
    passBox.shape.setFillColor(Color(40, 40, 40));
    passBox.text.setCharacterSize(18);
    passBox.text.setFillColor(Color::White);
    passBox.text.setPosition({width/2 - 140, 318});
    passBox.isPassword = true;
    passBox.label.setString(ru(u8"Пароль"));
    passBox.label.setCharacterSize(14);
    passBox.label.setFillColor(Color(179, 179, 179));
    passBox.label.setPosition({width/2 - 150, 290});

    Button loginBtn(font);
    loginBtn.shape.setSize({300, 45});
    loginBtn.shape.setOrigin({150, 22.5f});
    loginBtn.shape.setPosition({width/2, 410});
    loginBtn.shape.setFillColor(Color(30, 215, 96));
    loginBtn.text.setString(ru(u8"ВОЙТИ"));
    loginBtn.text.setCharacterSize(16);
    loginBtn.text.setFillColor(Color::Black);
    loginBtn.text.setStyle(Text::Bold);
    FloatRect loginRect = loginBtn.text.getLocalBounds();
    loginBtn.text.setOrigin({loginRect.position.x + loginRect.size.x/2.0f, loginRect.position.y + loginRect.size.y/2.0f});
    loginBtn.text.setPosition({width/2, 410});

    Button regBtn(font);
    regBtn.shape.setSize({300, 45});
    regBtn.shape.setOrigin({150, 22.5f});
    regBtn.shape.setPosition({width/2, 470});
    regBtn.shape.setFillColor(Color::Transparent);
    regBtn.shape.setOutlineThickness(1);
    regBtn.shape.setOutlineColor(Color(179, 179, 179));
    regBtn.text.setString(ru(u8"СОЗДАТЬ АККАУНТ"));
    regBtn.text.setCharacterSize(16);
    regBtn.text.setFillColor(Color::White);
    regBtn.text.setStyle(Text::Bold);
    FloatRect regRect = regBtn.text.getLocalBounds();
    regBtn.text.setOrigin({regRect.position.x + regRect.size.x/2.0f, regRect.position.y + regRect.size.y/2.0f});
    regBtn.text.setPosition({width/2, 470});

    string statusMsg = "";
    Text statusText(font);
    statusText.setCharacterSize(14);
    statusText.setFillColor(Color(255, 80, 80));
    statusText.setPosition({width/2, 370});

    String titleStr = ru(u8"N-BODY СИМУЛЯЦИЯ");

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return false;
            }

            userBox.handleInput(*event);
            passBox.handleInput(*event);

            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Tab) {
                    if (userBox.isActive) {
                        userBox.isActive = false;
                        passBox.isActive = true;
                    } else {
                        userBox.isActive = true;
                        passBox.isActive = false;
                    }
                    // Update visuals immediately
                    userBox.shape.setOutlineThickness(userBox.isActive ? 2 : 0);
                    userBox.shape.setOutlineColor(Color::White);
                    passBox.shape.setOutlineThickness(passBox.isActive ? 2 : 0);
                    passBox.shape.setOutlineColor(Color::White);
                } else if (key->code == Keyboard::Key::Enter) {
                    if (auth.loginUser(userBox.value, passBox.value)) {
                        current_username = userBox.value;
                        window.close();
                        return true;
                    } else {
                        statusMsg = u8"Неверное имя или пароль";
                    }
                }
            }

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    userBox.isActive = userBox.shape.getGlobalBounds().contains(mousePos);
                    passBox.isActive = passBox.shape.getGlobalBounds().contains(mousePos);
                    
                    userBox.shape.setOutlineThickness(userBox.isActive ? 2 : 0);
                    userBox.shape.setOutlineColor(Color::White);
                    passBox.shape.setOutlineThickness(passBox.isActive ? 2 : 0);
                    passBox.shape.setOutlineColor(Color::White);

                    if (loginBtn.shape.getGlobalBounds().contains(mousePos)) {
                        if (auth.loginUser(userBox.value, passBox.value)) {
                            current_username = userBox.value;
                            window.close();
                            return true;
                        } else {
                            statusMsg = u8"Неверное имя или пароль";
                        }
                    }

                    if (regBtn.shape.getGlobalBounds().contains(mousePos)) {
                        window.close();
                        return showRegisterScreen();
                    }
                }
            }
        }

        userBox.updateDisplay();
        passBox.updateDisplay();

        statusText.setString(String::fromUtf8(statusMsg.begin(), statusMsg.end()));
        FloatRect statusRect = statusText.getLocalBounds();
        statusText.setOrigin({statusRect.position.x + statusRect.size.x/2.0f, statusRect.position.y + statusRect.size.y/2.0f});
        statusText.setPosition({width/2, 375});

        window.clear(Color(18, 18, 18));
        drawBackground(window);

        Text title(font);
        title.setString(titleStr);
        title.setCharacterSize(32);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        FloatRect titleRect = title.getLocalBounds();
        title.setOrigin({titleRect.position.x + titleRect.size.x/2.0f, titleRect.position.y + titleRect.size.y/2.0f});
        title.setPosition({width/2, 100});
        window.draw(title);

        window.draw(userBox.label);
        window.draw(userBox.shape);
        window.draw(userBox.text);

        window.draw(passBox.label);
        window.draw(passBox.shape);
        window.draw(passBox.text);

        window.draw(loginBtn.shape);
        window.draw(loginBtn.text);

        window.draw(regBtn.shape);
        window.draw(regBtn.text);
        
        window.draw(statusText);

        window.display();
    }
    return false;
}

bool Menu::showRegisterScreen() {
    RenderWindow window(VideoMode({static_cast<unsigned>(width), static_cast<unsigned>(height)}), "Register - N-Body Simulation", 
                           Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);

    AuthManager auth;
    if (!auth.connect()) return false;

    InputBox userBox(font);
    userBox.shape.setSize({300, 40});
    userBox.shape.setOrigin({150, 20});
    userBox.shape.setPosition({width/2, 250});
    userBox.shape.setFillColor(Color(40, 40, 40));
    userBox.text.setCharacterSize(18);
    userBox.text.setFillColor(Color::White);
    userBox.text.setPosition({width/2 - 140, 238});
    userBox.label.setString(ru(u8"Выберите имя"));
    userBox.label.setCharacterSize(14);
    userBox.label.setFillColor(Color(179, 179, 179));
    userBox.label.setPosition({width/2 - 150, 210});

    InputBox passBox(font);
    passBox.shape.setSize({300, 40});
    passBox.shape.setOrigin({150, 20});
    passBox.shape.setPosition({width/2, 330});
    passBox.shape.setFillColor(Color(40, 40, 40));
    passBox.text.setCharacterSize(18);
    passBox.text.setFillColor(Color::White);
    passBox.text.setPosition({width/2 - 140, 318});
    passBox.isPassword = true;
    passBox.label.setString(ru(u8"Выберите пароль"));
    passBox.label.setCharacterSize(14);
    passBox.label.setFillColor(Color(179, 179, 179));
    passBox.label.setPosition({width/2 - 150, 290});

    Button regBtn(font);
    regBtn.shape.setSize({300, 45});
    regBtn.shape.setOrigin({150, 22.5f});
    regBtn.shape.setPosition({width/2, 410});
    regBtn.shape.setFillColor(Color(30, 215, 96));
    regBtn.text.setString(ru(u8"РЕГИСТРАЦИЯ"));
    regBtn.text.setCharacterSize(16);
    regBtn.text.setFillColor(Color::Black);
    regBtn.text.setStyle(Text::Bold);
    FloatRect regRect = regBtn.text.getLocalBounds();
    regBtn.text.setOrigin({regRect.position.x + regRect.size.x/2.0f, regRect.position.y + regRect.size.y/2.0f});
    regBtn.text.setPosition({width/2, 410});

    Button backBtn(font);
    backBtn.shape.setSize({300, 45});
    backBtn.shape.setOrigin({150, 22.5f});
    backBtn.shape.setPosition({width/2, 470});
    backBtn.shape.setFillColor(Color::Transparent);
    backBtn.shape.setOutlineThickness(1);
    backBtn.shape.setOutlineColor(Color(179, 179, 179));
    backBtn.text.setString(ru(u8"НАЗАД"));
    backBtn.text.setCharacterSize(16);
    backBtn.text.setFillColor(Color::White);
    backBtn.text.setStyle(Text::Bold);
    FloatRect backRect = backBtn.text.getLocalBounds();
    backBtn.text.setOrigin({backRect.position.x + backRect.size.x/2.0f, backRect.position.y + backRect.size.y/2.0f});
    backBtn.text.setPosition({width/2, 470});

    string statusMsg = "";
    Text statusText(font);
    statusText.setCharacterSize(14);
    statusText.setFillColor(Color(255, 80, 80));
    statusText.setPosition({width/2, 370});

    String titleStr = ru(u8"СОЗДАТЬ АККАУНТ");

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return false;
            }

            userBox.handleInput(*event);
            passBox.handleInput(*event);

            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Tab) {
                    if (userBox.isActive) {
                        userBox.isActive = false;
                        passBox.isActive = true;
                    } else {
                        userBox.isActive = true;
                        passBox.isActive = false;
                    }
                    // Update visuals immediately
                    userBox.shape.setOutlineThickness(userBox.isActive ? 2 : 0);
                    userBox.shape.setOutlineColor(Color::White);
                    passBox.shape.setOutlineThickness(passBox.isActive ? 2 : 0);
                    passBox.shape.setOutlineColor(Color::White);
                } else if (key->code == Keyboard::Key::Enter) {
                    if (userBox.value.length() < 3 || passBox.value.length() < 3) {
                        statusMsg = u8"Имя/Пароль слишком короткие";
                    } else if (auth.registerUser(userBox.value, passBox.value)) {
                        window.close();
                        return showLoginScreen();
                    } else {
                        statusMsg = u8"Имя уже занято";
                    }
                }
            }

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    userBox.isActive = userBox.shape.getGlobalBounds().contains(mousePos);
                    passBox.isActive = passBox.shape.getGlobalBounds().contains(mousePos);

                    userBox.shape.setOutlineThickness(userBox.isActive ? 2 : 0);
                    userBox.shape.setOutlineColor(Color::White);
                    passBox.shape.setOutlineThickness(passBox.isActive ? 2 : 0);
                    passBox.shape.setOutlineColor(Color::White);

                    if (regBtn.shape.getGlobalBounds().contains(mousePos)) {
                        if (userBox.value.length() < 3 || passBox.value.length() < 3) {
                            statusMsg = u8"Имя/Пароль слишком короткие";
                        } else if (auth.registerUser(userBox.value, passBox.value)) {
                            window.close();
                            return showLoginScreen();
                        } else {
                            statusMsg = u8"Имя уже занято";
                        }
                    }

                    if (backBtn.shape.getGlobalBounds().contains(mousePos)) {
                        window.close();
                        return showLoginScreen();
                    }
                }
            }
        }

        userBox.updateDisplay();
        passBox.updateDisplay();

        statusText.setString(String::fromUtf8(statusMsg.begin(), statusMsg.end()));
        FloatRect statusRect = statusText.getLocalBounds();
        statusText.setOrigin({statusRect.position.x + statusRect.size.x/2.0f, statusRect.position.y + statusRect.size.y/2.0f});
        statusText.setPosition({width/2, 375});

        window.clear(Color(18, 18, 18));
        drawBackground(window);

        Text title(font);
        title.setString(titleStr);
        title.setCharacterSize(32);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        FloatRect titleRect = title.getLocalBounds();
        title.setOrigin({titleRect.position.x + titleRect.size.x/2.0f, titleRect.position.y + titleRect.size.y/2.0f});
        title.setPosition({width/2, 100});
        window.draw(title);

        window.draw(userBox.label);
        window.draw(userBox.shape);
        window.draw(userBox.text);

        window.draw(passBox.label);
        window.draw(passBox.shape);
        window.draw(passBox.text);

        window.draw(regBtn.shape);
        window.draw(regBtn.text);

        window.draw(backBtn.shape);
        window.draw(backBtn.text);
        
        window.draw(statusText);

        window.display();
    }
    return false;
}
#endif

double Menu::selectTheta() {
    RenderWindow window(VideoMode({600, 400}), "Select Theta", 
                           Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    
    double minTheta = 0.1;
    double maxTheta = 2.0;
    double currentTheta = 0.5;
    
    float sliderX = 100.0f;
    float sliderY = 200.0f;
    float sliderWidth = 400.0f;
    float sliderHeight = 10.0f;
    
    RectangleShape sliderBar({sliderWidth, sliderHeight});
    sliderBar.setPosition({sliderX, sliderY});
    sliderBar.setFillColor(Color(60, 60, 60));
    sliderBar.setOrigin({0, sliderHeight/2});
    
    CircleShape sliderHandle(15.0f);
    sliderHandle.setOrigin({15.0f, 15.0f});
    sliderHandle.setFillColor(Color(30, 215, 96));
    
    float t = static_cast<float>((currentTheta - minTheta) / (maxTheta - minTheta));
    sliderHandle.setPosition({sliderX + t * sliderWidth, sliderY});
    
    Button confirmBtn(font);
    confirmBtn.shape.setSize({200, 45});
    confirmBtn.shape.setOrigin({100, 22.5f});
    confirmBtn.shape.setPosition({300, 300});
    confirmBtn.shape.setFillColor(Color(30, 215, 96));
    confirmBtn.text.setString(ru(u8"СТАРТ"));
    confirmBtn.text.setCharacterSize(16);
    confirmBtn.text.setFillColor(Color::Black);
    confirmBtn.text.setStyle(Text::Bold);
    FloatRect btnRect = confirmBtn.text.getLocalBounds();
    confirmBtn.text.setOrigin({btnRect.position.x + btnRect.size.x/2.0f, btnRect.position.y + btnRect.size.y/2.0f});
    confirmBtn.text.setPosition({300, 300});
    
    bool isDragging = false;
    String titleStr = ru(u8"Барнс-Хат Тета");
    
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return 0.5;
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    FloatRect handleBounds = sliderHandle.getGlobalBounds();
                    handleBounds.position.x -= 5; handleBounds.position.y -= 5;
                    handleBounds.size.x += 10; handleBounds.size.y += 10;
                    
                    if (handleBounds.contains(mousePos)) {
                        isDragging = true;
                    }
                    else if (sliderBar.getGlobalBounds().contains(mousePos)) {
                        isDragging = true;
                        float newX = max(sliderX, min(mousePos.x, sliderX + sliderWidth));
                        sliderHandle.setPosition({newX, sliderY});
                        float ratio = (newX - sliderX) / sliderWidth;
                        currentTheta = minTheta + ratio * (maxTheta - minTheta);
                        currentTheta = round(currentTheta * 10.0) / 10.0;
                    }
                    
                    if (confirmBtn.shape.getGlobalBounds().contains(mousePos)) {
                        window.close();
                        return currentTheta;
                    }
                }
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    isDragging = false;
                }
            }
            
            if (event->is<Event::MouseMoved>()) {
                if (isDragging) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    float newX = max(sliderX, min(mousePos.x, sliderX + sliderWidth));
                    sliderHandle.setPosition({newX, sliderY});
                    
                    float ratio = (newX - sliderX) / sliderWidth;
                    currentTheta = minTheta + ratio * (maxTheta - minTheta);
                    currentTheta = round(currentTheta * 10.0) / 10.0;
                }
            }
        }
        
        window.clear(Color(18, 18, 18));
        
        Text title(font);
        title.setString(titleStr);
        title.setCharacterSize(28);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        FloatRect titleRect = title.getLocalBounds();
        title.setOrigin({titleRect.position.x + titleRect.size.x/2.0f, titleRect.position.y + titleRect.size.y/2.0f});
        title.setPosition({300, 60});
        window.draw(title);
        
        Text valueText(font);
        stringstream ss;
        ss << fixed << setprecision(1) << currentTheta;
        valueText.setString(ss.str());
        valueText.setCharacterSize(48);
        valueText.setFillColor(Color(30, 215, 96));
        valueText.setStyle(Text::Bold);
        FloatRect valRect = valueText.getLocalBounds();
        valueText.setOrigin({valRect.position.x + valRect.size.x/2.0f, valRect.position.y + valRect.size.y/2.0f});
        valueText.setPosition({300, 130});
        window.draw(valueText);
        
        window.draw(sliderBar);
        window.draw(sliderHandle);
        
        Text minText(font);
        ss.str("");
        ss << fixed << setprecision(1) << minTheta;
        minText.setString(ss.str());
        minText.setCharacterSize(14);
        minText.setFillColor(Color(150, 150, 150));
        minText.setPosition({sliderX, sliderY + 20});
        window.draw(minText);
        
        Text maxText(font);
        ss.str("");
        ss << fixed << setprecision(1) << maxTheta;
        maxText.setString(ss.str());
        maxText.setCharacterSize(14);
        maxText.setFillColor(Color(150, 150, 150));
        FloatRect maxRect = maxText.getLocalBounds();
        maxText.setPosition({sliderX + sliderWidth - maxRect.size.x, sliderY + 20});
        window.draw(maxText);
        
        window.draw(confirmBtn.shape);
        window.draw(confirmBtn.text);
        
        window.display();
    }
    
    return currentTheta;
}

void Menu::showSettings() {
    RenderWindow window(VideoMode({600, 500}), ru(u8"НАСТРОЙКИ"), Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    
    string dbHost = getenv("DB_HOST") ? getenv("DB_HOST") : "127.0.0.1";
    string dbPort = getenv("DB_PORT") ? getenv("DB_PORT") : "5432";
    string dbName = getenv("DB_NAME") ? getenv("DB_NAME") : "nbody_db";
    string dbUser = getenv("DB_USER") ? getenv("DB_USER") : "nbody_user";
    string dbPass = getenv("DB_PASSWORD") ? getenv("DB_PASSWORD") : "nbody_password";
    
    auto createInput = [&](float y, const string& label, const string& val) {
        InputBox box(font);
        box.shape.setSize({300, 35});
        box.shape.setOrigin({150, 17.5f});
        box.shape.setPosition({300, y});
        box.shape.setFillColor(Color(40, 40, 40));
        box.text.setCharacterSize(16);
        box.text.setFillColor(Color::White);
        box.text.setPosition({160, y - 10});
        box.label.setString(ru(label));
        box.label.setCharacterSize(14);
        box.label.setFillColor(Color(150, 150, 150));
        box.label.setPosition({150, y - 40});
        box.value = val;
        return box;
    };
    
    InputBox hostBox = createInput(80, "DB Host", dbHost);
    InputBox portBox = createInput(150, "DB Port", dbPort);
    InputBox nameBox = createInput(220, "DB Name", dbName);
    InputBox userBox = createInput(290, "DB User", dbUser);
    InputBox passBox = createInput(360, "DB Password", dbPass);
    passBox.isPassword = true;
    
    Button saveBtn(font);
    saveBtn.shape.setSize({140, 40});
    saveBtn.shape.setPosition({220, 420});
    saveBtn.shape.setFillColor(Color(30, 215, 96));
    saveBtn.text.setString(ru(u8"СОХРАНИТЬ"));
    saveBtn.text.setCharacterSize(14);
    saveBtn.text.setFillColor(Color::Black);
    FloatRect saveRect = saveBtn.text.getLocalBounds();
    saveBtn.text.setOrigin({saveRect.position.x + saveRect.size.x/2.0f, saveRect.position.y + saveRect.size.y/2.0f});
    saveBtn.text.setPosition({290, 440});
    
    vector<InputBox*> boxes = {&hostBox, &portBox, &nameBox, &userBox, &passBox};
    
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }
            
            for (auto* box : boxes) {
                box->handleInput(*event);
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    for (auto* box : boxes) {
                        if (box->shape.getGlobalBounds().contains(mousePos)) {
                            box->isActive = true;
                        } else {
                            box->isActive = false;
                        }
                    }
                    
                    if (saveBtn.shape.getGlobalBounds().contains(mousePos)) {
                        setenv("DB_HOST", hostBox.value.c_str(), 1);
                        setenv("DB_PORT", portBox.value.c_str(), 1);
                        setenv("DB_NAME", nameBox.value.c_str(), 1);
                        setenv("DB_USER", userBox.value.c_str(), 1);
                        setenv("DB_PASSWORD", passBox.value.c_str(), 1);
                        window.close();
                    }
                }
            }
        }
        
        window.clear(Color(18, 18, 18));
        
        for (auto* box : boxes) {
            box->updateDisplay();
            window.draw(box->label);
            window.draw(box->shape);
            window.draw(box->text);
            
            if (box->isActive) {
                RectangleShape border = box->shape;
                border.setFillColor(Color::Transparent);
                border.setOutlineColor(Color(30, 215, 96));
                border.setOutlineThickness(1);
                window.draw(border);
            }
        }
        
        window.draw(saveBtn.shape);
        window.draw(saveBtn.text);
        
        window.display();
    }
}

void Menu::showAdminPanel() {
#ifdef ENABLE_AUTH
    RenderWindow window(VideoMode({900, 700}), ru(u8"Админ панель"), Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    
    AuthManager auth;
    auth.connect();
    auto allUsers = auth.getAllUsers();
    auto displayUsers = allUsers;
    
    float scrollY = 0;
    
    enum class State { List };
    State currentState = State::List;
    int selectedUserId = -1;
    string selectedUsername = "";
    String selectedHash = "";
    
    InputBox searchBox(font);
    searchBox.shape.setSize({300, 35});
    searchBox.shape.setPosition({110, 60});
    searchBox.shape.setFillColor(Color(40, 40, 40));
    searchBox.text.setCharacterSize(16);
    searchBox.text.setFillColor(Color::White);
    searchBox.text.setPosition({120, 68});
    searchBox.label.setString(ru(u8"Поиск по имени..."));
    searchBox.label.setCharacterSize(14);
    searchBox.label.setFillColor(Color(150, 150, 150));
    searchBox.label.setPosition({120, 68});
    
    InputBox editBox(font);
    editBox.shape.setSize({300, 40});
    editBox.shape.setOrigin({150, 20});
    editBox.shape.setPosition({450, 350});
    editBox.shape.setFillColor(Color(40, 40, 40));
    editBox.text.setCharacterSize(18);
    editBox.text.setFillColor(Color::White);
    editBox.text.setPosition({310, 338});
    
    Button saveBtn(font);
    saveBtn.shape.setSize({140, 40});
    saveBtn.shape.setPosition({370, 410});
    saveBtn.shape.setFillColor(Color(30, 215, 96));
    saveBtn.text.setString(ru(u8"СОХРАНИТЬ"));
    saveBtn.text.setCharacterSize(14);
    saveBtn.text.setFillColor(Color::Black);
    FloatRect saveRect = saveBtn.text.getLocalBounds();
    saveBtn.text.setOrigin({saveRect.position.x + saveRect.size.x/2.0f, saveRect.position.y + saveRect.size.y/2.0f});
    saveBtn.text.setPosition({440, 430});
    
    Button cancelBtn(font);
    cancelBtn.shape.setSize({140, 40});
    cancelBtn.shape.setPosition({530, 410});
    cancelBtn.shape.setFillColor(Color(200, 50, 50));
    cancelBtn.text.setString(ru(u8"ОТМЕНА"));
    cancelBtn.text.setCharacterSize(14);
    cancelBtn.text.setFillColor(Color::White);
    FloatRect cancelRect = cancelBtn.text.getLocalBounds();
    cancelBtn.text.setOrigin({cancelRect.position.x + cancelRect.size.x/2.0f, cancelRect.position.y + cancelRect.size.y/2.0f});
    cancelBtn.text.setPosition({600, 430});
    
    Button backBtn(font);
    backBtn.shape.setSize({80, 35});
    backBtn.shape.setPosition({20, 60});
    backBtn.shape.setFillColor(Color(60, 60, 60));
    backBtn.text.setString(ru(u8"НАЗАД"));
    backBtn.text.setCharacterSize(14);
    backBtn.text.setFillColor(Color::White);
    FloatRect backRect = backBtn.text.getLocalBounds();
    backBtn.text.setOrigin({backRect.position.x + backRect.size.x/2.0f, backRect.position.y + backRect.size.y/2.0f});
    backBtn.text.setPosition({60, 77});

    String userMgmtStr = ru(u8"УПРАВЛЕНИЕ ПОЛЬЗОВАТЕЛЯМИ");
    String idStr = ru("ID");
    String usernameStr = ru(u8"ИМЯ");
    String roleStr = ru(u8"РОЛЬ");
    String createdStr = ru(u8"СОЗДАН");
    String actionsStr = ru(u8"ДЕЙСТВИЯ");
    String editUserStr = ru(u8"Изменить имя для ");
    String changePassStr = ru(u8"Сменить пароль для ");
    String deleteUserStr = ru(u8"Удалить ");
    String passForStr = ru(u8"Пароль для ");
    String closeHintStr = ru(u8"Нажмите везде, чтобы закрыть");
    String cannotUndoneStr = ru(u8"Это действие необратимо.");

    while (window.isOpen()) {
        Vector2i mousePos = Mouse::getPosition(window);
        Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        float totalHeight = displayUsers.size() * 50.0f + 120.0f;

        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }
            
            if (currentState == State::List) {
                searchBox.handleInput(*event);
                if (event->is<Event::TextEntered>()) {
                    string query = searchBox.value;
                    displayUsers.clear();
                    for (const auto& user : allUsers) {
                        if (user.username.find(query) != string::npos) {
                            displayUsers.push_back(user);
                        }
                    }
                }
            } else {
                editBox.handleInput(*event);
            }
            
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    if (currentState == State::List) {
                        if (backBtn.shape.getGlobalBounds().contains(mousePosF)) {
                            window.close();
                            return;
                        }
                        
                        if (searchBox.shape.getGlobalBounds().contains(mousePosF)) {
                            searchBox.isActive = true;
                        } else {
                            searchBox.isActive = false;
                        }
                        
                        float y = 120 + scrollY;
                        for (const auto& user : displayUsers) {
                            if (y > 100 && y < 700) {
                                FloatRect roleRect({280, y}, {100, 30});
                                if (roleRect.contains(mousePosF)) {
                                    if (user.username != "admin") {
                                        auth.setAdminStatus(user.id, !user.is_admin);
                                        allUsers = auth.getAllUsers();
                                        displayUsers.clear();
                                        for (const auto& u : allUsers) {
                                            if (u.username.find(searchBox.value) != string::npos) {
                                                displayUsers.push_back(u);
                                            }
                                        }
                                    }
                                }
                                
                            }
                            y += 50;
                        }
                    } else {
                        
                    }
                }
            }
            
            if (currentState == State::List) {
                if (const auto* wheel = event->getIf<Event::MouseWheelScrolled>()) {
                    scrollY += wheel->delta * 30.0f;
                    if (scrollY > 0) scrollY = 0;
                    if (totalHeight > 700 && scrollY < -(totalHeight - 700)) scrollY = -(totalHeight - 700);
                }
            }
        }
        
        window.clear(Color(20, 20, 20));
        
        RectangleShape header({900.0f, 110.0f});
        header.setFillColor(Color(30, 30, 30));
        window.draw(header);
        
        Text title(font);
        title.setString(userMgmtStr);
        title.setCharacterSize(24);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        title.setPosition({20, 20});
        window.draw(title);
        
        window.draw(backBtn.shape);
        window.draw(backBtn.text);
        
        searchBox.updateDisplay();
        window.draw(searchBox.label);
        window.draw(searchBox.shape);
        window.draw(searchBox.text);
        if (searchBox.isActive) {
            RectangleShape border = searchBox.shape;
            border.setFillColor(Color::Transparent);
            border.setOutlineColor(Color(30, 215, 96));
            border.setOutlineThickness(1);
            window.draw(border);
        }
        
        float headerY = 100;
        auto drawHeader = [&](const String& text, float x) {
            Text t(font);
            t.setString(text);
            t.setCharacterSize(14);
            t.setFillColor(Color(150, 150, 150));
            t.setPosition({x, headerY});
            window.draw(t);
        };
        
        drawHeader(usernameStr, 20);
        drawHeader(roleStr, 230);
        drawHeader(createdStr, 450);
        drawHeader(actionsStr, 650);
        
        RectangleShape line({860.0f, 1.0f});
        line.setPosition({20, 120});
        line.setFillColor(Color(60, 60, 60));
        window.draw(line);
        
        float y = 130 + scrollY;
        for (const auto& user : displayUsers) {
            if (y > 110 && y < 700) {
                Text userText(font);
                userText.setString(user.username);
                userText.setCharacterSize(16);
                userText.setPosition({20, y + 4});
                userText.setFillColor(Color::White);
                window.draw(userText);
                
                RectangleShape roleBtn({100, 24});
                roleBtn.setPosition({230, y + 2});
                roleBtn.setFillColor(user.is_admin ? Color(60, 20, 20) : Color(20, 60, 20));
                roleBtn.setOutlineColor(user.is_admin ? Color(200, 50, 50) : Color(50, 200, 100));
                roleBtn.setOutlineThickness(1);
                window.draw(roleBtn);
                
                Text roleText(font);
                roleText.setString(user.is_admin ? "ADMIN" : "USER");
                roleText.setCharacterSize(12);
                roleText.setFillColor(Color::White);
                FloatRect roleRect = roleText.getLocalBounds();
                roleText.setOrigin({roleRect.position.x + roleRect.size.x/2, roleRect.position.y + roleRect.size.y/2});
                roleText.setPosition({280, y + 14});
                window.draw(roleText);
                
                Text dateText(font);
                string dateStr = user.created_at.substr(0, 19);
                dateText.setString(dateStr);
                dateText.setCharacterSize(14);
                dateText.setPosition({450, y + 5});
                dateText.setFillColor(Color(150, 150, 150));
                window.draw(dateText);
                
                RectangleShape rowLine({860.0f, 1.0f});
                rowLine.setPosition({20, y + 40});
                rowLine.setFillColor(Color(40, 40, 40));
                window.draw(rowLine);
            }
            y += 50;
        }
        
        if (false) {
            RectangleShape dim({900, 700});
            dim.setFillColor(Color(0, 0, 0, 200));
            window.draw(dim);
            
            RectangleShape modal({500, 300});
            modal.setOrigin({250, 150});
            modal.setPosition({450, 350});
            modal.setFillColor(Color(30, 30, 30));
            modal.setOutlineColor(Color(60, 60, 60));
            modal.setOutlineThickness(2);
            window.draw(modal);
            
            if (false) {
                Text modalTitle(font);
                String userStr = String::fromUtf8(selectedUsername.begin(), selectedUsername.end());
                modalTitle.setString(passForStr + userStr);
                modalTitle.setCharacterSize(18);
                modalTitle.setFillColor(Color::White);
                modalTitle.setPosition({250, 240});
                window.draw(modalTitle);
                
                Text hashText(font);
                hashText.setString(selectedHash);
                hashText.setCharacterSize(16);
                hashText.setFillColor(Color(30, 215, 96));
                
                FloatRect textRect = hashText.getLocalBounds();
                hashText.setOrigin({textRect.position.x + textRect.size.x/2, textRect.position.y + textRect.size.y/2});
                hashText.setPosition({450, 350});
                window.draw(hashText);
                
                Text closeHint(font);
                closeHint.setString(closeHintStr);
                closeHint.setCharacterSize(12);
                closeHint.setFillColor(Color(150, 150, 150));
                closeHint.setPosition({380, 450});
                window.draw(closeHint);
                
            } else {
                Text modalTitle(font);
                String userStr = String::fromUtf8(selectedUsername.begin(), selectedUsername.end());
                if (false) modalTitle.setString(editUserStr + userStr);
                
                modalTitle.setCharacterSize(20);
                modalTitle.setFillColor(Color::White);
                FloatRect titleRect = modalTitle.getLocalBounds();
                modalTitle.setOrigin({titleRect.position.x + titleRect.size.x/2, titleRect.position.y + titleRect.size.y/2});
                modalTitle.setPosition({450, 280});
                window.draw(modalTitle);
                
                if (false) {
                    window.draw(editBox.shape);
                    string displayString = editBox.value;
                    if (editBox.isPassword) {
                        displayString = string(displayString.length(), '*');
                    }
                    editBox.text.setString(String::fromUtf8(displayString.begin(), displayString.end()) + (editBox.isActive ? "|" : ""));
                    window.draw(editBox.text);
                    
                    if (editBox.isActive) {
                        RectangleShape border = editBox.shape;
                        border.setFillColor(Color::Transparent);
                        border.setOutlineColor(Color(30, 215, 96));
                        border.setOutlineThickness(2);
                        window.draw(border);
                    }
                } else {
                     Text warnText(font);
                    warnText.setString(cannotUndoneStr);
                    warnText.setCharacterSize(16);
                    warnText.setFillColor(Color(200, 100, 100));
                    FloatRect warnRect = warnText.getLocalBounds();
                    warnText.setOrigin({warnRect.position.x + warnRect.size.x/2, warnRect.position.y + warnRect.size.y/2});
                    warnText.setPosition({450, 350});
                    window.draw(warnText);
                }
                
                window.draw(saveBtn.shape);
                window.draw(saveBtn.text);
                window.draw(cancelBtn.shape);
                window.draw(cancelBtn.text);
            }
        }
        
        window.display();
    }
    auth.disconnect();
#endif
}
