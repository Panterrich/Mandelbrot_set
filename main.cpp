#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <x86intrin.h>
#include <stdio.h>
#include <math.h>
//==================================================================

const int Nmax = 10000;

const __m256 Steps = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
const __m256i Mask = _mm256_set1_epi32(1);

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
    // pixels[(1280 * dy + dx) * 4 + 0] = 5 * tanh (n);
    // pixels[(1280 * dy + dx) * 4 + 1] = sinh (n);
    // pixels[(1280 * dy + dx) * 4 + 2] = 255 - sin (n) * 12 * cosh(n);
    // pixels[(1280 * dy + dx) * 4 + 3] = 255 - exp (-(float)n / 255);

    // pixels[(1280 * dy + dx) * 4 + 0] = 5 * n;
    // pixels[(1280 * dy + dx) * 4 + 1] = n;
    // pixels[(1280 * dy + dx) * 4 + 2] = 255 - 12 * n;
    // pixels[(1280 * dy + dx) * 4 + 3] = 255;

    if (n == Nmax)
    {
        pixels[(1280 * dy + dx) * 4 + 0] = 0;
        pixels[(1280 * dy + dx) * 4 + 1] = 0;
        pixels[(1280 * dy + dx) * 4 + 2] = 0;
        pixels[(1280 * dy + dx) * 4 + 3] = 0;
    }
    
    else
    {
        pixels[(1280 * dy + dx) * 4 + 0] = 100 * (1 + sin(0.5 * n + 0));
        pixels[(1280 * dy + dx) * 4 + 1] = 100 * (1 + sin(0.5 * n + 2));
        pixels[(1280 * dy + dx) * 4 + 2] = 100 * (1 + sin(0.5 * n + 4));
        pixels[(1280 * dy + dx) * 4 + 3] = 255;
    }
    

}

void Count_mondelbrot_set(sf::Uint8* pixels, float scale, float cx, float cy, float R)
{
    const float square = R * R;
    const __m256 R2 = _mm256_set1_ps(square);

    __m256 Scale = _mm256_set1_ps(scale);

    for (int dy = 0; dy < 720; dy++)
    {
        float y0 = (dy - cy) * scale;
        float x0 = (   - cx) * scale;

        for (int dx = 0; dx < 1280; dx += 8, x0 += 8 * scale)
        {
            __m256 X0 = _mm256_add_ps(_mm256_set1_ps(x0), _mm256_mul_ps(Scale, Steps));
            __m256 Y0 = _mm256_set1_ps(y0);

            union 
            {
                __m256i N = _mm256_setzero_si256();
                int iterations[8];
            };
            

            int n = 0;

            __m256 X = X0;
            __m256 Y = Y0;
            __m256i add = _mm256_set1_epi32(1);
   
            for ( ; n < Nmax; n++)
            {
                __m256 x2 = _mm256_mul_ps(X, X);
                __m256 y2 = _mm256_mul_ps(Y, Y);
                __m256 xy = _mm256_mul_ps(X, Y);

                __m256 r2 = _mm256_add_ps(x2, y2);
        
                __m256 inc = _mm256_cmp_ps(r2, R2, _CMP_LT_OS);
                if (!_mm256_movemask_ps(inc)) break;

                X = _mm256_add_ps(_mm256_sub_ps(x2, y2), X0);
                Y = _mm256_add_ps(_mm256_add_ps(xy, xy), Y0);

                add = _mm512_maskz_and_epi32(inc, add, Mask);
                N = _mm512_add_epi32(add, N);
            }
            
            for (int i = 0; i < 16; i++)
            {   
                Set_pixel(pixels, dx + i, dy, iterations[i]);
            }
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

    bool draw_set = false;
    bool box_fps  = false;
    
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

                        case sf::Keyboard::Escape:
                        {
                            draw_set = !draw_set;
                        }
                        break;

                        case sf::Keyboard::Add:
                        {
                            scale *= 0.8;
                            x *= 1.25;
                            y *= 1.25;
                           
                        }
                        break;

                        case sf::Keyboard::Subtract:
                        {
                            scale *= 1.25;
                            x *= 0.8;
                            y *= 0.8;
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
                            x -= 100;
                        }
                        break;

                        case sf::Keyboard::Left:
                        {
                            x += 100;
                        }
                        break;

                        default:
                            break;
                    }
                }
                break;

            }
        }

        if (draw_set) 
        {
            Count_mondelbrot_set(pixels, scale, x, y, 100.f);
        }

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