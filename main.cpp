#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <x86intrin.h>
#include <stdio.h>
#include <math.h>
//==================================================================

const int Nmax = 10000;

//==================================================================

class FPS
{
    private:
        sf::Clock clock_; 
        unsigned int frame_;
        float fps_;

    public:
        FPS() : frame_(0), fps_(0) {}
       ~FPS() {frame_ = 0; fps_ = 0;}

        float getFPS() const {return fps_;}

        void update()
        {
            if (clock_.getElapsedTime().asSeconds() >= 1.f)
            {
                fps_   = frame_ / clock_.getElapsedTime().asSeconds();
                frame_ = 0;

                clock_.restart();                
            }

            ++frame_;
        }
};

void Fps_text_update(const FPS& fps, char* buffer, sf::Text* text)
{
    sprintf(buffer, "%.2f", fps.getFPS());
    sf::String str(buffer);
    text->setString(str);
}

void Set_text(sf::Text* text, const sf::Font& font, const sf::Color color, size_t font_size, float x, float y)
{
    text->setFont(font);
    text->setStyle(sf::Text::Bold);
    text->setFillColor(color);
    text->setCharacterSize(font_size);
    text->setPosition(x, y);
}

void Set_pixel(sf::Uint8* pixels, int dx, int dy, int n)
{
    pixels[(1280 * dy + dx) * 4 + 0] = 5 * tanh (n);
    pixels[(1280 * dy + dx) * 4 + 1] = sinh (n);
    pixels[(1280 * dy + dx) * 4 + 2] = 255 - sin (n) * 12 * cosh(n);
    pixels[(1280 * dy + dx) * 4 + 3] = 255 - exp (-(float)n / 255);
}

void Count_mondelbrot_set(sf::Uint8* pixels, float scale, float cx, float cy, float R)
{
    const float R2 = R * R;

    for (int dy = 0; dy < 720; dy++)
    {
        float y0 = (dy - cy) * scale;
        float x0 = (   - cx) * scale;

    //     for (int dx = 0; dx < 1280; dx += 4, x0 += 4 * scale)
    //     {
    //         float X0[4] = {x0, x0 + scale, x0 + 2 * scale, x0 + 3 * scale};
    //         float Y0[4] = {y0, y0, y0, y0};

    //         int N[4] = {0, 0, 0, 0};

    //         float X[4] = {X0[0], X0[1], X0[2], X0[3]};
    //         float Y[4] = {Y0[0], Y0[1], Y0[2], Y0[3]};

    //         for (int i = 0; i < 4; ++i)
    //         {
    //             for ( ; N[i] < Nmax; N[i]++)
    //             {
    //                 float x2 = X[i] * X[i];
    //                 float y2 = Y[i] * Y[i];
    //                 float xy = X[i] * Y[i];

    //                 float r2 = x2 + y2;

    //                 if (r2 > R2) break;

    //                 X[i] = x2 - y2 + X0[i];
    //                 Y[i] = xy + xy + Y0[i];
    //             }
    //         } 
            
    //         for (int i = 0; i < 4; ++i)
    //         {
    //             Set_pixel(pixels, dx + i, dy, N[i]);
    //         }
    //     }

        for (int dx = 0; dx < 1280; dx++, x0 += scale)
        {
            int n = 0;

            float x = x0;
            float y = y0;

        
            for ( ; n < Nmax; n++)
            {
                float x2 = x * x;
                float y2 = y * y;
                float xy = x * y;

                float r2 = x2 + y2;

                if (r2 > R2) break;

                x = x2 - y2 + x0;
                y = xy + xy + y0;
            }
            
        
            Set_pixel(pixels, dx, dy, n);
        }

    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mandelbrot set", sf::Style::Default);
    //window.setFramerateLimit(360);

    sf::Font font;
    font.loadFromFile("Minecraft.ttf");

    sf::Texture screen;
    screen.create(1280, 720);

    sf::Sprite sprite(screen);
    sf::Uint8* pixels = new sf::Uint8[1280 * 720 * 4];

    FPS fps;
    char fps_buffer[10];
    sf::Text fps_text;
    Set_text(&fps_text, font, sf::Color(20, 250, 20), 40, 20, 20);

    float scale = 0.004;
    int x = 760;
    int y = 360;

    bool box_fps = false;
    
    while (window.isOpen())
    {
        sf::Event Event;

        fps.update();

        while (window.pollEvent(Event))
        {
            switch (Event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();
                }
                break;
                    
                
                case sf::Event::KeyPressed:
                {
                    switch (Event.key.code)
                    {
                        case sf::Keyboard::F:
                        {
                            box_fps = !box_fps;
                        }
                        break;

                        case sf::Keyboard::Add:
                        {
                            scale *= 0.75;
                        }
                        break;

                        case sf::Keyboard::Subtract:
                        {
                            scale *= 1.25;
                        }
                        break;

                        case sf::Keyboard::Up:
                        {
                            y += 100;
                        }
                        break;

                        case sf::Keyboard::Down:
                        {
                            y -= 100;
                        }
                        break;

                        case sf::Keyboard::Right:
                        {
                            x += 100;
                        }
                        break;

                        case sf::Keyboard::Left:
                        {
                            x -= 100;
                        }
                        break;

                        default:
                            break;
                    }
                }
                break;

            }
        }

        Count_mondelbrot_set(pixels, scale, x, y, 100.f);
        screen.update(pixels);
        

        Fps_text_update(fps, fps_buffer, &fps_text);

        window.clear();
        window.draw(sprite);
        if (box_fps) window.draw(fps_text);
        window.display();
    }

    delete pixels;

    return 0;
}